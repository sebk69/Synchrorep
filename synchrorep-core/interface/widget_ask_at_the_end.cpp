/*
 * widget_ask_at_the_end.cpp
 *
 *  Created on: 26 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011 - Sébastien Kus
 *
 *  Widget to ask questions of a synchronization at the end
 *
 */

#include "widget_ask_at_the_end.h"

#include "string.h"
#include <sstream>

#include "../applicative/logging.h"
#include "../accessors/ac_logs.h"

widget_ask_at_the_end :: widget_ask_at_the_end(int config_id, time_t launch_time, widget_synchronization_log *log)
{
	this->log_widget = log;
	ac_log_list						log_list(config_id);
	list<ac_log_action>				*questions_list = log_list.get_questions(launch_time);
	list<ac_log_action> :: iterator	questions_it;
	GtkWidget						*vbox;
	if(questions_list != NULL)
	{
		questions_it = questions_list->begin();
		vbox = gtk_vbox_new(false, 1);
		while(questions_it != questions_list->end())
		{
			gtk_box_pack_start(GTK_BOX(vbox), this->build_question(&(*questions_it)), false, false, 1);
			questions_it++;
		}
	}
	else
		vbox = NULL;

	// create scroll buttons
	this->bg_id = 0;
	if(vbox != NULL)
	{
		GtkWidget			*scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), vbox);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

		// set size
		gtk_widget_set_size_request(scrolled_window, -1, 500);

		this->widget = scrolled_window;
		this->answer_datas_it = this->answer_datas.begin();
		this->bg_id = g_idle_add((GSourceFunc)this->check_filesystem, (gpointer)this);
	}
	else
		this->widget = NULL;
}

widget_ask_at_the_end :: ~widget_ask_at_the_end()
{
	if(this->bg_id != 0)
		g_source_remove(this->bg_id);
}

