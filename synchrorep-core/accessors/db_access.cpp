/*
 * synchrorep_db->cpp
 * Created on: 3 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011 - Sébastien Kus
 *
 * Synchrorep class to drive database requests
 */

#include <string.h>
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>

#include <glib.h>

#include "db_access.h"

// synchrorep_db class

// constructor
db_access :: db_access()
{
	const char		*home = g_get_home_dir();
	char			db_name[sizeof(home) + 50] = "";

	strcat(db_name, home);
	strcat(db_name, "/.synchrorep.db");
	if(sqlite3_open(db_name, &this->db))
		this->db = NULL;

	// test if db is initialized
	char			sql[500] = "select * from comportements";
	cursor			*table_list = this->execute(sql);
	if(sqlite3_errcode(this->db) != SQLITE_OK)
	{
		this->execute("create table comportements (key integer primary key, from_folder varchar(1000), action varchar(10), reaction varchar(10))");
		this->execute("create table config (key integer primary key, from_folder varchar(1000), to_folder varchar(1000), path_mode char(4), groups varchar(100), lock varchar(10), mode varchar(4))");
		this->execute("create table files (key integer primary key, from_folder varchar(1000), file varchar(1000), date integer(14), to_date integer(14), from_md5sum char(32), to_md5sum char(32))");
		this->execute("create unique index acces on files(from_folder, file)");
		this->execute("create table prefs (key varchar(10) primary key, value varchar(255))");
	}
	delete table_list;

	// test if groups exists
	this->execute("select groups from config");
	if(sqlite3_errcode(this->db) != SQLITE_OK)
	{
		this->execute("alter table config add column groups varchar(100)");
		this->execute("update config set groups = ''");
	}

	// test if locks exists
	this->execute("select lock from config");
	if(sqlite3_errcode(this->db) != SQLITE_OK)
	{
		this->execute("alter table config add column lock varchar(10)");
		this->execute("update config set lock = ''");
	}

	// test if md5sum exists
	this->execute("select from_md5sum from files");
	if(sqlite3_errcode(this->db) != SQLITE_OK)
	{
		this->execute("alter table files add column from_md5sum varchar(32)");
		this->execute("alter table files add column to_md5sum varchar(32)");
		this->execute("update files set from_md5sum = '', to_md5sum = ''");
	}

	// test if mode exists
	this->execute("select mode from config");
	if(sqlite3_errcode(this->db) != SQLITE_OK)
	{
		this->execute("alter table config add column mode varchar(4)");
		this->execute("update config set mode = 'sync'");
	}

	this->execute("create table if not exists logs (config_key integer, launch_time integer(14), no integer(14), action char(20), action_time integer(14), file varchar(1000), from_time integer(14), to_time integer(14), ask_answer char(20), error_context varchar(5000), error_reason varchar(5000))");
}

db_access :: ~db_access()
{
	sqlite3_close(this->db);
}

bool
db_access :: test_connection()
{
	if(this->db == NULL)
		return false;
	else
		return true;
}

cursor*
db_access :: execute(const char *req, int limit_rows)
{
	// initializations
	cursor			*tmp_cursor = new cursor;
	int				err;

	// execute
	do
	{
		sqlite3_exec(this->db, req, db_access::get_result, tmp_cursor, NULL);
		err = sqlite3_errcode(this->db);
	}
	while(err == 5);
	if(err !=  SQLITE_OK)
	{
		cerr<<sqlite3_errcode(this->db)<<" : "<<sqlite3_errmsg(this->db)<<endl;
	}

	return tmp_cursor;
}

// callback de contitution du curseur
int
db_access :: get_result(void *caller, int argc, char **argv, char **azColName)
{
	((cursor*)caller)->add_row((const char**)azColName, (const char**)argv, argc);

	return 0;
}

char*
db_access :: format_for_db(char *c_string)
{
	char				out[10000];
	size_t				i, j;

	j = 0;
	for(i=0;i<strlen(c_string);i++)
	{
		if(c_string[i] == '\'')
		{
			out[j] = '\'';
			j++;
		}
		out[j] = c_string[i];
		j++;
	}
	out[j] = '\0';
	return strdup(out);
}

int
db_access :: get_error_code()
{
	return sqlite3_errcode(this->db);
}

char*
db_access :: get_error_msg()
{
	return (char*)sqlite3_errmsg(this->db);
}

// cursor class
cursor :: cursor()
{
	this->num_rows = 0;
	this->first_row = this->last_row = this->current_row = NULL;
	this->cols = NULL;
}

cursor :: ~cursor()
{
	delete this->first_row;
}

