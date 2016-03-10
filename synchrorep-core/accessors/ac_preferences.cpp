/*
 * ac_preferences.cpp
 *
 *  Created on: 6 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  preferences accessor
 *
 */

#include "ac_preferences.h"
#include "../applicative/application.h"
#include "../interface/dialog_info.h"
#include "../technical/tc_misc.h"

#include <string.h>
#include <iostream>

#include <gio/gio.h>
#include <glib.h>

ac_preferences :: ac_preferences()
{
	// default values
	this->expert_mode = false;

	// set gfile for contextual menu configuration
	char				*home_dir = strdup(g_get_home_dir());
	char				*file_name = new char[strlen(home_dir) + 50];
	strcpy(file_name, home_dir);
	strcat(file_name, "/.synchrorep.menu");
	this->contextual_menu_file_param = g_file_new_for_commandline_arg(file_name);
	delete file_name;
	free(home_dir);

	this->alteration = _ac_unchanged;
}

ac_preferences :: ~ac_preferences()
{
}

bool
ac_preferences :: get_expert_mode()
{
	// expert mode
	char			sql[500] = "select value from prefs where key = 'EXPERT'";
	cursor			*result = this->db->execute(sql);
	if(result->fetch(FETCH_FIRST))
	{
		char			*value = result->current_row_get_result_by_column("value");
		if(strcmp(value, "OUI") == 0)
			this->expert_mode = true;
		else
			this->expert_mode = false;
	}

	return this->expert_mode;
}

void
ac_preferences :: set_expert_mode(bool activate)
{
	this->expert_mode = activate;
	/* expert mode */
	// test if expert mode exists in database
	char				sql[500] = "select key from prefs where key = 'EXPERT'";
	cursor				*result = this->db->execute(sql);
	// case exists, update it
	if(result->fetch(FETCH_FIRST))
	{
		strcpy(sql, "update prefs set value = ");
		if(this->expert_mode)
			strcat(sql, "'OUI'");
		else
			 strcat(sql, "'NON'");
		strcat(sql, " where key = 'EXPERT'");
		this->db->execute(sql);
	}
	// case don't exists, create it
	else
	{
		strcpy(sql, "insert into prefs values('EXPERT', ");
		if(this->expert_mode)
			strcat(sql, "'OUI'");
		else
			strcat(sql, "'NON'");
		strcat(sql, ")");
		this->db->execute(sql);
	}
	this->alteration = _ac_modified;
}

bool
ac_preferences :: get_contextual_menu()
{
	return g_file_query_exists(this->contextual_menu_file_param, NULL);
}

void
ac_preferences :: set_contextual_menu(bool activate)
{
	GError				*error = NULL;
	if(this->get_contextual_menu() && !activate)
		g_file_delete(this->contextual_menu_file_param, NULL, &error);
	else if(!this->get_contextual_menu() && activate)
		g_file_create(this->contextual_menu_file_param, G_FILE_CREATE_REPLACE_DESTINATION, NULL, &error);

	if(error != NULL)
	{
		dialog_info				dialog(NULL);
		dialog.asking(gettext("An error occurred when commit preferences."));
	}
}


unsigned int
ac_preferences :: get_log_duration()
{
	char			sql[500];
	cursor			*result;
	// log duration
	strcpy(sql, "select value from prefs where key = 'DURLOG'");
	result = this->db->execute(sql);
	if(result->fetch(FETCH_FIRST))
		this->log_duration = convert_string_to_int(result->current_row_get_result_by_column("value"));
	else
		this->log_duration = 10;

	return this->log_duration;
}

void
ac_preferences :: set_log_duration(unsigned int value)
{
	this->log_duration = value;
	/* log duration */
	// test if expert mode exists in database
	char			sql[500];
	cursor			*result;
	strcpy(sql, "select key from prefs where key = 'DURLOG'");
	result = this->db->execute(sql);
	// case exists, update it
	if(result->fetch(FETCH_FIRST))
	{
		strcpy(sql, "update prefs set value = ");
		char			int_to_char[4];
		gcvt(this->log_duration, 3, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, " where key = 'DURLOG'");
		this->db->execute(sql);
	}
	// case don't exists, create it
	else
	{
		strcpy(sql, "insert into prefs values('DURLOG', ");
		char			int_to_char[4];
		gcvt(this->log_duration, 3, int_to_char);
		strcat(sql, int_to_char);
		strcat(sql, ")");
		this->db->execute(sql);
	}
}
