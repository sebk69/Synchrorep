/*
 * execute.cpp
 * Originally : synchronization.cpp
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011,2019 - Sébastien Kus
 *
 *  Synchronization and copy process
 *
 */

#include "execute.h"

#include "../accessors/ac_config.h"

#include "../technical/tc_misc.h"
#include "../technical/tc_mount.h"

#include <iostream>
#include <sstream>

#include <strings.h>
#include <string.h>

GMutex				*execute :: ask_mutex;

// TODO : bug : the files copying don't work

// exchange between synchronize process and progession window
execute_exchanges :: execute_exchanges()
{
	if(!g_thread_supported())
		g_thread_init(NULL);
	this->synchronizing = g_mutex_new();

	this->ask_deletion_mutex = g_mutex_new();
	this->ask_file_conflict_mutex = g_mutex_new();

	this->answer_cond = g_cond_new();

	this->asking_deletion = new MUTEX_DIALOG_DELETE(NULL);
	this->asking_file_conflict = new MUTEX_DIALOG_FILE_CONFLICT(NULL);
	this->asking_error = new MUTEX_DIALOG_ERROR(NULL);
	this->asking_info = new MUTEX_DIALOG_INFO(NULL);
}

void
execute_exchanges :: init(char *command_line_parameter)
{
	while(g_mutex_trylock(this->synchronizing));
	this->command_line_parameter.set(command_line_parameter);
	this->num_files.set(0);
	this->num_files_scanned.set(0);
	this->message =NULL;
	this->sync_step.set(_pending);
	this->cancel.set(false);
}

execute_exchanges :: ~execute_exchanges()
{
	if(this->message != NULL)
		free(this->message);
	g_mutex_free(this->synchronizing);
	g_mutex_free(this->ask_deletion_mutex);
	g_mutex_free(this->ask_file_conflict_mutex);
}

execute :: execute()
{
	this->exchanges = new execute_exchanges*[150];
	this->exchanges[0] = NULL;
	this->syncs = new GThread*[150];
	this->syncs[0] = NULL;
}

execute :: ~execute()
{
	delete this->exchanges;
	delete this->syncs;
	g_mutex_free(execute :: ask_mutex);
}

void
execute :: new_folder(char *command_line_p, config_mode mode)
{
	ac_config			configuration(command_line_p);

	// insert synchronization at the end of then list
	int					i = 0;
	while(this->exchanges[i] != NULL)
		i++;
	this->exchanges[i] = new execute_exchanges;
	this->exchanges[i]->init(command_line_p);
	this->exchanges[i+1] = NULL;

	// try to mount locations if not mounted
	tc_mount_from_file		mounting_from(configuration.get_from_folder());
	ostringstream			text;
	if(!mounting_from.is_mounted())
		if(!mounting_from.mount())
		{
			text<<gettext("Impossible to mount disk/share of ")<<g_uri_unescape_string(configuration.get_from_folder_uri(), NULL);
			//cerr<<text<<endl;
			this->exchanges[i]->asking_info->asking((char*)text.str().c_str());
			return;
		}
	tc_mount_from_file		mounting_to(configuration.get_to_folder());
	if(!mounting_to.is_mounted())
		if(!mounting_to.mount())
		{
			text<<gettext("Impossible to mount disk/share of ")<<g_uri_unescape_string(configuration.get_to_folder_uri(), NULL);
			//cerr<<text<<endl;
			this->exchanges[i]->asking_info->asking((char*)text.str().c_str());
			return;
		}

	if(mode == _sync)
		this->syncs[i] = g_thread_create(&this->synchronize, (gpointer)this->exchanges[i], true, NULL);
	else
		this->syncs[i] = g_thread_create(&this->perform_copy, (gpointer)this->exchanges[i], true, NULL);

	this->syncs[i+1] = NULL;
}

void
execute :: start()
{
	this->progress = g_thread_create(&this->progression_window, (gpointer)this->exchanges, true, NULL);
	execute :: ask_mutex = g_mutex_new();

	// join threads
	int					i = 0;
	while(this->syncs[i] != NULL)
	{
		g_thread_join(this->syncs[i]);
		i++;
	}
	g_thread_join(this->progress);
}

