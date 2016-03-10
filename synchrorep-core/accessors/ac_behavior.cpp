/*
 * ac_behaviour.cpp
 * Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009-2010 - Sébastien Kus
 *
 * accessor to behaviors
 *
 */

#include "ac_behavior.h"
#include "db_access.h"

#include "string.h"
#include <iostream>

// constructor of default behavior
ac_behavior :: ac_behavior()
{
	char			*default_id = strdup("DEFAUT");

	this->_construct(default_id);

	free(default_id);
}

// public constructor
ac_behavior :: ac_behavior(char *from_folder_uri)
{
	_construct(from_folder_uri);
}

// Duplication constructor
ac_behavior :: ac_behavior(const ac_behavior &from_behavior) : ac_common(from_behavior)
{
	this->id = strdup(from_behavior.id);
	this->alert_when_file_deletion=from_behavior.alert_when_file_deletion;
	this->move_to_trash_when_delete=from_behavior.move_to_trash_when_delete;
	this->move_to_trash_when_overwrite=from_behavior.move_to_trash_when_overwrite;
	this->to_do_on_error=from_behavior.to_do_on_error;
	this->to_do_on_modify_conflict=from_behavior.to_do_on_modify_conflict;
	this->md5sum_level = from_behavior.md5sum_level;
	this->log_level = from_behavior.log_level;
	this->alteration = from_behavior.alteration;
	this->ask_at_the_end = from_behavior.ask_at_the_end;
}

// real constructor
void
ac_behavior :: _construct(char *loc_id)
{
	cursor			*behaviour_cursor;
	char			sql[strlen(loc_id)+500],
					*parm,
					*answer;
	bool			check_md5sum_exists = false;
	bool			check_log_exists = false;
	bool			ask_at_the_end_exists = false;

	// save id
	this->id = strdup(loc_id);

	// execute sql request
	sql[0] = '\0';
	strcat(sql, "select action, reaction from comportements where from_folder = '");
	strcat(sql, loc_id);
	strcat(sql, "'");
	behaviour_cursor = this->db->execute(sql);
	// if none found, take default values
	if(!behaviour_cursor->fetch(FETCH_FIRST))
	{
		sql[0] = '\0';
		strcat(sql, "select action, reaction from comportements where from_folder = 'DEFAUT'");
		behaviour_cursor = this->db->execute(sql);
		this->alteration = _ac_create;
	}
	else
		this->alteration = _ac_unchanged;
	if(behaviour_cursor->fetch(FETCH_FIRST))
	{
		do
		{
			parm = behaviour_cursor->current_row_get_result_by_column("action");
			answer = behaviour_cursor->current_row_get_result_by_column("reaction");
			if(strcmp(parm, "alert_supp") == 0)
				this->alert_when_file_deletion = (strcmp(answer, "OUI") == 0);
			if(strcmp(parm, "corb_supp") == 0)
				this->move_to_trash_when_delete = (strcmp(answer, "OUI") == 0);
			if(strcmp(parm, "corb_mod") == 0)
				this->move_to_trash_when_overwrite = (strcmp(answer, "OUI") == 0);
			if(strcmp(parm, "error") == 0)
			{
				if(strcmp(answer, "question") == 0)
					this->to_do_on_error = _question_on_error;
				if(strcmp(answer, "ignorer") == 0)
					this->to_do_on_error = _ignore_error;
				if(strcmp(answer, "stop") == 0)
					this->to_do_on_error = _stop_on_error;
			}
			if(strcmp(parm, "modif") == 0)
			{
				if(strcmp(answer, "question") == 0)
					this->to_do_on_modify_conflict = _question_on_modify;
				if(strcmp(answer, "ignorer") == 0)
					this->to_do_on_modify_conflict = _ignore_modifications;
				if(strcmp(answer, "stop") == 0)
					this->to_do_on_modify_conflict = _stop_on_modified;
				if(strcmp(answer, "ancien") == 0)
					this->to_do_on_modify_conflict = _get_older_modified;
				if(strcmp(answer, "recent") == 0)
					this->to_do_on_modify_conflict = _get_recent_modified;
			}
			if(strcmp(parm, "md5sum") == 0)
			{
				check_md5sum_exists = true;
				if(strcmp(answer, "none") == 0)
					this->md5sum_level = _no_md5sum;
				else if(strcmp(answer, "both_modif") == 0)
					this->md5sum_level = _both_modified_md5sum;
				else if(strcmp(answer, "database") == 0)
					this->md5sum_level = _database_md5sum;
			}
			if(strcmp(parm, "log") == 0)
			{
				check_log_exists = true;
				if(strcmp(answer, "none") == 0)
					this->log_level = _log_nothing;
				else if(strcmp(answer, "errors") == 0)
					this->log_level = _log_errors;
				else if(strcmp(answer, "actions") == 0)
					this->log_level = _log_actions;
			}
			if(strcmp(parm, "end_asking") == 0)
			{
				this->ask_at_the_end = (strcmp(answer, "OUI") == 0);
				ask_at_the_end_exists = true;
			}
		} while(behaviour_cursor->fetch(FETCH_NEXT));
		if(!check_md5sum_exists)
		{
			this->md5sum_level = _no_md5sum;
			sql[0] = '\0';
			strcat(sql, "insert into comportements values(NULL, '");
			strcat(sql, this->id);
			strcat(sql, "', 'md5sum', '");
			strcat(sql, this->convert_to_database(&this->md5sum_level));
			strcat(sql, "')");
			this->db->execute(sql);
		}
		if(!check_log_exists)
		{
			this->log_level = _log_errors;
			sql[0] = '\0';
			strcat(sql, "insert into comportements values(NULL, '");
			strcat(sql, this->id);
			strcat(sql, "', 'log', '");
			strcat(sql, this->convert_to_database(&this->log_level));
			strcat(sql, "')");
			this->db->execute(sql);
		}
		if(!ask_at_the_end_exists)
		{
			this->ask_at_the_end = false;
			sql[0] = '\0';
			strcat(sql, "insert into comportements values(NULL, '");
			strcat(sql, this->id);
			strcat(sql, "', 'end_asking', '");
			strcat(sql, this->convert_to_database(&this->ask_at_the_end));
			strcat(sql, "')");
			this->db->execute(sql);
		}
	}
	// default values are not set yet
	else
	{
		// setup new behavior with application default
		this->alert_when_file_deletion = true;
		this->move_to_trash_when_delete = true;
		this->move_to_trash_when_overwrite = true;
		this->to_do_on_error = _question_on_error;
		this->to_do_on_modify_conflict = _question_on_modify;
		this->md5sum_level = _no_md5sum;
		this->log_level = _log_errors;
	}
}

