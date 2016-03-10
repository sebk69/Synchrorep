/*
 * log.cpp
 *
 *  Created on: 26 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009-2010 - Sébastien Kus
 *
 *  log events class
 */

#include "logging.h"

#include <gio/gio.h>

#include <string.h>
#include <iostream>
#include <sstream>

#include "../technical/tc_file_with_infos.h"

logging :: logging(ac_config *current_config, time_t launch_time) : ac_log_action(current_config->get_id(), launch_time)
{
	this->config = current_config;
	switch(current_config->behavior->get_log_level())
	{
		case _log_nothing:
			this->verbose = _silent;
		break;

		case _log_errors:
			this->verbose = _verbose;
		break;

		case _log_actions:
			this->verbose = _ultra_verbose;
		break;
	}
	this->current_binomial = NULL;
}

logging :: ~logging()
{
}

void
logging :: set_context(_log_context context)
{
	this->context = context;
	if(this->verbose == _verbose || this->verbose == _ultra_verbose)
		switch(this->context)
		{
			case _context_pending:
				cout<<"Start synchronization..."<<endl;
			break;

			case _context_scan:
				cout<<"Analyzing..."<<endl;
			break;

			case _context_process:
				cout<<"Synchronize..."<<endl;
			break;
		}
}

void
logging :: start_action(_log_actions_type action, tc_files_binomial *files)
{
	this->action = action;
	if(files != NULL)
	{
		this->current_binomial = files;
		this->partial_path = files->get_from()->get_partial_path();
		this->from_time = files->get_from()->get_modification_time().tv_sec;
		this->to_time = files->get_to()->get_modification_time().tv_sec;
		this->answer = _answer_none;
	}
	else
	{
		this->current_binomial = NULL;
		this->partial_path = strdup("");
		this->from_time = 0;
		this->to_time = 0;
		this->answer = _answer_none;
	}
	if(this->verbose == _ultra_verbose && files != NULL)
	{
		tc_file_with_infos		*from = files->get_from();
		char					*partial_path = from->get_partial_path();
		switch(action)
		{
			case _copy_from_to_to:
				cout<<"copying "<<partial_path<<" (from->to)..."<<endl;
			break;

			case _copy_to_to_from:
				cout<<"copying "<<partial_path<<" (to->from)..."<<endl;
			break;

			case _delete_from:
				cout<<"deleting "<<partial_path<<" (from)..."<<endl;
			break;

			case _delete_to:
				cout<<"deleting "<<partial_path<<" (to)..."<<endl;
			break;

			case _trash_from:
				cout<<"put "<<partial_path<<" (from) to trash..."<<endl;
			break;

			case _trash_to:
				cout<<"put "<<partial_path<<" (to) to trash..."<<endl;
			break;

			default:
			break;
		}
		free(partial_path);
		delete from;
	}
}

void
logging :: answer_question(_log_answers answer)
{
	this->answer = answer;
}

error_infos
logging :: get_error_context(GError *error)
{
	char				*context_string;
	switch(this->context)
	{
		case _context_pending:
			context_string = strdup(gettext("An error occurred before starting synchronization."));
		break;

		case _context_scan:
			context_string = strdup(gettext("An error occurred while analyzing folders."));
		break;

		case _context_process:
			context_string = strdup(gettext("An error occurred while synchronizing."));
		break;

		case _context_abort:
			context_string = strdup(gettext("An error occurred while aborting."));
		break;

		case _context_pending_question:
			context_string = strdup(gettext("An error occurred while processing answer of question."));
		break;
	}
	error_infos			result = {context_string, error->message};
	this->error = new error_infos;
	*this->error = result;

	if(this->verbose != _silent)
		cout<<context_string<<" : "<<error->message<<endl;

	return result;
}

error_infos
logging :: get_error()
{
	ostringstream			error_context;

	switch(this->action)
	{
		case _compare:
			error_context<<gettext("Comparing")<<" \""<<g_uri_unescape_string(this->current_binomial->get_from()->get_partial_path(), NULL)<<"\" "<<gettext("between folder 1 and folder 2");
		break;

		case _copy_from_to_to:
			if(g_file_query_file_type(this->current_binomial->get_from()->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
				error_context<<gettext("Can't copy file")<<" \""<<g_uri_unescape_string(this->current_binomial->get_from()->get_uri(), NULL)<<"\"";
			else
				error_context<<gettext("Can't copy folder")<<" \""<<g_uri_unescape_string(this->current_binomial->get_from()->get_uri(), NULL)<<"\"";
		break;

		case _copy_to_to_from:
			if(g_file_query_file_type(this->current_binomial->get_to()->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
				error_context<<gettext("Can't copy file")<<" \""<<g_uri_unescape_string(this->current_binomial->get_to()->get_uri(), NULL)<<"\"";
			else
				error_context<<gettext("Can't copy folder")<<" \""<<g_uri_unescape_string(this->current_binomial->get_to()->get_uri(), NULL)<<"\"";
		break;

		case _delete_from:
			error_context<<gettext("Can't delete")<<" \""<<g_uri_unescape_string(this->current_binomial->get_to()->get_uri(), NULL)<<"\"";
		break;

		case _delete_to:
			error_context<<gettext("Can't delete")<<" \""<<g_uri_unescape_string(this->current_binomial->get_from()->get_uri(), NULL)<<"\"";
		break;
		case _trash_from:
			error_context<<gettext("Can't move file")<<" \""<<g_uri_unescape_string(this->current_binomial->get_to()->get_uri(), NULL)<<"\" "<<gettext("to trash");
		break;
		case _trash_to:
			error_context<<gettext("Can't move file")<<" \""<<g_uri_unescape_string(this->current_binomial->get_from()->get_uri(), NULL)<<"\" "<<gettext("to trash");
		break;
	}

	error_infos			result = {strdup(error_context.str().c_str()), this->current_binomial->error->message};
	this->error = new error_infos;
	*this->error = result;

	cerr<<error_context.str().c_str()<<" : "<<this->current_binomial->error->message<<endl;

	return result;
}

void
logging :: close_action()
{
	// log if an action is performed
	if(this->action != _none && this->verbose == _ultra_verbose)
		this->commit();
	// log if error
	else if(this->error != NULL && (this->verbose == _verbose || this->verbose == _ultra_verbose))
		this->commit();
	// log if abort
	else if((this->action == _abort || this->action == _start || this->action == _end)
			&& (this->verbose == _verbose || this->verbose == _ultra_verbose))
		this->commit();
	else if(this->is_asking(this->action))
		this->commit();
	this->reset();
}