bool
cursor :: fetch(int how)
{
	switch(how)
	{
		case FETCH_FIRST:
			if(this->first_row != NULL)
				this->current_row = this->first_row;
			else
				return false;
		break;

		case FETCH_NEXT:
			if(this->current_row != NULL)
			{
				this->current_row = this->current_row->next;
				if(this->current_row == NULL)
					return false;
			}
			else
				return false;
		break;

		case FETCH_PREVIOUS:
			if(this->current_row != NULL)
			{
				this->current_row = this->current_row->previous;
				if(this->current_row == NULL)
					return false;
			}
			else
				return false;
		break;
	}

	return true;
}

char*
cursor :: current_row_get_result_by_column(const char *column)
{
	if(this->current_row == NULL)
		return NULL;

	int					col_cmp = 1;
	int					i;
	for(i=0;i<num_cols && col_cmp != 0;i++)
		col_cmp = strcmp(this->cols[i], column);
	i--;
	if(col_cmp == 0)
		return strdup(this->current_row->row[i]);

	return NULL;
}

char*
cursor :: current_row_get_result_by_index(int index)
{
	if(this->current_row == NULL)
		return NULL;

	if(index < this->num_cols)
		return strdup(this->current_row->row[index]);

	return NULL;
}

long
cursor :: get_num_rows()
{
	return this->num_rows;
}

void
cursor :: add_row(const char **cols, const char **cells, int num_cols)
{
	int						i;

	// get cols
	if(this->cols == NULL)
	{
		this->cols = new const char*[num_cols];
		for(i=0;i<num_cols;i++)
			this->cols[i] = strdup(cols[i]);
		this->num_cols = num_cols;
	}

	// get row
	resultset			*cell_to_add = new resultset;
	resultset			*search;
	cell_to_add->row = new const char*[num_cols];
	for(i=0;i<num_cols;i++)
		cell_to_add->row[i] = strdup(cells[i]);
	// add row sorted
	if(this->last_row == NULL)
	{
		cell_to_add->previous = NULL;
		cell_to_add->next = NULL;
		this->first_row = this->last_row = cell_to_add;
	}
	else
	{
		search = this->first_row;
		while(search != NULL && strcmp(search->row[0], cell_to_add->row[0]) < 0)
			search = search->next;
		cell_to_add->previous = this->last_row;
		cell_to_add->next = NULL;
		this->last_row->next = cell_to_add;
		this->last_row = cell_to_add;
	}
	this->num_rows++;
}

void
cursor :: sort(int index, resultset *to_sort, resultset *end_sort)
{
	bool			first = false;

	// assign first cell on first call
	if(to_sort == NULL)
	{
		first = true;
		to_sort = this->first_row;
		end_sort = this->last_row;
	}

	// test if single cell called
	if(to_sort == end_sort)
		return;

	// find middle
	resultset			*middle,
						*travel;
	bool				step_to_next = true;

	travel = middle = to_sort;
	do
	{
		travel = travel->next;
		if(step_to_next)
		{
			middle = middle->next;
			step_to_next = false;
		}
		else
		{
			step_to_next = true;
		}
	} while(travel != end_sort);

	// sort two middles
	this->sort(index, to_sort, middle->previous);
	this->sort(index, middle, end_sort);

	// resort part
	resultset			*save_previous = to_sort->previous;
	resultset			*save_next = end_sort->next;
	resultset			*scan;
	resultset			*new_cell;
	resultset			*new_first_cell = NULL;
	resultset			*new_end_cell = NULL;

	travel = to_sort;
	while(travel != end_sort->next)
	{
		// first cell to insert
		if(new_first_cell == NULL)
		{
			new_first_cell = new resultset;
			new_first_cell->previous = new_first_cell->next = NULL;
			new_first_cell->row = travel->row;
			new_end_cell = new_first_cell;
		}
		else
		{
			// next cells to insert
			scan = new_first_cell;
			while(scan != NULL && strcmp(scan->row[index], travel->row[index]) < 0)
				scan = scan->next;

			// create new cell
			new_cell = new resultset;
			new_cell->row = travel->row;
			if(scan != NULL)
			{
				new_cell->previous = scan->previous;
				new_cell->next = scan;
				if(new_cell->previous != NULL)
					new_cell->previous->next = new_cell;
				scan->previous = new_cell;
			}
			else
			{
				new_cell->previous = new_end_cell;
				new_end_cell->next = new_cell;
				new_cell->next = NULL;
				new_end_cell = new_cell;
			}
			// test limits of new part
			if(scan == new_first_cell)
				new_first_cell = new_cell;
		}

		travel = travel->next;
	}

	// affect new begin and end of part
	new_first_cell->previous = save_previous;
	if(save_previous != NULL)
		save_previous->next = new_first_cell;
	new_end_cell->next = save_next;
	if(save_next != NULL)
		save_next->previous = new_end_cell;

	if(first)
	{
		this->first_row = new_first_cell;
		this->last_row = new_end_cell;
	}
}