GtkWidget*
widget_ask_at_the_end :: build_question(ac_log_action *action)
{
	// check filesystem if changed
	if(action->get_answer() == _answer_none)
		if(action->get_from_time_when_registered()
				!= action->get_binomial()->get_from()->get_modification_time().tv_sec
				|| action->get_to_time_when_registered()
				!= action->get_binomial()->get_to()->get_modification_time().tv_sec)
		{
			action->answer_question(_answer_force_ignore_filesystem);
			action->commit();
		}

	// build message string
	ostringstream		title;
	ostringstream		message;
	char				time_string[100];
	time_t				time = action->get_action_time();
	strftime(time_string, 256, "%c", localtime(&time));
	title<<underline(time_string)<<" : ";
	switch(action->get_action())
	{
		case _ask_delete:
		case _ask_trash:
			title<<gettext("A file has been deleted");
			message<<action->get_binomial()->get_from()->get_partial_path()<<" "
				<<gettext("has been removed since last synchronisation.\n Delete the other ?");
		break;

		case _ask_both:
		case _ask_both_trash:
			title<<gettext("A file has been modified in both folders");
			tc_file_with_infos	*source = action->get_binomial()->get_from();
			tc_file_with_infos	*target = action->get_binomial()->get_to();
			time_t				time_val_source = action->get_from_time_when_registered();
			time_t				time_val_target = action->get_to_time_when_registered();
			char				source_modification_string[256];
			char				target_modification_string[256];
			char				*source_folder = g_uri_unescape_string(source->get_parent_uri(), NULL);
			char				*target_folder = g_uri_unescape_string(target->get_parent_uri(), NULL);
			char				*file_name = source->get_base_name();

			strftime(source_modification_string, 256, "%c", localtime(&time_val_source));
			strftime(target_modification_string, 256, "%c", localtime(&time_val_target));

			message<<gettext("The file ")<<file_name<<gettext(" has been modified in both directories since last synchronization")<<"\n\n";
			message<<gettext("Source folder")<<" : "<<source_folder<<"\n("<<gettext("Modified on")<<" "<<source_modification_string<<")\n\n";
			message<<gettext("Target folder")<<" : "<<target_folder<<"\n("<<gettext("Modified on")<<" "<<target_modification_string<<")";

			delete source;
			delete target;
			free(source_folder);
			free(target_folder);
			free(file_name);
	}
	char				*c_message = (char *)message.str().c_str();
	GtkWidget			*message_label = gtk_label_new(cut_line_word(c_message, 300));
	gtk_label_set_use_underline(GTK_LABEL(message_label), true);

	// get the answer if one
	GtkWidget			*answer = gtk_label_new("");
	GdkColor			color;
	gdk_color_parse("darkgreen", &color);
	gtk_widget_modify_fg (answer, GTK_STATE_NORMAL, &color);
	GtkWidget			*question = gtk_hbox_new(false, 1);
	answering_data		*data = new answering_data;
	*data = {answer, question, action, this};

	if(action->get_answer() != _answer_none)
	{
		char				answer_str[1000];
		strcpy(answer_str, "\n");
		strcat(answer_str, action->get_answer_string());
		gtk_label_set_text(GTK_LABEL(answer), answer_str);
	}
	else
	{
		// user have to answer the question
		GtkWidget			*answer_yes;
		GtkWidget			*answer_no;
		GtkWidget			*copy_recent_btn;
		GtkWidget			*copy_older_btn;
		GtkWidget			*copy_source_btn;
		GtkWidget			*copy_target_btn;
		GtkWidget			*nothing_btn;
		// what are the answer buttons
		switch(action->get_action())
		{
			case _ask_delete:
			case _ask_trash:
				answer_yes = gtk_button_new_with_label(gettext("Yes"));
				g_signal_connect(answer_yes, "clicked", G_CALLBACK(this->answer_delete_yes), data);
				gtk_box_pack_start(GTK_BOX(question), answer_yes, true, true, 1);
				answer_no = gtk_button_new_with_label(gettext("No"));
				g_signal_connect(answer_no, "clicked", G_CALLBACK(this->answer_delete_no), data);
				gtk_box_pack_start(GTK_BOX(question), answer_no, true, true, 1);
			break;

			case _ask_both:
			case _ask_both_trash:
				copy_recent_btn = gtk_button_new_with_label(gettext("Copy recent"));
				g_signal_connect(copy_recent_btn, "clicked", G_CALLBACK(this->answer_copy_recent), data);
				gtk_box_pack_start(GTK_BOX(question), copy_recent_btn, true, true, 1);
				copy_older_btn = gtk_button_new_with_label(gettext("Copy older"));
				g_signal_connect(copy_older_btn, "clicked", G_CALLBACK(this->answer_copy_older), data);
				gtk_box_pack_start(GTK_BOX(question), copy_older_btn, true, true, 1);
				copy_source_btn = gtk_button_new_with_label(gettext("Copy from source"));
				g_signal_connect(copy_source_btn, "clicked", G_CALLBACK(this->answer_copy_from), data);
				gtk_box_pack_start(GTK_BOX(question), copy_source_btn, true, true, 1);
				copy_target_btn = gtk_button_new_with_label(gettext("Copy from target"));
				g_signal_connect(copy_target_btn, "clicked", G_CALLBACK(this->answer_copy_to), data);
				gtk_box_pack_start(GTK_BOX(question), copy_target_btn, true, true, 1);
				nothing_btn = gtk_button_new_with_label(gettext("Do nothing"));
				g_signal_connect(nothing_btn, "clicked", G_CALLBACK(this->answer_ignore_modifications), data);
				gtk_box_pack_start(GTK_BOX(question), nothing_btn, true, true, 1);
			break;

			default:
			break;
		}
	}

	// assemble interface
	GtkWidget			*question_frame = gtk_frame_new(title.str().c_str());
	GtkWidget			*label = gtk_frame_get_label_widget(GTK_FRAME(question_frame));
	gtk_label_set_use_underline(GTK_LABEL(label), true);
	GtkWidget			*question_hbox = gtk_hbox_new(false, 1);
	GtkWidget			*answer_hbox = gtk_hbox_new(false, 1);
	GtkWidget			*question_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(question_frame), question_vbox);
	gtk_box_pack_start(GTK_BOX(question_hbox), message_label, false, false, 1);
	gtk_box_pack_start(GTK_BOX(question_vbox), question_hbox, false, false, 1);
	gtk_box_pack_start(GTK_BOX(answer_hbox), answer, false, false, 1);
	gtk_box_pack_start(GTK_BOX(question_vbox), answer_hbox, false, false, 1);
	if(question != NULL)
	{
		gtk_box_pack_start(GTK_BOX(question_vbox), question, false, false, 1);
		gtk_widget_show(question);
	}
	else
		gtk_widget_show(answer);
	gtk_widget_show(question_frame);
	gtk_widget_show(message_label);
	gtk_widget_show(question_vbox);

	this->answer_datas.push_back(data);

	return question_frame;
}

