/*
 * ac_config.cpp
 *
 *  Created on: 8 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011 - Sébastien Kus
 *
 *  driving synchronization settings
 */

#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "../applicative/application.h"

#include "ac_config.h"

#include "../technical/tc_misc.h"

ac_config :: ac_config(int id) : ac_common()
{
	this->id = id;
	char			sql[1000];
	char			int_to_char[11];
	gcvt(this->id, 10, int_to_char);

	strcpy(sql, "select * from config where key = ");
	strcat(sql, int_to_char);

	this->init(sql);
}

ac_config :: ac_config(char *folder) : ac_common()
{
	GFile				*gfile = g_file_new_for_commandline_arg(folder);
	char				*folder_uri = g_file_get_uri(gfile);
	char				*folder_path = g_file_get_path(gfile);
	char				sql[strlen(folder_uri)+1000];

	sql[0] = '\0';
	strcat(sql, "select * from config where from_folder = '");
	strcat(sql, this->db->format_for_db(folder_uri));
	strcat(sql, "' or to_folder='");
	strcat(sql, this->db->format_for_db(folder_uri));
	if(folder_path != NULL)
	{
		strcat(sql, "' or from_folder='");
		strcat(sql, this->db->format_for_db(folder_path));
		strcat(sql, "' or to_folder='");
		strcat(sql, this->db->format_for_db(folder_path));
		strcat(sql, "'");
	}
	else
		strcat(sql, "'");
	if(!this->init(sql))
	{
		// setting not found, make it incomplete
		this->id = -1;
		this->from_folder = gfile;
		this->to_folder = NULL;
		this->path_mode = _unknown;
		this->group = strdup("");
		this->mode = _unknown_mode;
		this->alteration = _ac_create;
		this->behavior = new ac_behavior(folder_uri);
	}

	// delete local pointers
	g_free(folder_uri);
	g_free(folder_path);
}

bool
ac_config :: init(char *sql)
{
	char				*loc_from_folder_of_db;
	char				*to_folder_of_db;
	cursor				*config_cursor;
	char				*path_mode_string;
	char				*mode_string;

	this->from_folder_uri = NULL;
	this->to_folder_uri = NULL;

	config_cursor = this->db->execute(sql);
	if(config_cursor->fetch(FETCH_FIRST))
	{
		// setting found
		// get id
		this->id = convert_string_to_int(config_cursor->current_row_get_result_by_column("key"));

		// get from_folder from db and set class's to sqlite format
		loc_from_folder_of_db = config_cursor->current_row_get_result_by_column("from_folder");
		this->from_folder_of_db = this->db->format_for_db(loc_from_folder_of_db);

		// create folders
		this->from_folder = g_file_new_for_commandline_arg(loc_from_folder_of_db);
		to_folder_of_db = config_cursor->current_row_get_result_by_column("to_folder");
		this->to_folder = g_file_new_for_commandline_arg(to_folder_of_db);

		// get type of path mode
		path_mode_string = config_cursor->current_row_get_result_by_column("path_mode");
		if(strcmp(path_mode_string, (char*)"uri") == 0)
			this->path_mode = _uri;
		else if(strcmp(path_mode_string, (char*)"path") == 0)
			this->path_mode = _path;
		else
			this->path_mode = _unknown;

		// group
		this->group = config_cursor->current_row_get_result_by_column("groups");

		// mode
		mode_string = config_cursor->current_row_get_result_by_column("mode");
		if(strcmp(mode_string, "sync") == 0)
			this->mode = _sync;
		else
			this->mode = _copy;
		free(mode_string);
		// alteration initialization
		this->alteration = _ac_unchanged;

		// create behaviour
		this->behavior = new ac_behavior(this->from_folder_of_db);

		// delete that we don't need anymore
		free(loc_from_folder_of_db);
		free(to_folder_of_db);

		// test if uri mode, else switch to uri mode (if both folders are accessible)
		if(this->path_mode == _path && this->ready_to_start())
		{
			char			*from_folder_string;
			char			*to_folder_string;
			char			*from_folder_sqlite;
			char			*to_folder_sqlite;
			char			*path_mode_string;
			char			*sql;

			// make database strings
			from_folder_string = g_file_get_uri(this->from_folder);
			from_folder_sqlite = this->db->format_for_db(from_folder_string);
			g_free(from_folder_string);
			to_folder_string = g_file_get_uri(this->to_folder);
			to_folder_sqlite = this->db->format_for_db(to_folder_string);
			g_free(to_folder_string);
			// make path_mode string
			path_mode_string = strdup("uri");
			// make sql string
			sql = new char[strlen(from_folder_sqlite)+strlen(to_folder_sqlite)+strlen(path_mode_string)+500];
			sql[0] = '\0';
			sql = strcat(sql, "update config set from_folder='");
			sql = strcat(sql, from_folder_sqlite);
			sql = strcat(sql, "', to_folder='");
			sql = strcat(sql, to_folder_sqlite);
			sql = strcat(sql, "', path_mode='");
			sql = strcat(sql, path_mode_string);
			sql = strcat(sql, "' where from_folder='");
			sql = strcat(sql, this->from_folder_of_db);
			sql = strcat(sql, "'");
			// execute
			this->db->execute(sql);
			// set class path type
			this->path_mode = _uri;

			// delete local pointers
			free(from_folder_sqlite);
			free(to_folder_sqlite);
			free(path_mode_string);
			free(sql);
		}
	}
	else
	{
		return false;
	}

	return true;
}