// destructor
ac_behavior :: ~ac_behavior()
{
	free(this->id);
}

// duplication by operation =
ac_behavior&
ac_behavior :: operator=(const ac_behavior& rhs)
{
	if(this == &rhs)
		return *this;

	this->id = strdup(rhs.id);
	this->alert_when_file_deletion=rhs.alert_when_file_deletion;
	this->move_to_trash_when_delete=rhs.move_to_trash_when_delete;
	this->move_to_trash_when_overwrite=rhs.move_to_trash_when_overwrite;
	this->to_do_on_error=rhs.to_do_on_error;
	this->to_do_on_modify_conflict=rhs.to_do_on_modify_conflict;
	this->md5sum_level=rhs.md5sum_level;
	this->log_level=rhs.log_level;
	this->alteration = rhs.alteration;
	this->ask_at_the_end = rhs.ask_at_the_end;

	return *this;
}

// properties accessors
bool
ac_behavior :: get_alert_when_file_deletion()
{
	return this->alert_when_file_deletion;
}

bool
ac_behavior :: get_move_to_trash_when_delete()
{
	return this->move_to_trash_when_delete;
}

bool
ac_behavior :: get_move_to_trash_when_overwrite()
{
	return this->move_to_trash_when_overwrite;
}

to_do_on_error_type
ac_behavior :: get_to_do_on_error()
{
	return this->to_do_on_error;
}

to_do_on_modify_type
ac_behavior :: get_to_do_on_modify_conflict()
{
	return this->to_do_on_modify_conflict;
}

