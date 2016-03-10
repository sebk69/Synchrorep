/*
 * tc_find_files.h
 *
 *  Created on: 15 juil. 2009
 *      ©2009, 2010 - Sébastien Kus
 *  This source is under GNU/GLP V3 licence
 *
 *  Technical class to retrieve list of files in a folder
 */

#ifndef TC_FIND_FILES_H_
#define TC_FIND_FILES_H_

#include <list>
using namespace std;

#include <gio/gio.h>

#include "tc_file_with_infos.h"
#include "tc_thread_utils.h"

typedef struct tc_find_files_cell
{
	tc_file_with_infos			*data;
	tc_find_files_cell			*down,
								*next,
								*up;
} tc_find_files_cell;

class tc_find_files
{
private:
	tc_find_files_cell			*base,
								*current_file;
	int							num_files;
	mutex_var<bool>				*cancel;

	void						sub_folder(tc_find_files_cell *node);
	static bool					compare_elements(tc_file_with_infos* first, tc_file_with_infos* second);
	void						add(tc_find_files_cell *node, tc_file_with_infos *file);
	void						refresh_current(tc_find_files_cell *from);

	tc_file_with_infos			*base_folder;
	char						*base_folder_path;
	static gpointer				process(gpointer THIS);

public:
	GError						*error;
								tc_find_files(GFile *base_folder, mutex_var<bool> *cancel = NULL);
								~tc_find_files();

	// true if find ok
	// false if base_folder don't exists
	GThread*					process_t();
	bool						process();

	// get machine time
	GTimeVal					get_time(GError **error = NULL);
	// delete for ftp
	bool						ftp_delete(GError **error, mutex_var<int> *decrement = NULL);
	// copy for ftp
	bool						ftp_copy(GFile *target, GError **error);

	// don't scan down
	void						remove_down();

	// path to base folder
	char*						get_base_folder_path();

	// manipulate result list
	bool						begin();
	bool						next(bool scan_down = true);
	int							count();

	// get datas
	GFile*						get_current();
	GTimeVal					get_current_modification_time();
	GTimeVal					get_current_database_modification_time();
	void						set_current_database_modification_time(GTimeVal time);
	tc_file_with_infos*			get_file_with_info();
	char*						get_current_partial_path();
	void						refresh_current();

	void						add_db_data(char *file_partial_path, GTimeVal modification_date);

	// find a file
	bool						find(char *target);
	bool						exists(GFile *target);

	// dumping paths
	void						dump();
};

#endif /* TC_FIND_FILES_H_ */