bool
widget_ask_at_the_end :: check_filesystem(gpointer pdata)
{
	widget_ask_at_the_end		*THIS = (widget_ask_at_the_end*)pdata;
	answering_data				*data = *THIS->answer_datas_it;

	// check filesystem if changed
	if(data->action->get_answer() == _answer_none)
		if(data->action->get_from_time_when_registered()
				!= data->action->get_binomial()->get_from()->get_modification_time().tv_sec
				|| data->action->get_to_time_when_registered()
				!= data->action->get_binomial()->get_to()->get_modification_time().tv_sec)
		{
			data->action->answer_question(_answer_force_ignore_filesystem);
			data->action->commit();
			// show answer label
			gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
			gtk_widget_show(data->answer_label);
			gtk_widget_hide(data->btn_area);
		}

	if(THIS->answer_datas_it == THIS->answer_datas.end())
		THIS->answer_datas_it = THIS->answer_datas.begin();
	else
		THIS->answer_datas_it++;
}


void
widget_ask_at_the_end :: answer_delete_yes(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	tc_files_binomial			*files = data->action->get_binomial();
	ac_config					config(data->action->get_config_id());
	bool						put_to_trash;
	from_or_to_of_binome_type	which;

	logging			log(&config, data->action->get_launch_time());
	log.set_context(_context_pending_question);

	if(files->sync_compare(false) == _from_deleted)
		which = _binomial_to;
	else
		which = _binomial_from;
	if(data->action->get_action() == _ask_delete)
		put_to_trash = false;
	else
		put_to_trash = true;

	if(which == _binomial_from && put_to_trash)
		log.start_action(_trash_from, files);
	else if(which == _binomial_from && !put_to_trash)
		log.start_action(_delete_from, files);
	else if(which == _binomial_to && put_to_trash)
		log.start_action(_trash_to, files);
	else if(which == _binomial_to && !put_to_trash)
		log.start_action(_delete_to, files);

	if(files->remove(which, put_to_trash))
	{
		data->action->answer_question(_answer_yes);
		data->action->commit();
		tc_file_with_infos			*from = files->get_from();
		tc_file_with_infos			*to = files->get_to();
		from->refresh();
		to->refresh();
		config.refresh_file_datas(from->get_partial_path(), from, to);
		gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
		gtk_widget_show(data->answer_label);
		gtk_widget_hide(data->btn_area);
	}
	else
	{
		dialog_error			asking_error(NULL);
		asking_error.asking(log.get_error());
	}
	log.close_action();
	if(data->THIS->log_widget != NULL)
		data->THIS->log_widget->build();
}

void
widget_ask_at_the_end :: answer_delete_no(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	ac_config					config(data->action->get_config_id());

	data->action->answer_question(_answer_no);
	data->action->commit();
	gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
	gtk_widget_show(data->answer_label);
	gtk_widget_hide(data->btn_area);
}

void
widget_ask_at_the_end :: answer_copy_recent(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	tc_files_binomial			*files = data->action->get_binomial();
	ac_config					config(data->action->get_config_id());
	bool						put_to_trash;
	from_or_to_of_binome_type	which;

	logging			log(&config, data->action->get_launch_time());
	log.set_context(_context_pending_question);

	if(data->action->get_from_time_when_registered() < data->action->get_to_time_when_registered())
	{
		which = _binomial_from;
		log.start_action(_copy_from_to_to, files);
	}
	else
	{
		which = _binomial_to;
		log.start_action(_copy_to_to_from, files);
	}
	if(data->action->get_action() == _ask_both)
		put_to_trash = false;
	else
		put_to_trash = true;

	if(files->copy(which, put_to_trash))
	{
		data->action->answer_question(_answer_copy_recent);
		data->action->commit();
		tc_file_with_infos			*from = files->get_from();
		tc_file_with_infos			*to = files->get_to();
		from->refresh();
		to->refresh();
		config.refresh_file_datas(from->get_partial_path(), from, to);
		gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
		gtk_widget_show(data->answer_label);
		gtk_widget_hide(data->btn_area);
	}
	else
	{
		dialog_error			asking_error(NULL);
		asking_error.asking(log.get_error());
	}
	log.close_action();
	if(data->THIS->log_widget != NULL)
		data->THIS->log_widget->build();
}