md5sum_level_type
ac_behavior :: get_md5sum_level()
{
	return this->md5sum_level;
}

log_level_type
ac_behavior :: get_log_level()
{
	return this->log_level;
}

bool
ac_behavior :: get_ask_at_the_end()
{
	return this->ask_at_the_end;
}

void
ac_behavior :: set_alert_when_file_deletion(bool value)
{
	this->alert_when_file_deletion = value;
	this->modified();
}

void
ac_behavior :: set_move_to_trash_when_delete(bool value)
{
	this->move_to_trash_when_delete = value;
	this->modified();
}

void
ac_behavior :: set_move_to_trash_when_overwrite(bool value)
{
	this->move_to_trash_when_overwrite = value;
	this->modified();
}

void
ac_behavior :: set_to_do_on_error(to_do_on_error_type value)
{
	this->to_do_on_error = value;
	this->modified();
}

void
ac_behavior :: set_to_do_on_modify_conflict(to_do_on_modify_type value)
{
	this->to_do_on_modify_conflict = value;
	this->modified();
}

void
ac_behavior :: set_md5sum_level(md5sum_level_type value)
{
	this->md5sum_level = value;
	this->modified();
}

void
ac_behavior :: set_log_level(log_level_type value)
{
	this->log_level = value;
	this->modified();
}

void
ac_behavior :: set_ask_at_the_end(bool value)
{
	this->ask_at_the_end = value;
	this->modified();
}

// convertions
char*
ac_behavior :: convert_to_database(void *property)
{
	if(property == &this->move_to_trash_when_delete)
	{
		if(this->move_to_trash_when_delete == true)
			return strdup("OUI");
		else
			return strdup("NON");
	}
	else if(property == &this->move_to_trash_when_overwrite)
	{
		if(this->move_to_trash_when_overwrite == true)
			return strdup("OUI");
		else
			return strdup("NON");
	}
	else if(property == &this->alert_when_file_deletion)
	{
		if(this->alert_when_file_deletion == true)
			return strdup("OUI");
		else
			return strdup("NON");
	}
	else if(property == &this->to_do_on_error)
	{
		switch(this->to_do_on_error)
		{
			case _question_on_error:
				return strdup("question");
			break;

			case _ignore_error:
				return strdup("ignorer");
			break;

			case _stop_on_error:
				return strdup("stop");
			break;
		}
	}
	else if(property == &this->to_do_on_modify_conflict)
	{
		switch(this->to_do_on_modify_conflict)
		{
			case _question_on_modify:
				return strdup("question");
			break;

			case _ignore_modifications:
				return strdup("ignorer");
			break;

			case _stop_on_modified:
				return strdup("stop");
			break;

			case _get_older_modified:
				return strdup("ancien");
			break;

			case _get_recent_modified:
				return strdup("recent");
			break;
		}
	}
	else if(property == &this->md5sum_level)
	{
		switch(this->md5sum_level)
		{
			case _no_md5sum:
				return strdup("none");
			break;

			case _both_modified_md5sum:
				return strdup("both_modif");
			break;

			case _database_md5sum:
				return strdup("database");
			break;
		}
	}
	else if(property == &this->log_level)
	{
		switch(this->log_level)
		{
			case _log_nothing:
				return strdup("none");
			break;

			case _log_errors:
				return strdup("errors");
			break;

			case _log_actions:
				return strdup("actions");
			break;
		}
	}
	else if(property == &this->ask_at_the_end)
	{
		if(this->ask_at_the_end == true)
			return strdup("OUI");
		else
			return strdup("NON");
	}
	return NULL;
}

// database modifications
bool
ac_behavior :: commit()
{
	switch(this->alteration)
	{
		case _ac_create:
			this->create();
			return true;
		break;

		case _ac_modified:
			this->update();
			return true;
		break;

		case _ac_deleted:
			this->remove();
			return true;
		break;

		case _ac_unchanged:
			return true;
		break;

		case _ac_invalid:
			return false;
		break;
	}

	return false;
}