ac_config :: ~ac_config()
{
	// purge database from both deleted
	this->db->execute("delete from files where date=0 and to_date=0");

	// deleting properties
	free(this->from_folder_of_db);
	free(this->group);
	delete this->behavior;
}

// construct by duplication
ac_config :: ac_config(const ac_config& rhs) : ac_common(rhs)
{
	this->id = rhs.id;
	this->from_folder_of_db = strdup(rhs.from_folder_of_db);
	this->from_folder=rhs.from_folder;
	this->to_folder=rhs.to_folder;
	this->path_mode=rhs.path_mode;
	this->group = strdup(rhs.group);
	this->mode = rhs.mode;
	this->behavior = new ac_behavior(*rhs.behavior);
}

// duplication by operation =
ac_config&
ac_config :: operator=(const ac_config& rhs)
{
	if(this == &rhs)
		return *this;

	this->id = rhs.id;
	this->from_folder_of_db = strdup(rhs.from_folder_of_db);
	this->from_folder=rhs.from_folder;
	this->to_folder=rhs.to_folder;
	this->path_mode=rhs.path_mode;
	this->group = strdup(rhs.group);
	this->mode = rhs.mode;
	this->behavior = new ac_behavior(*rhs.behavior);

	return *this;
}

GFile*
ac_config :: get_from_folder()
{
		return this->from_folder;
}

char*
ac_config :: get_from_folder_uri()
{
	if(this->from_folder_uri == NULL)
		this->from_folder_uri = g_file_get_uri(this->from_folder);

	return this->from_folder_uri;
}

GFile*
ac_config :: get_to_folder()
{
	return this->to_folder;
}

char*
ac_config :: get_to_folder_uri()
{
	if(this->to_folder_uri == NULL)
		this->to_folder_uri = g_file_get_uri(this->to_folder);

	return this->to_folder_uri;
}


// to_folder is modified only on creation and before first successful commit
bool
ac_config :: set_to_folder(char *folder)
{
	if(this->alteration == _ac_create)
	{
		GFile				*new_gfile = g_file_new_for_commandline_arg(folder);
		bool				result = this->set_to_folder(new_gfile);

		return result;
	}
	return false;
}

bool
ac_config :: set_to_folder(GFile *folder)
{
	if(this->alteration == _ac_create)
	{
		this->to_folder = folder;
		return true;
	}
	return false;
}

