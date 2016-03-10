/*
 * synchrorep_db->h
 * Created on: 3 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 * Synchrorep class to drive database requests
 */

#ifndef SYNCHROREP_DB_H_
#define SYNCHROREP_DB_H_

#include <sqlite3.h>
#include <list>
using namespace std;

// application class
#include "../applicative/application.h"

// constants
#define			LIMIT_ROWS_NO_LIMIT		-1
#define			FETCH_FIRST				0
#define			FETCH_PREVIOUS			1
#define			FETCH_NEXT				2

#define			_no_error				0

class cursor;

// management of user database
class db_access
{
private:
	sqlite3			*db;

	// callback to get request result
	static int		get_result(void *NotUsed, int argc, char **argv, char **azColName);

public:
					db_access();
					~db_access();
	bool			test_connection();
	cursor*			execute(const char *req, int limit_rows = LIMIT_ROWS_NO_LIMIT);
	char*			format_for_db(char *c_string);
	int				get_error_code();
	char*			get_error_msg();
};

// cursor
typedef struct resultset
{
	const char			**row;
	resultset			*previous;
	resultset			*next;
} resultset;

class cursor
{
private:
	const char					**cols;
	int							num_cols;
	resultset					*current_row,
								*first_row,
								*last_row;
	long						num_rows;

protected:
	void						add_row(const char **cols, const char **cells, int num_cols);

public:
								cursor();
								~cursor();
	bool						fetch(int how = FETCH_NEXT);
	char*						current_row_get_result_by_column(const char *column);
	char*						current_row_get_result_by_index(int index);
	char**						current_row_get_all_row_result();
	long						get_num_rows();

	// sorting file aphabetic order of first field (or indexed field if indicated)
	void						sort(int index = 0, resultset *to_sort = NULL, resultset *end_sort = NULL);

friend class db_access;
};


#endif /* SYNCHROREP_DB_H_ */