void
ac_behavior :: create()
{
	char			sql[strlen(this->id) + 500];

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'alert_supp', '");
	strcat(sql, this->convert_to_database(&this->alert_when_file_deletion));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'corb_supp', '");
	strcat(sql, this->convert_to_database(&this->move_to_trash_when_delete));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'corb_mod', '");
	strcat(sql, this->convert_to_database(&this->move_to_trash_when_overwrite));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'error', '");
	strcat(sql, this->convert_to_database(&this->to_do_on_error));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'modif', '");
	strcat(sql, this->convert_to_database(&this->to_do_on_modify_conflict));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'md5sum', '");
	strcat(sql, this->convert_to_database(&this->md5sum_level));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'log', '");
	strcat(sql, this->convert_to_database(&this->log_level));
	strcat(sql, "')");
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "insert into comportements values(NULL, '");
	strcat(sql, this->id);
	strcat(sql, "', 'end_asking', '");
	strcat(sql, this->convert_to_database(&this->ask_at_the_end));
	strcat(sql, "')");
	this->db->execute(sql);

	this->alteration = _ac_unchanged;
}

void
ac_behavior :: update()
{
	char			sql[strlen(this->id) + 500];
	char			whereid[strlen(this->id) + 100];

	whereid[0] = '\0';
	strcat(whereid, " and from_folder='");
	strcat(whereid, this->id);
	strcat(whereid, "'");

	sql[0] = '\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->alert_when_file_deletion));
	strcat(sql, "' where action='alert_supp'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->move_to_trash_when_delete));
	strcat(sql, "' where action='corb_supp'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->move_to_trash_when_overwrite));
	strcat(sql, "' where action='corb_mod'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->to_do_on_error));
	strcat(sql, "' where action='error'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->to_do_on_modify_conflict));
	strcat(sql, "' where action='modif'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->md5sum_level));
	strcat(sql, "' where action='md5sum'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0]='\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->log_level));
	strcat(sql, "' where action='log'");
	strcat(sql, whereid);
	this->db->execute(sql);

	sql[0] = '\0';
	strcat(sql, "update comportements set reaction='");
	strcat(sql, this->convert_to_database(&this->ask_at_the_end));
	strcat(sql, "' where action='end_asking'");
	strcat(sql, whereid);
	this->db->execute(sql);

	this->alteration = _ac_unchanged;
}

void
ac_behavior :: remove()
{
	char			sql[strlen(this->id) + 500];

	sql[0] = '\0';
	strcat(sql, "delete from comportements where from_folder='");
	strcat(sql, this->id);
	strcat(sql, "'");
	this->db->execute(sql);
	this->_construct(this->id);
}

// dumping datas to terminal
void
ac_behavior :: dump()
{
	char			*data;

	cout<<"Dumping behaviour : "<<this->id<<endl;
	data = this->convert_to_database(&this->alert_when_file_deletion);
	if(data != NULL)
		cout<<"alert when file deletion : "<<data<<endl;
	else
		cout<<"alert when file deletion : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->move_to_trash_when_delete);
	if(data != NULL)
		cout<<"move to trash when delete : "<<data<<endl;
	else
		cout<<"move to trash when delete : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->move_to_trash_when_overwrite);
	if(data != NULL)
		cout<<"move to trash when overwrite : "<<data<<endl;
	else
		cout<<"move to trash when overwrite : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->to_do_on_error);
	if(data != NULL)
		cout<<"what to do on error : "<<data<<endl;
	else
		cout<<"what to do on error : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->to_do_on_modify_conflict);
	if(data != NULL)
		cout<<"what to do on modify conflict : "<<data<<endl;
	else
		cout<<"what to do on modify conflict : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->md5sum_level);
	if(data != NULL)
		cout<<"md5sum level : "<<data<<endl;
	else
		cout<<"md5sum level : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->log_level);
	if(data != NULL)
		cout<<"log level : "<<data<<endl;
	else
		cout<<"log level : #########"<<endl;
	free(data);
	data = this->convert_to_database(&this->ask_at_the_end);
	if(data != NULL)
		cout<<"ask at the end : "<<data<<endl;
	else
		cout<<"ask at the end : #########"<<endl;
	free(data);
}