bool
ac_config :: trylock()
{
	char			sql[1000];
	bool			success = false;

	strcpy(sql, "select lock from config where from_folder = '");
	strcat(sql, this->from_folder_of_db);
	strcat(sql, "'");

	cursor			*resultset = this->db->execute(sql);
	if(resultset->fetch(FETCH_FIRST))
	{
		char				*lock_pid = resultset->current_row_get_result_by_index(0);
		if(lock_pid[0] == '\0')
			// if no lock, success
			success = true;
		else
		{
			// test if locked pid exists
			char			cmd[100];
			char			*output;
			char			*outerr;
			int				status;
			strcpy(cmd, "ps --no-header ");
			strcat(cmd, lock_pid);
			g_spawn_command_line_sync(cmd, &output, &outerr, &status, NULL);
			cout<<output<<endl;
			if(output[0] == '\n' || output[0] == '\0')
				// if not, success
				success = true;
		}
	}

	// if success lock the config to the process
	if(success)
	{
		strcpy(sql, "update config set lock = '");
		strcat(sql, application :: get_pid());
		strcat(sql, "' where from_folder = '");
		strcat(sql, this->from_folder_of_db);
		strcat(sql, "'");
		this->db->execute(sql);

		return true;
	}
	else
		return false;
}

void
ac_config :: unlock()
{
	char			sql[1000];
	strcpy(sql, "update config set lock = '' where from_folder = '");
	strcat(sql, this->from_folder_of_db);
	strcat(sql, "'");
	this->db->execute(sql);
}

char*
ac_config :: get_group()
{
	return strdup(this->group);
}

int
ac_config :: get_id()
{
	return this->id;
}

char**
ac_config :: get_groups()
{
	cursor			*sql_result = ac_common :: db->execute("select distinct groups from config order by groups");
	int				rows = sql_result->get_num_rows();
	char			**result = new char*[rows+1];
	int				i = 0;
	sql_result->fetch(FETCH_FIRST);
	while(i < rows)
	{
		result[i] = sql_result->current_row_get_result_by_index(0);
		sql_result->fetch(FETCH_NEXT);
		i++;
	}
	result[i] = NULL;

	return result;
}

bool
ac_config :: set_group(char *new_group)
{
	// test new group is alphanumeric
	if(!test_alphanumeric(new_group))
		return false;

	// test if new group has a good len for db
	if(strlen(new_group) > 100)
		return false;

	// set new group
	free(this->group);
	this->group = strdup(new_group);
	this->alteration = _ac_modified;
	return true;
}

folders_accessible_type
ac_config :: accessibility()
{
	gboolean		from_folder_exists,
					to_folder_exists;

	from_folder_exists = g_file_query_exists(this->from_folder, NULL);
	if(this->to_folder != NULL)
		to_folder_exists = g_file_query_exists(this->to_folder, NULL);
	else
		to_folder_exists = false;

	if(from_folder_exists && to_folder_exists)
		return _both_accessible;
	else if(from_folder_exists)
		return _to_folder_missing;
	else if(to_folder_exists)
		return _from_folder_missing;
	else
		return _both_missing;
}

bool
ac_config :: ready_to_start()
{
	return (this->accessibility() == _both_accessible);
}

GTimeVal
ac_config :: get_from_file_db_time(char *partial_path)
{
	char			*sql = new char[1000 + strlen(this->from_folder_of_db) + strlen(partial_path)];
	strcpy(sql, "select date from files where from_folder = '");
	strcat(sql, this->from_folder_of_db);
	strcat(sql, "' and file = '");
	strcat(sql, this->db->format_for_db(partial_path));
	strcat(sql, "'");
	cursor			*result = this->db->execute(sql);
	GTimeVal		result_time = {0, 0};
	if(result->fetch(FETCH_FIRST))
	{
		result_time.tv_sec = convert_string_to_int(result->current_row_get_result_by_index(0));
	}

	return result_time;
}

