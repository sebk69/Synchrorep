/*
 * ac_logs.cpp
 *
 *  Created on: 17 mars 2010
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Logs accessor
 *
 */

#include "ac_logs.h"
#include "ac_preferences.h"

#include "../technical/tc_misc.h"

#include <string.h>
#include <iostream>
#include <sstream>
#include <time.h>

ac_log_action :: ac_log_action(int config_id, time_t launch_time, long no)
{
	this->config_id = config_id;
	this->context = _context_pending;
	this->launch_time = launch_time;
	this->no = no;
	this->error = NULL;
	this->alteration = _ac_unchanged;
	this->reset();
}

ac_log_action :: ~ac_log_action()
{
}

void
ac_log_action :: reset()
{
	this->alteration = _ac_create;
	this->action = _none;
	if(this->error != NULL)
		delete this->error;
	this->error = NULL;
}

void
ac_log_action :: answer_question(_log_answers answer)
{
	this->answer = answer;
	this->alteration = _ac_modified;
}

bool
ac_log_action :: is_asking(_log_actions_type action)
{
	switch(action)
	{
		case _ask_both:
		case _ask_both_trash:
		case _ask_delete:
		case _ask_trash:
			return true;
		break;

		default:
			return false;
	}
}

void
ac_log_action :: commit()
{
	if(this->alteration == _ac_create)
	{
		this->no++;
		char			sql[1000] = "";
		char			int_to_char[100];

		strcat(sql, "insert into logs values(");
		gcvt(this->config_id, 10, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, ", ");
		gcvt(this->launch_time, 20, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, ", ");
		gcvt(this->no, 14, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, ", '");
		switch(this->action)
		{
			case _compare:
				strcat(sql, "compare");
			break;

			case _copy_from_to_to:
				strcat(sql, "cpy_from");
			break;

			case _copy_to_to_from:
				strcat(sql, "cpy_to");
			break;

			case _delete_from:
				strcat(sql, "del_from");
			break;

			case _delete_to:
				strcat(sql, "del_to");
			break;

			case _trash_from:
				strcat(sql, "trash_from");
			break;

			case _trash_to:
				strcat(sql, "trash_to");
			break;

			case _ask_both:
				strcat(sql, "ask_both");
			break;

			case _ask_both_trash:
				strcat(sql, "ask_both_trash");
			break;

			case _ask_delete:
				strcat(sql, "ask_del");
			break;

			case _ask_trash:
				strcat(sql, "ask_trash");
			break;

			case _start:
				strcat(sql, "start");
			break;

			case _end:
				strcat(sql, "end");
			break;

			case _abort:
				strcat(sql, "abort");
			break;

			case _none:
			break;
		}
		strcat(sql, "', ");

		time_t				action_time;
		time(&action_time);
		gcvt(action_time, 20, int_to_char);
		strcat(sql, int_to_char);

		strcat(sql, ", '");
		if(this->partial_path != NULL)
		{
			char			*db_partial_path = this->db->format_for_db(this->partial_path);
			strcat(sql, db_partial_path);
			free(db_partial_path);
		}

		strcat(sql, "', ");
		gcvt(this->from_time, 20, int_to_char);
		strcat(sql, int_to_char);

		strcat(sql, ", ");
		gcvt(this->to_time, 20, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, ", '");
		switch(this->answer)
		{
			case _answer_copy_from:
				strcat(sql, "copy_from");
			break;

			case _answer_copy_to:
				strcat(sql, "copy_to");
			break;

			case _answer_copy_recent:
				strcat(sql, "copy_recent");
			break;

			case _answer_copy_older:
				strcat(sql, "copy_older");
			break;

			case _answer_yes:
				strcat(sql, "yes");
			break;

			case _answer_no:
				strcat(sql, "no");
			break;
			case _answer_ignore:
				strcat(sql, "ignore");
			break;

			case _answer_force_ignore_filesystem:
				strcat(sql, "ignore_new_sync");
			break;

			case _answer_none:
			break;
		}
		strcat(sql, "', '");

		if(this->error != NULL)
		{
			strcat(sql, this->db->format_for_db(this->error->context));
			strcat(sql, "', '");
			strcat(sql, this->db->format_for_db(this->error->reason));
		}
		else
			strcat(sql, "', '");
		strcat(sql, "')");

		this->db->execute(sql);
		this->alteration = _ac_unchanged;
	}
	else if(this->alteration == _ac_modified)
	{
		char			sql[1000] = "update logs set ask_answer = '";
		char			int_to_char[100];

		switch(this->answer)
		{
			case _answer_copy_from:
				strcat(sql, "copy_from");
			break;

			case _answer_copy_to:
				strcat(sql, "copy_to");
			break;

			case _answer_copy_recent:
				strcat(sql, "copy_recent");
			break;

			case _answer_copy_older:
				strcat(sql, "copy_older");
			break;

			case _answer_yes:
				strcat(sql, "yes");
			break;

			case _answer_no:
				strcat(sql, "no");
			break;

			case _answer_ignore:
				strcat(sql, "ignore");
			break;

			case _answer_force_ignore_filesystem:
				strcat(sql, "ignore_filesystem");
			break;

			case _answer_force_ignore_new_sync:
				strcat(sql, "ignore_new_sync");
			break;

			case _answer_none:
			break;
		}
		strcat(sql, "' where config_key = '");
		gcvt(this->config_id, 10, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, "' and launch_time = ");
		gcvt(this->launch_time, 20, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, " and no = ");
		gcvt(this->no, 14, int_to_char);
		strcat(sql, int_to_char);

		this->db->execute(sql);
		this->alteration = _ac_unchanged;
	}
}