void
widget_ask_at_the_end :: answer_copy_older(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	tc_files_binomial			*files = data->action->get_binomial();
	ac_config					config(data->action->get_config_id());
	bool						put_to_trash;
	from_or_to_of_binome_type	which;

	logging			log(&config, data->action->get_launch_time());
	log.set_context(_context_pending_question);

	if(data->action->get_from_time_when_registered() < data->action->get_to_time_when_registered())
	{
		which = _binomial_to;
		log.start_action(_copy_to_to_from, files);
	}
	else
	{
		which = _binomial_from;
		log.start_action(_copy_from_to_to, files);
	}
	if(data->action->get_action() == _ask_both)
		put_to_trash = false;
	else
		put_to_trash = true;

	if(files->copy(which, put_to_trash))
	{
		data->action->answer_question(_answer_copy_older);
		data->action->commit();
		tc_file_with_infos			*from = files->get_from();
		tc_file_with_infos			*to = files->get_to();
		from->refresh();
		to->refresh();
		config.refresh_file_datas(from->get_partial_path(), from, to);
		gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
		gtk_widget_show(data->answer_label);
		gtk_widget_hide(data->btn_area);
	}
	else
	{
		dialog_error			asking_error(NULL);
		asking_error.asking(log.get_error());
	}
	log.close_action();
	if(data->THIS->log_widget != NULL)
		data->THIS->log_widget->build();
}

void
widget_ask_at_the_end :: answer_copy_from(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	tc_files_binomial			*files = data->action->get_binomial();
	ac_config					config(data->action->get_config_id());
	bool						put_to_trash;

	logging			log(&config, data->action->get_launch_time());
	log.set_context(_context_pending_question);

	if(data->action->get_action() == _ask_both)
		put_to_trash = false;
	else
		put_to_trash = true;

	log.start_action(_copy_from_to_to, files);

	if(files->copy(_binomial_from, put_to_trash))
	{
		data->action->answer_question(_answer_copy_from);
		data->action->commit();
		tc_file_with_infos			*from = files->get_from();
		tc_file_with_infos			*to = files->get_to();
		from->refresh();
		to->refresh();
		config.refresh_file_datas(from->get_partial_path(), from, to);
		gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
		gtk_widget_show(data->answer_label);
		gtk_widget_hide(data->btn_area);
	}
	else
	{
		dialog_error			asking_error(NULL);
		asking_error.asking(log.get_error());
	}
	log.close_action();
	if(data->THIS->log_widget != NULL)
		data->THIS->log_widget->build();
}

void
widget_ask_at_the_end :: answer_copy_to(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	tc_files_binomial			*files = data->action->get_binomial();
	ac_config					config(data->action->get_config_id());
	bool						put_to_trash;

	logging			log(&config, data->action->get_launch_time());
	log.set_context(_context_pending_question);

	if(data->action->get_action() == _ask_both)
		put_to_trash = false;
	else
		put_to_trash = true;

	log.start_action(_copy_to_to_from, files);

	if(files->copy(_binomial_to, put_to_trash))
	{
		data->action->answer_question(_answer_copy_to);
		data->action->commit();
		tc_file_with_infos			*from = files->get_from();
		tc_file_with_infos			*to = files->get_to();
		from->refresh();
		to->refresh();
		config.refresh_file_datas(from->get_partial_path(), from, to);
		gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
		gtk_widget_show(data->answer_label);
		gtk_widget_hide(data->btn_area);
	}
	else
	{
		dialog_error			asking_error(NULL);
		asking_error.asking(log.get_error());
	}
	log.close_action();
	if(data->THIS->log_widget != NULL)
		data->THIS->log_widget->build();
}

void
widget_ask_at_the_end :: answer_ignore_modifications(GtkWidget *btn, gpointer pdata)
{
	answering_data				*data = (answering_data*)pdata;
	ac_config					config(data->action->get_config_id());

	data->action->answer_question(_answer_ignore);
	data->action->commit();
	gtk_label_set_text(GTK_LABEL(data->answer_label), data->action->get_answer_string());
	gtk_widget_show(data->answer_label);
	gtk_widget_hide(data->btn_area);
}