GTimeVal
ac_config :: get_to_file_db_time(char *partial_path)
{
	char			*sql = new char[1000 + strlen(this->from_folder_of_db) + strlen(partial_path)];
	strcpy(sql, "select to_date from files where from_folder = '");
	strcat(sql, this->from_folder_of_db);
	strcat(sql, "' and file = '");
	strcat(sql, this->db->format_for_db(partial_path));
	strcat(sql, "'");
	cursor			*result = this->db->execute(sql);
	GTimeVal		result_time = {0, 0};
	if(result->fetch(FETCH_FIRST))
	{
		result_time.tv_sec = convert_string_to_int(result->current_row_get_result_by_index(0));
	}

	return result_time;
}

bool
ac_config :: commit()
{
	char			*from_folder_string;
	char			*to_folder_string;
	char			*from_folder_sqlite;
	char			*to_folder_sqlite;
	char			*path_mode_string;
	char			*sql;
	bool			result = false;

	// normal cases
	switch(this->alteration)
	{
		case _ac_create:
			// test both folders are accessible
			if(!this->ready_to_start())
				return false;
			// make database strings
			from_folder_string = g_file_get_uri(this->from_folder);
			from_folder_sqlite = this->db->format_for_db(from_folder_string);
			g_free(from_folder_string);
			to_folder_string = g_file_get_uri(this->to_folder);
			to_folder_sqlite = this->db->format_for_db(to_folder_string);
			g_free(to_folder_string);
			path_mode_string = strdup("uri");
			// make sql string
			sql = new char[strlen(from_folder_sqlite)+strlen(to_folder_sqlite)+strlen(path_mode_string)+500];
			sql[0] = '\0';
			strcat(sql, "insert into config values(null, '");
			strcat(sql, from_folder_sqlite);
			strcat(sql, "', '");
			strcat(sql, to_folder_sqlite);
			strcat(sql, "', '");
			strcat(sql, path_mode_string);
			strcat(sql, "', '");
			strcat(sql, this->group);
			strcat(sql, "', '', ");
			if(this->mode == _sync)
				strcat(sql, "'sync')");
			else
				strcat(sql, "'copy')");
			// execute
			this->db->execute(sql);
			if(this->db->get_error_code() == _no_error)
			{
				// set from_folder_of_db to new sqlite string
				this->from_folder_of_db = strdup(from_folder_sqlite);
				// set alteration to unchanged
				this->alteration = _ac_unchanged;

				// commit done
				result = true;
			}

			// free memory
			free(from_folder_sqlite);
			free(to_folder_sqlite);
			free(path_mode_string);
			free(sql);
		break;

		case _ac_deleted:
			// make database strings
			from_folder_string = g_file_get_uri(this->from_folder);
			from_folder_sqlite = this->db->format_for_db(from_folder_string);
			g_free(from_folder_string);
			// make sql string
			sql = new char[strlen(from_folder_sqlite)+500];
			sql[0] = '\0';
			strcat(sql, "delete from config where from_folder = '");
			strcat(sql, from_folder_sqlite);
			strcat(sql, "'");
			this->db->execute(sql);
			if(this->db->get_error_code() == _no_error)
			{
				// make sql string
				sql = new char[strlen(from_folder_sqlite)+500];
				sql[0] = '\0';
				strcat(sql, "delete from files where from_folder = '");
				strcat(sql, from_folder_sqlite);
				strcat(sql, "'");
				this->db->execute(sql);
				if(this->db->get_error_code() == _no_error)
				{
					// invalidate object property
					if(this->from_folder_of_db != NULL)
						free(this->from_folder_of_db);
					this->from_folder_of_db = NULL;
					this->from_folder = NULL;
					this->to_folder = NULL;
					this->path_mode = _unknown;
					this->group = NULL;
					// set alteration to unchanged
					this->alteration = _ac_invalid;
					// delete associated behaviour
					this->behavior->set_to_delete();
					if(this->behavior->commit())
						// commit done succesfully
						result = true;
				}
			}

			// free memory
			free(from_folder_sqlite);
			free(sql);
		break;

		case _ac_modified:
			sql = new char[strlen(from_folder_sqlite)+500];
			sql[0] = '\0';
			strcat(sql, "update config set groups='");
			strcat(sql, this->group);
			strcat(sql, "' where from_folder='");
			strcat(sql, this->from_folder_of_db);
			strcat(sql, "'");
			this->db->execute(sql);
			if(this->db->get_error_code() == _no_error)
			{
				this->alteration = _ac_unchanged;
				result = true;
			}
		break;

		case _ac_unchanged:
			result = true;
		break;

		default:
		break;
	}

	return result;
}