bool
ac_log_action :: filesystem_modified()
{
	tc_files_binomial		*binomial = this->get_binomial();
	if(binomial->get_from()->get_modification_time().tv_sec != this->from_time
			|| binomial->get_to()->get_modification_time().tv_sec != this->to_time)
		return false;
	return true;
}

int
ac_log_action :: get_config_id()
{
	return this->config_id;
}

_log_actions_type
ac_log_action :: get_action()
{
	return this->action;
}

tc_files_binomial*
ac_log_action :: get_binomial()
{
	ac_config			config(this->config_id);
	char				*from_uri = config.get_from_folder_uri();
	char				*to_uri = config.get_to_folder_uri();
	char				*from_file_uri = new char[strlen(from_uri) + strlen(this->partial_path) + 2];
	char				*to_file_uri = new char[strlen(to_uri) + strlen(this->partial_path) + 2];
	strcpy(from_file_uri, from_uri);
	strcat(from_file_uri, "/");
	strcat(from_file_uri, this->partial_path);
	strcpy(to_file_uri, to_uri);
	strcat(to_file_uri, "/");
	strcat(to_file_uri, this->partial_path);
	tc_file_with_infos	*from_file = new tc_file_with_infos(g_file_new_for_uri(from_file_uri));
	tc_file_with_infos	*to_file = new tc_file_with_infos(g_file_new_for_uri(to_file_uri));
	from_file->set_partial_path(this->partial_path);
	to_file->set_partial_path(this->partial_path);

	tc_files_binomial	*binomial = new tc_files_binomial(from_file, to_file);
	binomial->set_db_times(config.get_from_file_db_time(this->partial_path), config.get_to_file_db_time(this->partial_path));

	delete from_file;
	delete to_file;
	delete from_file_uri;
	delete to_file_uri;
	g_free(from_uri);
	g_free(to_uri);

	return binomial;
}

long
ac_log_action :: get_from_time_when_registered()
{
	return this->from_time;
}

long
ac_log_action :: get_to_time_when_registered()
{
	return this->to_time;
}

time_t
ac_log_action :: get_action_time()
{
	return this->action_time;
}

_log_answers
ac_log_action :: get_answer()
{
	return this->answer;
}

char*
ac_log_action :: get_answer_string()
{
	char			*answer_str = new char[1000];
	switch(this->get_answer())
	{
		case _answer_copy_from:
			strcpy(answer_str, gettext("You have chosen copying from source folder"));
		break;

		case _answer_copy_to:
			strcpy(answer_str, gettext("You have chosen copying from target folder"));
		break;

		case _answer_copy_recent:
			strcpy(answer_str, gettext("You have chosen copying from recent file"));
		break;

		case _answer_copy_older:
			strcpy(answer_str, gettext("You have chosen copying from older file"));
		break;

		case _answer_yes:
			strcpy(answer_str, gettext("You have chosen yes"));
		break;

		case _answer_no:
			strcpy(answer_str, gettext("You have chosen no"));
		break;

		case _answer_ignore:
			strcpy(answer_str, gettext("You have chosen to do nothing"));
		break;

		case _answer_force_ignore_filesystem:
			strcpy(answer_str, gettext("The question have been ignored because the file \nhas been changed since synchronization"));
		break;

		case _answer_force_ignore_new_sync:
			strcpy(answer_str, gettext("The question have been ignored because you have started another synchronization between"));
		break;

		default:
			answer_str[0] = '\0';
	}

	return answer_str;
}