gpointer
execute :: synchronize(gpointer pexchanges)
{
	execute_exchanges		*exchanges = (execute_exchanges*)pexchanges;

	// get configuration
	ac_config			configuration(exchanges->command_line_parameter.get());
	if(!configuration.trylock())
	{
		cerr<<"Configuration is already running !"<<endl;
		exchanges->sync_step.set(_terminated);
		return NULL;
	}
	// setup log object
	time_t				launch_time;
	time(&launch_time);
	logging						log(&configuration, launch_time);

	// by security test that configuration is ready to start
	if(!configuration.ready_to_start())
	{
		cerr<<"Configuration is not ready to start !"<<endl;
		while(!g_mutex_trylock(execute :: ask_mutex));
		exchanges->asking_info->asking(gettext("One of the synchronization folder is missing."));
		g_mutex_unlock(execute :: ask_mutex);
		exchanges->sync_step.set(_terminated);
		return NULL;
	}

	log.start_action(_start, NULL);
	log.close_action();

	// build files database
	log.set_context(_context_scan);
	exchanges->sync_step.set(_analyse_step);
	tc_find_files			*from_files;
	tc_find_files			*to_files;

	configuration.files(&from_files, &to_files, &exchanges->cancel);
	if(exchanges->cancel.get())
	{
		log.start_action(_abort, NULL);
		if(from_files->error != NULL)
			exchanges->asking_error->asking(log.get_error_context(from_files->error));
		else if(to_files->error != NULL)
			exchanges->asking_error->asking(log.get_error_context(to_files->error));
		log.close_action();
		exchanges->sync_step.set(_terminated);
		return NULL;
	}


	// process files
	bool					from_not_at_the_end = false;
	bool					to_not_at_the_end = false;
	char					*current_from_path;
	char					*current_to_path;
	char					*current_partial_path;
	char					*temp_base_folder;
	char					temp_file_path[10000];
	tc_file_with_infos		*from_file;
	tc_file_with_infos		*to_file;
	int						partial_path_compare;
	tc_files_binomial		*binomial;
	bool					move_supr_to_trash;
	bool					move_replaced_to_trash;
	// get number of files to synchronize
	exchanges->num_files.set(from_files->count() + to_files->count());

	// constant zero time
	GTimeVal				zero_time;
	zero_time.tv_sec = 0;
	zero_time.tv_usec = 0;

	// detect time difference between locations
	// system time
	GTimeVal				sys_time;
	GError					*error = NULL;
	int						from_dif;
	int						to_dif;
	g_get_current_time(&sys_time);
	// from folder time
	GTimeVal				from_time = from_files->get_time(&error);
	if(from_time.tv_sec != 0)
		from_dif =  (int)sys_time.tv_sec - (int)from_time.tv_sec;
	else
	{
		error_infos				infos;
		infos.context = strdup(gettext("Error when getting system time."));
		infos.reason = error->message;
		while(!g_mutex_trylock(execute :: ask_mutex));
		if(exchanges->asking_error->asking(infos) == _error_choose_cancel)
			exchanges->cancel.set(true);
		else
			from_dif = 0;
		g_mutex_unlock(execute :: ask_mutex);
	}

	// system time
	g_get_current_time(&sys_time);
	// to folder time
	error = NULL;
	GTimeVal				to_time = to_files->get_time(&error);
	if(to_time.tv_sec != 0)
		to_dif =  (int)sys_time.tv_sec - (int)to_time.tv_sec;
	else
	{
		error_infos				infos;
		infos.context = strdup(gettext("Error when getting system time."));
		infos.reason = error->message;
		while(!g_mutex_trylock(execute :: ask_mutex));
		if(exchanges->asking_error->asking(infos) == _error_choose_cancel)
			exchanges->cancel.set(true);
		else
			to_dif = 0;
		g_mutex_unlock(execute :: ask_mutex);
	}

	char				prior_after[256];
	unsigned int		non_negative_dif;

	// from warning
	if(from_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)from_dif;
	if(non_negative_dif != 0)
		cout<<"Warning : The remote system time is "<<non_negative_dif<<" seconds "<<prior_after<<" your system time."<<endl;
	// to warning
	if(to_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)to_dif;
	if(non_negative_dif != 0)
		cout<<"Warning : The remote system time is "<<non_negative_dif<<" seconds "<<prior_after<<" your system time."<<endl;

	log.set_context(_context_process);
	if(!exchanges->cancel.get())
		exchanges->sync_step.set(_synchronize_step);
	else
		application :: exit_app();

	// setup dialogs from behavior
	bool				ask_at_the_end = configuration.behavior->get_ask_at_the_end();
	bool				automatic_suppr = false;
	bool				automatic_both_modified = false;
	if(!configuration.behavior->get_alert_when_file_deletion())
	{
		automatic_suppr = true;
		exchanges->asking_deletion->no_confirmation(_delete_choose_yes);
	}
	switch(configuration.behavior->get_to_do_on_modify_conflict())
	{
		case _question_on_modify:
		break;

		case _ignore_modifications:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_ignore);
		break;

		case _stop_on_modified:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_cancel);
		break;

		case _get_older_modified:
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_older);
		break;

		case _get_recent_modified:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_recent);
		break;
	}
	switch(configuration.behavior->get_to_do_on_error())
	{
		case _question_on_error:
			if(ask_at_the_end)
				exchanges->asking_error->no_confirmation(_error_choose_ignore);
		break;

		case _ignore_error:
			exchanges->asking_error->no_confirmation(_error_choose_ignore);
		break;

		case _stop_on_error:
			exchanges->asking_error->no_confirmation(_error_choose_cancel);
		break;
	}
	move_supr_to_trash = configuration.behavior->get_move_to_trash_when_delete();
	move_replaced_to_trash = configuration.behavior->get_move_to_trash_when_overwrite();

	bool				md5sum_compare;
	bool				check_md5sum_on_modify;

	switch(configuration.behavior->get_md5sum_level())
	{
		case _no_md5sum:
			md5sum_compare = false;
			check_md5sum_on_modify = false;
		break;

		case _both_modified_md5sum:
			md5sum_compare = false;
			check_md5sum_on_modify = true;
		break;

		case _database_md5sum:
			md5sum_compare = true;
			check_md5sum_on_modify = true;
		break;
	}

	// first position
	from_not_at_the_end = from_files->begin();
	to_not_at_the_end = to_files->begin();
	bool			from_remove_ok = false,
					to_remove_ok = false;
	while((from_not_at_the_end || to_not_at_the_end) && exchanges->sync_step.get() != _cancel)
	{
		// force comparisons if one reach the end
		if(from_not_at_the_end)
			current_from_path = from_files->get_current_partial_path();
		if(to_not_at_the_end)
			current_to_path = to_files->get_current_partial_path();
		if(!from_not_at_the_end)
			partial_path_compare = 1;
		else if(!to_not_at_the_end)
			partial_path_compare = -1;
		else
		{
			// get files partial path to compare both progression between from and to
			if(from_not_at_the_end && to_not_at_the_end)
				partial_path_compare = strcmp(current_from_path, current_to_path);
		}

		// if equals, binomial is ready
		if(partial_path_compare == 0)
		{
			from_file = from_files->get_file_with_info();
			to_file = to_files->get_file_with_info();
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_from_path;
			from_remove_ok = true;
			to_remove_ok = true;
		}
		// from is before to, build to file and create binomial
		else if(partial_path_compare < 0)
		{
			// build path of to file
			temp_file_path[0] = '\0';
			temp_base_folder = to_files->get_base_folder_path();
			strcat(temp_file_path, temp_base_folder);
			strcat(temp_file_path, "/");
			strcat(temp_file_path, current_from_path);

			// make to file
			from_file = from_files->get_file_with_info();
			to_file = new tc_file_with_infos(temp_file_path);
			to_file->set_partial_path(from_file->get_partial_path());

			// make binomial
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_from_path;

			// free pointers
			free(temp_base_folder);

			from_remove_ok = true;
			to_remove_ok = false;
		}
		// else to is before from, build from file and create binomial
		else if(partial_path_compare > 0)
		{
			// build path of to file
			temp_file_path[0] = '\0';
			temp_base_folder = from_files->get_base_folder_path();
			strcat(temp_file_path, temp_base_folder);
			strcat(temp_file_path, "/");
			strcat(temp_file_path, current_to_path);

			// make to file
			from_file =  new tc_file_with_infos(temp_file_path);
			to_file = to_files->get_file_with_info();
			from_file->set_partial_path(to_file->get_partial_path());

			// make binomial
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_to_path;

			// free pointers
			free(temp_base_folder);

			from_remove_ok = false;
			to_remove_ok = true;
		}
		binomial->from_dif = from_dif;
		binomial->to_dif = to_dif;
		// now that binomial is ready, test possible cases and act
		_files_compare			compare_result;
		if(md5sum_compare)
			compare_result = binomial->md5sum_compare();
		else
			compare_result = binomial->sync_compare(check_md5sum_on_modify);
		switch(compare_result)
		{
			case _from_modified:
				log.start_action(_copy_from_to_to, binomial);
				if(binomial->copy(_binomial_from, move_replaced_to_trash))
				{
					// save database dates
					to_file->refresh();
					configuration.refresh_file_datas(current_partial_path, from_file, to_file);
				}
				else
				{
					if(!ask_at_the_end)
					{
						while(!g_mutex_trylock(execute :: ask_mutex));
						switch(exchanges->asking_error->asking(log.get_error()))
						{
							case _error_choose_cancel:
								exchanges->sync_step.set(_cancel);
							break;

							default:
							break;
						}
						g_mutex_unlock(execute :: ask_mutex);
					}
				}
			break;

			case _both_modified_from_recent:
				if(!ask_at_the_end || automatic_both_modified)
				{
					switch(exchanges->asking_file_conflict->asking(binomial))
					{
						case _conflict_choose_recent:
						case _conflict_choose_source:
							log.start_action(_copy_from_to_to, binomial);
							if(binomial->copy(_binomial_from, move_replaced_to_trash))
							{
								// save database dates
								from_file->refresh();
								configuration.refresh_file_datas(current_partial_path, from_file, to_file);
							}
							else
							{
								while(!g_mutex_trylock(execute :: ask_mutex));
								switch(exchanges->asking_error->asking(log.get_error()))
								{
									case _error_choose_cancel:
										exchanges->sync_step.set(_cancel);
									break;

									default:
									break;
								}
								g_mutex_unlock(execute :: ask_mutex);
							}
						break;

						case _conflict_choose_older:
						case _conflict_choose_target:
							log.start_action(_copy_to_to_from, binomial);
							if(binomial->copy(_binomial_to, move_replaced_to_trash))
							{
								// save database dates
								from_file->refresh();
								configuration.refresh_file_datas(current_partial_path, from_file, to_file);
							}
							else
							{
								while(!g_mutex_trylock(execute :: ask_mutex));
								switch(exchanges->asking_error->asking(log.get_error()))
								{
									case _error_choose_cancel:
										exchanges->sync_step.set(_cancel);
									break;

									default:
									break;
								}
								g_mutex_unlock(execute :: ask_mutex);
							}
						break;

						case _conflict_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						case _conflict_choose_ignore:
						break;
					}
				}
				else
				{
					if(!move_replaced_to_trash)
						log.start_action(_ask_both, binomial);
					else
						log.start_action(_ask_both_trash, binomial);
					log.close_action();
				}
			break;

			case _to_modified:
				log.start_action(_copy_to_to_from, binomial);
				if(binomial->copy(_binomial_to, move_replaced_to_trash))
				{
					// save database dates
					from_file->refresh();
					configuration.refresh_file_datas(current_partial_path, from_file, to_file);
				}
				else
				{
					while(!g_mutex_trylock(execute :: ask_mutex));
					switch(exchanges->asking_error->asking(log.get_error()))
					{
						case _error_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						default:
						break;
					}
					g_mutex_unlock(execute :: ask_mutex);
				}
			break;

			case _both_modified_to_recent:
				if(!ask_at_the_end || automatic_both_modified)
				{
					while(!g_mutex_trylock(execute :: ask_mutex));
					switch(exchanges->asking_file_conflict->asking(binomial))
					{
						case _conflict_choose_older:
						case _conflict_choose_source:
							log.start_action(_copy_from_to_to, binomial);
							if(binomial->copy(_binomial_from, move_replaced_to_trash))
							{
								// save database dates
								from_file->refresh();
								configuration.refresh_file_datas(current_partial_path, from_file, to_file);
							}
							else
								switch(exchanges->asking_error->asking(log.get_error()))
								{
									case _error_choose_cancel:
										exchanges->sync_step.set(_cancel);
									break;

									default:
									break;
								}
						break;

						case _conflict_choose_recent:
						case _conflict_choose_target:
							log.start_action(_copy_to_to_from, binomial);
							if(binomial->copy(_binomial_to, move_replaced_to_trash))
							{
								// save database dates
								from_file->refresh();
								configuration.refresh_file_datas(current_partial_path, from_file, to_file);
							}
							else
								switch(exchanges->asking_error->asking(log.get_error()))
								{
									case _error_choose_cancel:
										exchanges->sync_step.set(_cancel);
									break;

									default:
									break;
								}
							g_mutex_unlock(execute :: ask_mutex);
						break;

						case _conflict_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						case _conflict_choose_ignore:
						break;
					}
				}
				else
				{
					if(!move_replaced_to_trash)
						log.start_action(_ask_both, binomial);
					else
						log.start_action(_ask_both_trash, binomial);
					log.close_action();
				}
			break;

			case _from_deleted:
				// in case of folder containing items remove down items
				if(from_remove_ok)
					from_files->remove_down();
				if(to_remove_ok)
					to_files->remove_down();

				if(!ask_at_the_end || automatic_suppr)
				{
					while(!g_mutex_trylock(execute :: ask_mutex));
					switch(exchanges->asking_deletion->asking(g_uri_unescape_string(binomial->get_from()->get_uri(), NULL)))
					{
						case _delete_choose_yes:
							if(move_supr_to_trash)
								log.start_action(_trash_to, binomial);
							else
								log.start_action(_delete_to, binomial);
							if(binomial->remove(_binomial_to, move_supr_to_trash, &exchanges->num_files, &exchanges->cancel))
							{
								// save database dates
								configuration.refresh_file_datas(current_partial_path, NULL, NULL);
							}
							else
							{
								if(exchanges->cancel.get())
									exchanges->sync_step.set(_cancel);
								else
									switch(exchanges->asking_error->asking(log.get_error()))
									{
										case _error_choose_cancel:
											exchanges->sync_step.set(_cancel);
										break;

										default:
										break;
									}
							}
						break;

						case _delete_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						default:
						break;
					}
					g_mutex_unlock(execute :: ask_mutex);
				}
				else
				{
					if(move_supr_to_trash)
						log.start_action(_ask_trash, binomial);
					else
						log.start_action(_ask_delete, binomial);
					log.close_action();
				}
			break;

			case _to_deleted:
				// in case of folder containing items remove down items
				if(from_remove_ok)
					from_files->remove_down();
				if(to_remove_ok)
					to_files->remove_down();

				if(!ask_at_the_end || automatic_suppr)
				{
					while(!g_mutex_trylock(execute :: ask_mutex));
					switch(exchanges->asking_deletion->asking(g_uri_unescape_string(binomial->get_to()->get_uri(), NULL)))
					{
						case _delete_choose_yes:
							if(move_supr_to_trash)
								log.start_action(_trash_from, binomial);
							else
								log.start_action(_delete_from, binomial);
							if(binomial->remove(_binomial_from, move_supr_to_trash, &exchanges->num_files, &exchanges->cancel))
							{
								// save database dates
								configuration.refresh_file_datas(current_partial_path, NULL, NULL);
							}
							else
							{
								if(exchanges->cancel.get())
									exchanges->sync_step.set(_cancel);
								else
									switch(exchanges->asking_error->asking(log.get_error()))
									{
										case _error_choose_cancel:
											exchanges->sync_step.set(_cancel);
										break;

										default:
										break;
									}
							}
						break;

						case _delete_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						default:
						break;
					}
					g_mutex_unlock(execute :: ask_mutex);
				}
				else
				{
					if(move_supr_to_trash)
						log.start_action(_ask_trash, binomial);
					else
						log.start_action(_ask_delete, binomial);
					log.close_action();
				}
			break;

			case _both_deleted:
				configuration.refresh_file_datas(current_partial_path, NULL, NULL);
			break;

			case _both_modified_equal:
				from_file->refresh();
				to_file->refresh();
				configuration.refresh_file_datas(current_partial_path, from_file, to_file);
			break;

			case _comparison_error:
				log.start_action(_compare, binomial);
				switch(exchanges->asking_error->asking(log.get_error()))
				{
					case _error_choose_cancel:
						exchanges->sync_step.set(_cancel);
					break;

					default:
					break;
				}
				log.close_action();
			break;

			case _files_not_modified:
			break;
		}
		log.close_action();

		// free pointers
		if(from_not_at_the_end)
			free(current_from_path);
		if(to_not_at_the_end)
			free(current_to_path);
		delete from_file;
		delete to_file;
		delete binomial;

		// next files in lists
		if(partial_path_compare == 0)
		{
			if(from_not_at_the_end)
			{
				from_not_at_the_end = from_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
			if(to_not_at_the_end)
			{
				to_not_at_the_end = to_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
		else if(partial_path_compare < 0)
		{
			if(from_not_at_the_end)
			{
				from_not_at_the_end = from_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
		else if(partial_path_compare > 0)
		{
			if(to_not_at_the_end)
			{
				to_not_at_the_end = to_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
	}
	configuration.clean_deleted_files();
	configuration.unlock();
	if(exchanges->sync_step.get() == _cancel)
	{
		log.start_action(_abort, NULL);
		log.close_action();
	}
	exchanges->sync_step.set(_terminated);
	cout<<exchanges->num_files_scanned.get()<<" files scanned."<<endl;
	log.start_action(_end, NULL);
	log.close_action();

	if(configuration.behavior->get_ask_at_the_end())
	{
		char			cmd[255];
		char			launch_time_str[255];

		strcpy(cmd, "synchrorep --pending-questions ");
		strcat(cmd, configuration.get_from_folder_uri());
		strcat(cmd, " ");
		gcvt(launch_time, 15, launch_time_str);
		strcat(cmd, launch_time_str);
		strcat(cmd, " &");
		system(cmd);
	}

	return NULL;
}

gpointer
execute :: perform_copy(gpointer pexchanges)
{
	execute_exchanges		*exchanges = (execute_exchanges*)pexchanges;

	// get configuration
	ac_config			configuration(exchanges->command_line_parameter.get());
	if(!configuration.trylock())
	{
		cerr<<"Configuration is already running !"<<endl;
		exchanges->sync_step.set(_terminated);
		return NULL;
	}
	// setup log object
	time_t				launch_time;
	time(&launch_time);
	logging						log(&configuration, launch_time);

	// by security test that configuration is ready to start
	if(!configuration.ready_to_start())
	{
		cerr<<"Configuration is not ready to start !"<<endl;
		while(!g_mutex_trylock(execute :: ask_mutex));
		exchanges->asking_info->asking(gettext("One of the synchronization folder is missing."));
		g_mutex_unlock(execute :: ask_mutex);
		exchanges->sync_step.set(_terminated);
		return NULL;
	}

	log.start_action(_start, NULL);
	log.close_action();

	// build files database
	log.set_context(_context_scan);
	exchanges->sync_step.set(_analyse_step);
	tc_find_files			*from_files;
	tc_find_files			*to_files;

	configuration.files(&from_files, &to_files, &exchanges->cancel);
	if(exchanges->cancel.get())
	{
		log.start_action(_abort, NULL);
		if(from_files->error != NULL)
			exchanges->asking_error->asking(log.get_error_context(from_files->error));
		else if(to_files->error != NULL)
			exchanges->asking_error->asking(log.get_error_context(to_files->error));
		log.close_action();
		exchanges->sync_step.set(_terminated);
		return NULL;
	}


	// process files
	bool					from_not_at_the_end = false;
	bool					to_not_at_the_end = false;
	char					*current_from_path;
	char					*current_to_path;
	char					*current_partial_path;
	char					*temp_base_folder;
	char					temp_file_path[10000];
	tc_file_with_infos		*from_file;
	tc_file_with_infos		*to_file;
	int						partial_path_compare;
	tc_files_binomial		*binomial;
	bool					move_supr_to_trash;
	bool					move_replaced_to_trash;
	// get number of files to synchronize
	exchanges->num_files.set(from_files->count() + to_files->count());

	// constant zero time
	GTimeVal				zero_time;
	zero_time.tv_sec = 0;
	zero_time.tv_usec = 0;

	// detect time difference between locations
	// system time
	GTimeVal				sys_time;
	GError					*error = NULL;
	int						from_dif;
	int						to_dif;
	g_get_current_time(&sys_time);
	// from folder time
	GTimeVal				from_time = from_files->get_time(&error);
	if(from_time.tv_sec != 0)
		from_dif =  (int)sys_time.tv_sec - (int)from_time.tv_sec;
	else
	{
		error_infos				infos;
		infos.context = strdup(gettext("Error when getting system time."));
		infos.reason = error->message;
		while(!g_mutex_trylock(execute :: ask_mutex));
		if(exchanges->asking_error->asking(infos) == _error_choose_cancel)
			exchanges->cancel.set(true);
		else
			from_dif = 0;
		g_mutex_unlock(execute :: ask_mutex);
	}

	// system time
	g_get_current_time(&sys_time);
	// to folder time
	error = NULL;
	GTimeVal				to_time = to_files->get_time(&error);
	if(to_time.tv_sec != 0)
		to_dif =  (int)sys_time.tv_sec - (int)to_time.tv_sec;
	else
	{
		error_infos				infos;
		infos.context = strdup(gettext("Error when getting system time."));
		infos.reason = error->message;
		while(!g_mutex_trylock(execute :: ask_mutex));
		if(exchanges->asking_error->asking(infos) == _error_choose_cancel)
			exchanges->cancel.set(true);
		else
			to_dif = 0;
		g_mutex_unlock(execute :: ask_mutex);
	}

	char				prior_after[256];
	unsigned int		non_negative_dif;

	// from warning
	if(from_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)from_dif;
	if(non_negative_dif != 0)
		cout<<"Warning : The remote system time is "<<non_negative_dif<<" seconds "<<prior_after<<" your system time."<<endl;
	// to warning
	if(to_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)to_dif;
	if(non_negative_dif != 0)
		cout<<"Warning : The remote system time is "<<non_negative_dif<<" seconds "<<prior_after<<" your system time."<<endl;

	log.set_context(_context_process);
	if(!exchanges->cancel.get())
		exchanges->sync_step.set(_synchronize_step);
	else
		application :: exit_app();

	// setup dialogs from behavior
	bool				ask_at_the_end = configuration.behavior->get_ask_at_the_end();
	bool				automatic_suppr = false;
	bool				automatic_both_modified = false;
	if(!configuration.behavior->get_alert_when_file_deletion())
	{
		automatic_suppr = true;
		exchanges->asking_deletion->no_confirmation(_delete_choose_yes);
	}
	switch(configuration.behavior->get_to_do_on_modify_conflict())
	{
		case _question_on_modify:
		break;

		case _ignore_modifications:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_ignore);
		break;

		case _stop_on_modified:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_cancel);
		break;

		case _get_older_modified:
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_older);
		break;

		case _get_recent_modified:
			automatic_both_modified = true;
			exchanges->asking_file_conflict->no_confirmation(_conflict_choose_recent);
		break;
	}
	switch(configuration.behavior->get_to_do_on_error())
	{
		case _question_on_error:
			if(ask_at_the_end)
				exchanges->asking_error->no_confirmation(_error_choose_ignore);
		break;

		case _ignore_error:
			exchanges->asking_error->no_confirmation(_error_choose_ignore);
		break;

		case _stop_on_error:
			exchanges->asking_error->no_confirmation(_error_choose_cancel);
		break;
	}
	move_supr_to_trash = configuration.behavior->get_move_to_trash_when_delete();
	move_replaced_to_trash = configuration.behavior->get_move_to_trash_when_overwrite();

	bool				md5sum_compare;
	bool				check_md5sum_on_modify;

	switch(configuration.behavior->get_md5sum_level())
	{
		case _no_md5sum:
			md5sum_compare = false;
			check_md5sum_on_modify = false;
		break;

		case _both_modified_md5sum:
			md5sum_compare = false;
			check_md5sum_on_modify = true;
		break;

		case _database_md5sum:
			md5sum_compare = true;
			check_md5sum_on_modify = true;
		break;
	}

	// first position
	from_not_at_the_end = from_files->begin();
	to_not_at_the_end = to_files->begin();
	bool			from_remove_ok = false,
					to_remove_ok = false;
	while((from_not_at_the_end || to_not_at_the_end) && exchanges->sync_step.get() != _cancel)
	{
		// force comparisons if one reach the end
		if(from_not_at_the_end)
			current_from_path = from_files->get_current_partial_path();
		if(to_not_at_the_end)
			current_to_path = to_files->get_current_partial_path();
		if(!from_not_at_the_end)
			partial_path_compare = 1;
		else if(!to_not_at_the_end)
			partial_path_compare = -1;
		else
		{
			// get files partial path to compare both progression between from and to
			if(from_not_at_the_end && to_not_at_the_end)
				partial_path_compare = strcmp(current_from_path, current_to_path);
		}

		// if equals, binomial is ready
		if(partial_path_compare == 0)
		{
			from_file = from_files->get_file_with_info();
			to_file = to_files->get_file_with_info();
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_from_path;
			from_remove_ok = true;
			to_remove_ok = true;
		}
		// from is before to, build to file and create binomial
		else if(partial_path_compare < 0)
		{
			// build path of to file
			temp_file_path[0] = '\0';
			temp_base_folder = to_files->get_base_folder_path();
			strcat(temp_file_path, temp_base_folder);
			strcat(temp_file_path, "/");
			strcat(temp_file_path, current_from_path);

			// make to file
			from_file = from_files->get_file_with_info();
			to_file = new tc_file_with_infos(temp_file_path);
			to_file->set_partial_path(from_file->get_partial_path());

			// make binomial
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_from_path;

			// free pointers
			free(temp_base_folder);

			from_remove_ok = true;
			to_remove_ok = false;
		}
		// else to is before from, build from file and create binomial
		else if(partial_path_compare > 0)
		{
			// build path of to file
			temp_file_path[0] = '\0';
			temp_base_folder = from_files->get_base_folder_path();
			strcat(temp_file_path, temp_base_folder);
			strcat(temp_file_path, "/");
			strcat(temp_file_path, current_to_path);

			// make to file
			from_file =  new tc_file_with_infos(temp_file_path);
			to_file = to_files->get_file_with_info();
			from_file->set_partial_path(to_file->get_partial_path());

			// make binomial
			binomial = new tc_files_binomial(from_file, to_file);
			current_partial_path = current_to_path;

			// free pointers
			free(temp_base_folder);

			from_remove_ok = false;
			to_remove_ok = true;
		}
		binomial->from_dif = from_dif;
		binomial->to_dif = to_dif;
		// now that binomial is ready, test possible cases and act
		_files_compare			compare_result;
		if(md5sum_compare)
			compare_result = binomial->md5sum_compare();
		else
			compare_result = binomial->copy_compare(check_md5sum_on_modify);
		switch(compare_result)
		{
			case _from_modified:
			case _both_modified_from_recent:
			case _to_modified:
			case _both_modified_to_recent:
			case _to_deleted:
				log.start_action(_copy_from_to_to, binomial);
				if(binomial->copy(_binomial_from, move_replaced_to_trash))
				{
					// save database dates
					from_file->refresh();
					to_file->refresh();
					configuration.refresh_file_datas(current_partial_path, from_file, to_file);
				}
				else
				{
					if(!ask_at_the_end)
					{
						while(!g_mutex_trylock(execute :: ask_mutex));
						switch(exchanges->asking_error->asking(log.get_error()))
						{
							case _error_choose_cancel:
								exchanges->sync_step.set(_cancel);
							break;

							default:
							break;
						}
						g_mutex_unlock(execute :: ask_mutex);
					}
				}
			break;

			case _from_deleted:
				// in case of folder containing items remove down items
				if(from_remove_ok)
					from_files->remove_down();
				if(to_remove_ok)
					to_files->remove_down();

				if(!ask_at_the_end || automatic_suppr)
				{
					while(!g_mutex_trylock(execute :: ask_mutex));
					switch(exchanges->asking_deletion->asking(g_uri_unescape_string(binomial->get_from()->get_uri(), NULL)))
					{
						case _delete_choose_yes:
							if(move_supr_to_trash)
								log.start_action(_trash_to, binomial);
							else
								log.start_action(_delete_to, binomial);
							if(binomial->remove(_binomial_to, move_supr_to_trash, &exchanges->num_files, &exchanges->cancel))
							{
								// save database dates
								configuration.refresh_file_datas(current_partial_path, NULL, NULL);
							}
							else
							{
								if(exchanges->cancel.get())
									exchanges->sync_step.set(_cancel);
								else
									switch(exchanges->asking_error->asking(log.get_error()))
									{
										case _error_choose_cancel:
											exchanges->sync_step.set(_cancel);
										break;

										default:
										break;
									}
							}
						break;

						case _delete_choose_cancel:
							exchanges->sync_step.set(_cancel);
						break;

						default:
						break;
					}
					g_mutex_unlock(execute :: ask_mutex);
				}
				else
				{
					if(move_supr_to_trash)
						log.start_action(_ask_trash, binomial);
					else
						log.start_action(_ask_delete, binomial);
					log.close_action();
				}
			break;


			case _both_deleted:
				configuration.refresh_file_datas(current_partial_path, NULL, NULL);
			break;

			case _both_modified_equal:
				from_file->refresh();
				to_file->refresh();
				configuration.refresh_file_datas(current_partial_path, from_file, to_file);
			break;

			case _comparison_error:
				log.start_action(_compare, binomial);
				switch(exchanges->asking_error->asking(log.get_error()))
				{
					case _error_choose_cancel:
						exchanges->sync_step.set(_cancel);
					break;

					default:
					break;
				}
				log.close_action();
			break;

			case _files_not_modified:
			break;
		}
		log.close_action();

		// free pointers
		if(from_not_at_the_end)
			free(current_from_path);
		if(to_not_at_the_end)
			free(current_to_path);
		delete from_file;
		delete to_file;
		delete binomial;

		// next files in lists
		if(partial_path_compare == 0)
		{
			if(from_not_at_the_end)
			{
				from_not_at_the_end = from_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
			if(to_not_at_the_end)
			{
				to_not_at_the_end = to_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
		else if(partial_path_compare < 0)
		{
			if(from_not_at_the_end)
			{
				from_not_at_the_end = from_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
		else if(partial_path_compare > 0)
		{
			if(to_not_at_the_end)
			{
				to_not_at_the_end = to_files->next();
				exchanges->num_files_scanned.set(exchanges->num_files_scanned.get() + 1);
			}
		}
	}
	configuration.clean_deleted_files();
	configuration.unlock();
	if(exchanges->sync_step.get() == _cancel)
	{
		log.start_action(_abort, NULL);
		log.close_action();
	}
	exchanges->sync_step.set(_terminated);
	cout<<exchanges->num_files_scanned.get()<<" files scanned."<<endl;
	log.start_action(_end, NULL);
	log.close_action();

	if(configuration.behavior->get_ask_at_the_end())
	{
		char			cmd[255];
		char			launch_time_str[255];

		strcpy(cmd, "synchrorep --pending-questions ");
		strcat(cmd, configuration.get_from_folder_uri());
		strcat(cmd, " ");
		gcvt(launch_time, 15, launch_time_str);
		strcat(cmd, launch_time_str);
		strcat(cmd, " &");
		system(cmd);
	}

	return NULL;
}


gpointer
execute :: progression_window(gpointer pexchanges)
{
	execute_exchanges	**exchanges = (execute_exchanges**)pexchanges;

	// count progress needed
	int						num_synchronizations = 0;
	while(exchanges[num_synchronizations] != NULL)
		num_synchronizations++;

	// progress window
	GtkWidget				*progress_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(progress_window), "Synchrorep");
	gtk_window_set_icon_from_file(GTK_WINDOW(progress_window), application :: get_file_path(ICON_FILE), NULL);
	gtk_window_set_resizable(GTK_WINDOW(progress_window), true);
	gtk_window_set_default_size(GTK_WINDOW(progress_window), 200, 10);
	GtkWidget				*vbox = gtk_vbox_new(true, 1);
	gtk_container_add(GTK_CONTAINER(progress_window), vbox);
	// add progress bars
	int						i;
	for(i=0; i < num_synchronizations;i++)
	{
		ac_config				config(exchanges[i]->command_line_parameter.get());
		char					*from_folder = cut_uri(g_uri_unescape_string(config.get_from_folder_uri(), NULL), 40);
		char					*to_folder = cut_uri(g_uri_unescape_string(config.get_to_folder_uri(), NULL), 40);
		char					message[200] = "";
		strcat(message, from_folder);
		strcat(message, gettext(" with "));
		strcat(message, to_folder);
		exchanges[i]->progress = gtk_progress_bar_new();
		exchanges[i]->progress_name = gtk_label_new(message);
		gtk_box_pack_start(GTK_BOX(vbox), exchanges[i]->progress_name, false, false, 1);
		gtk_box_pack_start(GTK_BOX(vbox), exchanges[i]->progress, false, false, 1);
	}
	GtkWidget				*cancel_btn = gtk_button_new_with_label(gettext("Cancel"));
	gtk_box_pack_start(GTK_BOX(vbox), cancel_btn, false, false, 1);
	gtk_widget_show_all(progress_window);
	g_signal_connect(G_OBJECT(progress_window), "destroy", (GCallback)execute :: progression_window_close, exchanges);
	g_signal_connect(G_OBJECT(cancel_btn), "clicked", (GCallback)execute :: progression_cancel, exchanges);

	g_timeout_add(50, execute :: progression_window_hook, exchanges);

	gtk_main();

	cout<<"end"<<endl;

	return NULL;
}

void
execute :: progression_cancel(GtkWidget *window_widnget, gpointer data)
{
	execute_exchanges			**exchanges = (execute_exchanges**)data;
	int								i = 0;
	while(exchanges[i] != NULL)
	{
		if(exchanges[i]->sync_step.get() != _terminated)
			exchanges[i]->sync_step.set(_cancel);
		exchanges[i]->cancel.set(true);
		i++;
	}
}

void
execute :: progression_window_close(GtkWidget *window_widnget, gpointer data)
{
	execute_exchanges			**exchanges = (execute_exchanges**)data;
	int								i = 0;
	while(exchanges[i] != NULL)
	{
		exchanges[i]->sync_step.set(_window_closed);
		exchanges[i]->cancel.set(true);
		i++;
	}
}

gboolean
execute :: progression_window_hook(gpointer data)
{
	execute_exchanges			**exchanges = (execute_exchanges**)data;
	int								i = 0;
	int								terminated = 0;

	while(exchanges[i] != NULL)
	{
		GtkWidget						*progress = exchanges[i]->progress;
		ostringstream		msg;

		// dialogs
		exchanges[i]->asking_deletion->asked();
		exchanges[i]->asking_file_conflict->asked();
		exchanges[i]->asking_error->asked();
		exchanges[i]->asking_info->asked();

		// steps management
		switch(exchanges[i]->sync_step.get())
		{
		case _analyse_step:
			msg<<gettext("Scanning folder")<<"...";
			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), msg.str().c_str());
			gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
		break;

		case _cancel:
			if(exchanges[i]->progress != NULL)
			{
				msg<<gettext("Canceling...");
				gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), msg.str().c_str());
				gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress));
			}
		break;

		case _synchronize_step:
			float			percent;
			if(exchanges[i]->num_files.get() != 0)
				percent = ((float)exchanges[i]->num_files_scanned.get()/(float)exchanges[i]->num_files.get());
			else
				percent = 1;
			gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), percent);
			msg<<gettext("Synchronization...")<<(int)(percent*100)<<"%";

			gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), msg.str().c_str());
		break;

		case _window_closed:
			exchanges[i]->sync_step.set(_cancel);
			exchanges[i]->progress = NULL;
			terminated++;
		break;

		case _terminated:
			gtk_widget_hide(exchanges[i]->progress);
			gtk_widget_hide(exchanges[i]->progress_name);
			gdk_window_resize(gtk_widget_get_parent_window(exchanges[i]->progress_name), 1, 1);
			terminated++;
		break;

		default:
		break;
		}

		i++;
	}

	// all synchronizations are terminated
	if(terminated == i)
	{
		gtk_main_quit();
		return false;
	}

	return true;
}