void
ac_config :: dump()
{
	char			*data;

	cout<<"Dumping configuration : "<<this->from_folder_of_db<<endl;

	data= NULL;
	data = this->get_from_folder_uri();
	cout<<"From folder : ";
	if(data != NULL)
	{
		cout<<data<<endl;
		g_free(data);
	}
	else
		cout<<"Unreachable"<<endl;

	data = NULL;
	data = this->get_to_folder_uri();
	cout<<"To folder : ";
	if(data != NULL)
	{
		cout<<data<<endl;
		g_free(data);
	}
	else
		cout<<"Unreachable"<<endl;

	if(!this->behavior->get_alteration() != _ac_invalid)
		this->behavior->dump();
}

config_mode
ac_config :: get_mode()
{
	return this->mode;
}

void
ac_config :: set_mode(config_mode mode)
{
	if(this->mode == _unknown_mode)
		this->mode = mode;
}


// class ac_config_list
ac_config_list :: ac_config_list(char *group) : ac_common()
{
	char			sql[500] = "select from_folder, groups from config";
	cursor			*result;
	char			*from_folder;
	char			*db_group;

	result = this->db->execute(sql);
	if(result->fetch(FETCH_FIRST))
		do
		{
			from_folder = result->current_row_get_result_by_column("from_folder");
			db_group = result->current_row_get_result_by_column("groups");
			if(group == NULL || strcmp(group, db_group) == 0)
				this->config_list.push_back(new ac_config(from_folder));
			free(from_folder);
			free(db_group);
		} while(result->fetch());

	this->begin();
}

ac_config_list :: ~ac_config_list()
{
	int					i;
	this->begin();
	for(i= this->config_list.size(); i>0; i--)
		this->remove();
}

bool
ac_config_list :: begin()
{
	if((int)this->config_list.size() > 0)
	{
		this->config_list_it = this->config_list.begin();
		this->config_list_pos = 1;
		return true;
	}
	else
		return false;
}

bool
ac_config_list :: end()
{
	if((int)this->config_list.size() > 0)
	{
		this->config_list_it = this->config_list.end();
		this->config_list_pos = (int)this->config_list.size();
		return true;
	}
	else
		return false;
}

bool
ac_config_list :: next()
{
	if(this->config_list_pos < (int)this->config_list.size() && (int)this->config_list.size() > 0)
	{
		this->config_list_it++;
		this->config_list_pos++;
		return true;
	}
	else
		return false;
}

bool
ac_config_list :: previous()
{
	if(this->config_list_pos > 1 && (int)this->config_list.size() > 0)
	{
		this->config_list_it--;
		this->config_list_pos--;
		return true;
	}
	else
		return false;
}

bool
ac_config_list :: goto_position(unsigned int index)
{
	if(this->begin())
	{
		if(index == 0)
			return true;
		index--;
		while(this->next())
		{
			if(index == 0)
				return true;
			index--;
		}
	}

	return false;
}

void
ac_config_list :: add(ac_config new_config)
{
	this->config_list.push_back(new ac_config(new_config));
}

void
ac_config_list :: remove()
{
	delete *this->config_list_it;
	this->config_list_it = this->config_list.erase(this->config_list_it);
	if(this->config_list_pos > (int)this->config_list.size())
		this->end();
}

ac_config*
ac_config_list :: get_current()
{
	return *this->config_list_it;
}

bool
ac_config_list :: find(char *folder)
{
	GFile							*to_find = g_file_new_for_commandline_arg(folder);

	this->begin();
	do
	{
		if(g_file_equal((*this->config_list_it)->from_folder, to_find))
			return true;
	} while(this->next());

	return false;
}