time_t
ac_log_action :: get_launch_time()
{
	return this->launch_time;
}

ac_log_list :: ac_log_list(ac_config *config)
{
	this->config_id = config->get_id();
}

ac_log_list :: ac_log_list(int config_id)
{
	this->config_id = config_id;
}

ac_log_list :: ~ac_log_list()
{
}

list<time_t>*
ac_log_list :: get_time_launch_list()
{
	char			sql[1000];
	char			int_to_char[11];

	strcpy(sql, "select distinct launch_time from logs where config_key=");
	gcvt(this->config_id, 10, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " order by launch_time desc");
	cursor			*resultset = this->db->execute(sql);

	if(resultset->fetch(FETCH_FIRST))
	{
		list<time_t>		*result = new list<time_t>;
		do
		{
			result->push_back(convert_string_to_int(resultset->current_row_get_result_by_column("launch_time")));
		} while(resultset->fetch(FETCH_NEXT));
		return result;
	}
	else
		return NULL;
}

list<log_timed_message>*
ac_log_list :: get_errors(time_t time_launch)
{
	char			sql[1000];
	char			int_to_char[21];

	strcpy(sql, "select distinct no, action_time, error_context, error_reason from logs where config_key=");
	gcvt(this->config_id, 10, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and launch_time=");
	gcvt(time_launch, 20, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and error_context<>'' and action not like '%ask%' order by action_time");
	cursor			*resultset = this->db->execute(sql);

	if(resultset->fetch(FETCH_FIRST))
	{
		list<log_timed_message>		*result = new list<log_timed_message>;
		do
		{
			log_timed_message		item;
			item.log_no = convert_string_to_int(resultset->current_row_get_result_by_column("no"));
			item.time = convert_string_to_int(resultset->current_row_get_result_by_column("action_time"));
			char					*context = resultset->current_row_get_result_by_column("error_context");
			char					*reason = resultset->current_row_get_result_by_column("error_reason");
			ostringstream			message;
			message<<context<<"\n"<<gettext("Reason : ")<<reason;
			item.message = strdup(message.str().c_str());
			result->push_back(item);
			free(context);
			free(reason);
		} while(resultset->fetch(FETCH_NEXT));
		delete resultset;
		return result;
	}
	delete resultset;
	return NULL;
}

list<log_timed_message>*
ac_log_list :: get_actions(time_t time_launch)
{
	char			sql[1000];
	char			int_to_char[21];

	strcpy(sql, "select distinct no, action_time, action, file from logs where config_key=");
	gcvt(this->config_id, 10, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and launch_time=");
	gcvt(time_launch, 20, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and error_context='' and action not like '%ask%' order by action_time");
	cursor			*resultset = this->db->execute(sql);

	if(resultset->fetch(FETCH_FIRST))
	{
		list<log_timed_message>		*result = new list<log_timed_message>;
		do
		{
			log_timed_message		item;
			item.log_no = convert_string_to_int(resultset->current_row_get_result_by_column("no"));
			item.time = convert_string_to_int(resultset->current_row_get_result_by_column("action_time"));
			char					*action = resultset->current_row_get_result_by_column("action");
			char					*file = resultset->current_row_get_result_by_column("file");
			ostringstream			message;
			if(strcmp(action, "compare") == 0)
				message<<gettext("Comparing")<<" \""<<file<<"\" "<<gettext("between folder 1 and folder 2");
			else if(strcmp(action, "cpy_from") == 0)
				message<<gettext("Copying")<<" \""<<file<<"\" "<<gettext("from folder 1 to folder 2");
			else if(strcmp(action, "cpy_to") == 0)
				message<<gettext("Copying")<<" \""<<file<<"\" "<<gettext("from folder 2 to folder 1");
			else if(strcmp(action, "del_from") == 0)
				message<<gettext("Removing definitively")<<" \""<<file<<"\" "<<gettext("from folder 1");
			else if(strcmp(action, "del_to") == 0)
				message<<gettext("Removing definitively")<<" \""<<file<<"\" "<<gettext("from folder 2");
			else if(strcmp(action, "trash_from") == 0)
				message<<gettext("Move")<<" \""<<file<<"\" "<<gettext(" to trash from folder 1");
			else if(strcmp(action, "trash_to") == 0)
				message<<gettext("Move")<<" \""<<file<<"\" "<<gettext(" to trash from folder 2");
			else if(strcmp(action, "abort") == 0)
				message<<gettext("Synchronization stopped");
			else if(strcmp(action, "start") == 0)
				message<<gettext("Synchronization start");
			else if(strcmp(action, "end") == 0)
				message<<gettext("Synchronization completed");
			item.message = strdup(message.str().c_str());
			result->push_back(item);
			free(action);
			free(file);
		} while(resultset->fetch(FETCH_NEXT));
		return result;
	}
	else
		return NULL;
}

list<ac_log_action>*
ac_log_list :: get_questions(time_t time_launch)
{
	char			sql[1000];
	char			int_to_char[21];

	strcpy(sql, "select * from logs where config_key=");
	gcvt(this->config_id, 10, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and launch_time=");
	gcvt(time_launch, 20, int_to_char);
	strcat(sql, int_to_char);
	strcat(sql, " and error_context='' and action like '%ask%' order by action_time");
	cursor			*resultset = this->db->execute(sql);
	list<ac_log_action>		*result = NULL;

	if(resultset->fetch(FETCH_FIRST))
	{
		result = new list<ac_log_action>;
		do
		{
			long					log_no = convert_string_to_int(resultset->current_row_get_result_by_column("no"));
			ac_log_action			cur_action(this->config_id, time_launch, log_no);

			// set action
			char					*action_str = resultset->current_row_get_result_by_column("action");
			if(strcmp(action_str, "ask_del") == 0)
				cur_action.action = _ask_delete;
			else if(strcmp(action_str, "ask_trash") == 0)
				cur_action.action = _ask_trash;
			else if(strcmp(action_str, "ask_both") == 0)
				cur_action.action = _ask_both;
			else if(strcmp(action_str, "ask_both_trash") == 0)
				cur_action.action = _ask_both_trash;
			free(action_str);
			// get files time when synchronize
			char					*action_time_str = resultset->current_row_get_result_by_column("action_time");
			cur_action.action_time = convert_string_to_int(action_time_str);
			free(action_time_str);
			// get files time on filesystem when synchronize
			char					*from_time_str = resultset->current_row_get_result_by_column("from_time");
			cur_action.from_time = convert_string_to_int(from_time_str);
			free(from_time_str);
			char					*to_time_str = resultset->current_row_get_result_by_column("to_time");
			cur_action.to_time = convert_string_to_int(to_time_str);
			free(to_time_str);
			// get file partial path
			cur_action.partial_path = resultset->current_row_get_result_by_column("file");
			// get answer
			char					*answer_str =  resultset->current_row_get_result_by_column("ask_answer");
			if(strcmp(answer_str, "yes") == 0)
				cur_action.answer = _answer_yes;
			else if(strcmp(answer_str, "no") == 0)
				cur_action.answer = _answer_no;
			else if(strcmp(answer_str, "copy_from") == 0)
				cur_action.answer = _answer_copy_from;
			else if(strcmp(answer_str, "copy_to") == 0)
				cur_action.answer = _answer_copy_to;
			else if(strcmp(answer_str, "copy_recent") == 0)
				cur_action.answer = _answer_copy_recent;
			else if(strcmp(answer_str, "copy_older") == 0)
				cur_action.answer = _answer_copy_older;
			else if(strcmp(answer_str, "ignore") == 0)
				cur_action.answer = _answer_ignore;
			else if(strcmp(answer_str, "ignore_filesystem") == 0)
				cur_action.answer = _answer_force_ignore_filesystem;
			else if(strcmp(answer_str, "ignore_new_sync") == 0)
				cur_action.answer = _answer_force_ignore_new_sync;
			else
				cur_action.answer = _answer_none;

			result->push_back(cur_action);
		} while (resultset->fetch(FETCH_NEXT));
	}

	return result;
}

void
purge_logs()
{
	ac_preferences			preferences;
	db_access				db;
	time_t					point_of_del;
	time(&point_of_del);
	point_of_del -= preferences.get_log_duration() * 24 * 60 * 60;
	char					sql[200] = "delete from logs where launch_time < ";
	char					int_to_char[21];
	gcvt(point_of_del, 21, int_to_char);
	strcat(sql, int_to_char);
	db.execute(sql);
}