bool
ac_config_list :: folder_configured(char *folder)
{
	if((int)this->config_list.size() == 0)
		return false;

	list<ac_config*> :: iterator	temp_it = this->config_list.begin();
	GFile							*folder_gfile = g_file_new_for_commandline_arg(folder);
	char							*conf_folder_uri,
									*folder_uri;

	do
	{
		// compare from_folder
		conf_folder_uri = (*temp_it)->get_from_folder_uri();
		folder_uri = g_file_get_uri(folder_gfile);
		if(strlen(conf_folder_uri) < strlen(folder_uri))
		{
			if(folder_uri[strlen(conf_folder_uri)] == '/')
				folder_uri[strlen(conf_folder_uri)] = '\0';
		}
		else
		{
			if(conf_folder_uri[strlen(conf_folder_uri)] == '/')
				conf_folder_uri[strlen(folder_uri)] = '\0';
		}
		if(strcmp(folder_uri, conf_folder_uri) == 0)
			return true;
		//free(conf_folder_uri);
		g_free(folder_uri);

		// compare to_folder
		conf_folder_uri = (*temp_it)->get_to_folder_uri();
		folder_uri = g_file_get_uri(folder_gfile);
		if(strlen(conf_folder_uri) < strlen(folder_uri))
		{
			if(folder_uri[strlen(conf_folder_uri)] == '/')
				folder_uri[strlen(conf_folder_uri)] = '\0';
		}
		else
		{
			if(conf_folder_uri[strlen(conf_folder_uri)] == '/')
				conf_folder_uri[strlen(folder_uri)] = '\0';
		}
		if(strcmp(folder_uri, conf_folder_uri) == 0)
			return true;
		//free(conf_folder_uri);
		g_free(folder_uri);
		temp_it++;
	} while(temp_it != this->config_list.end());

	return false;
}

// files accessors
void
ac_config :: files(tc_find_files **pfrom, tc_find_files **pto, mutex_var<bool> *cancel)
{
	tc_find_files *from = NULL;
	tc_find_files *to = NULL;

	// can't process files if not ready to start
	if(!this->ready_to_start())
		return;

	from = new tc_find_files(this->from_folder, cancel);
	to = new tc_find_files(this->to_folder, cancel);
	GThread			*from_thread = from->process_t();
	GThread			*to_thread = to->process_t();
	g_thread_join(from_thread);
	g_thread_join(to_thread);

	// add database informations to file list
	char					sql[strlen(this->from_folder_of_db)+500];
	cursor					*files_cursor;
	char					*modification_date_string,
							*file_partial_path;
	long					modification_time_from;
	long					modification_time_to;
	GTimeVal				g_modification_time;

	if(!cancel->get())
	{
		// read database
		sql[0] = '\0';
		strcat(sql, "select file, date, to_date, from_md5sum, to_md5sum from files where from_folder='");
		strcat(sql, this->from_folder_of_db);
		strcat(sql, "' order by file");
		files_cursor = this->db->execute(sql);

		// sorting for performances reason and position to first
		if(files_cursor->fetch(FETCH_FIRST))
			do
			{
				// build GFile for current file
				file_partial_path = files_cursor->current_row_get_result_by_index(0);
				// get database modification date
				modification_date_string = files_cursor->current_row_get_result_by_index(1);
				modification_time_from = convert_string_to_int(modification_date_string);
				free( modification_date_string);
				modification_date_string = files_cursor->current_row_get_result_by_index(2);
				modification_time_to = convert_string_to_int(modification_date_string);
				free( modification_date_string);
				// find next match for from
				g_modification_time.tv_sec = modification_time_from;
				g_modification_time.tv_usec = 0;
				from->add_db_data(file_partial_path, g_modification_time);
				from->get_file_with_info()->set_db_md5(files_cursor->current_row_get_result_by_index(3));
				g_modification_time.tv_sec = modification_time_to;
				g_modification_time.tv_usec = 0;
				to->add_db_data(file_partial_path, g_modification_time);
				to->get_file_with_info()->set_db_md5(files_cursor->current_row_get_result_by_index(4));
				free( file_partial_path);
			} while(files_cursor->fetch() && !cancel->get());
	}
	*pfrom = from;
	*pto = to;
}

bool
ac_config :: refresh_file_datas(char *partial_path, tc_file_with_infos *from_infos, tc_file_with_infos *to_infos)
{
	char					sql[strlen(this->from_folder_of_db)+strlen(partial_path)+1000];
	char					*db_partial_path = this->db->format_for_db(partial_path);
	char					int_to_char[15];
	cursor					*resultset;

	// files have been deleted
	if(from_infos == NULL ||
			(from_infos->get_modification_time().tv_sec == 0 &&
			to_infos->get_modification_time().tv_sec == 0))
	{
		sql[0] = '\0';
		strcat(sql, "delete from files where from_folder='");
		strcat(sql, this->from_folder_of_db);
		strcat(sql, "' and file='");
		strcat(sql, db_partial_path);
		strcat(sql, "' or file like '");
		strcat(sql, db_partial_path);
		strcat(sql, "/%'");
		this->db->execute(sql);
	}
	else
	{
		// is file exists in database
		if(from_infos->get_database_modification_time().tv_sec != 0 &&
				from_infos->get_database_modification_time().tv_sec != 0)
		{
			// file exists, update it
			sql[0] = '\0';
			strcat(sql, "update files set date=");
			gcvt((long)from_infos->get_modification_time().tv_sec, 14, int_to_char);
			strcat(sql, int_to_char);
			strcat(sql, ", to_date=");
			gcvt((long)to_infos->get_modification_time().tv_sec, 14, int_to_char);
			strcat(sql, int_to_char);
			if(this->behavior->get_md5sum_level() == _database_md5sum)
			{
				char			*from_md5 = from_infos->get_md5(NULL);
				if(from_md5 == NULL)
					from_md5 = strdup("");
				char			*to_md5 = to_infos->get_md5(NULL);
				if(to_md5 == NULL)
					to_md5 = strdup("");
				strcat(sql, ", from_md5sum='");
				strcat(sql, from_md5);
				strcat(sql, "', to_md5sum='");
				strcat(sql, to_md5);
				strcat(sql, "'");
				free(from_md5);
				free(to_md5);
			}
			else
				strcat(sql, ", from_md5sum='', to_md5sum=''");
			strcat(sql, " where from_folder='");
			strcat(sql, this->from_folder_of_db);
			strcat(sql, "' and file='");
			strcat(sql, db_partial_path);
			strcat(sql, "'");
			this->db->execute(sql);
		}
		else
		{
			// file don't exists, create it
			sql[0] = '\0';
			strcat(sql, "insert into files values(null, '");
			strcat(sql, this->from_folder_of_db);
			strcat(sql, "', '");
			strcat(sql, db_partial_path);
			strcat(sql, "', ");
			gcvt((long)from_infos->get_modification_time().tv_sec, 14, int_to_char);
			strcat(sql, int_to_char);
			strcat(sql, ", ");
			gcvt((long)to_infos->get_modification_time().tv_sec, 14, int_to_char);
			strcat(sql, int_to_char);
			if(this->behavior->get_md5sum_level() == _database_md5sum)
			{
				char			*from_md5 = from_infos->get_md5(NULL);
				if(from_md5 == NULL)
					from_md5 = strdup("");
				char			*to_md5 = to_infos->get_md5(NULL);
				if(to_md5 == NULL)
					to_md5 = strdup("");
				strcat(sql, ", '");
				strcat(sql, from_md5);
				strcat(sql, "', '");
				strcat(sql, to_md5);
				strcat(sql, "'");
				free(from_md5);
				free(to_md5);
			}
			else
				strcat(sql, ", '', ''");
			strcat(sql, ")");
			this->db->execute(sql);
		}
	}

	if(this->db->get_error_code() == 0)
		return true;

	return false;
}

void
ac_config :: clean_deleted_files()
{
	char			sql[255] = "delete from files where date = 0 and to_date = 0";
	this->db->execute(sql);
}
