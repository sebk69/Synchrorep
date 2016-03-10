/*
 * tc_file_with_info.h
 *
 *  Created on: 15 juil. 2009
 *
 *      ©2009, 2010 - Sébastien Kus
 *
 *  This source is under GNU/GLP V3 licence
 */

#ifndef TC_FILE_WITH_INFO_H_
#define TC_FILE_WITH_INFO_H_

#include <gio/gio.h>

class tc_file_with_infos
{
private:
	GFile			*file;
	char			*file_path;
	char			*parent_folder_path;
	GTimeVal		modification_time;
	GTimeVal		database_modification_time;
	char			*partial_path;
	char			*base_name;
	char			*md5;
	char			*db_md5;

public:

					tc_file_with_infos(char *file);
					tc_file_with_infos(GFile *file);
					tc_file_with_infos(char *file, char *parent_folder);
					tc_file_with_infos(GFile *file, char *parent_folder);
					tc_file_with_infos(char *file, GFile *parent_folder);
					tc_file_with_infos(GFile *file, GFile *parent_folder);
					tc_file_with_infos(tc_file_with_infos &to_dup);
					~tc_file_with_infos();

	// duplication by operation =
	tc_file_with_infos&		operator=(const tc_file_with_infos& rhs);

	// comparators
	bool			operator<(const tc_file_with_infos& compared);
	bool			operator>(const tc_file_with_infos& compared);
	bool			operator==(const tc_file_with_infos& compared);
	int				compare_with_file(GFile *compared);

	// get data
	GFile*			get_file();
	GTimeVal		get_modification_time();
	char*			get_path();
	char*			get_uri();
	GTimeVal		get_database_modification_time();
	void			set_database_modification_time(GTimeVal);
	GFile*			get_parent();
	char*			get_base_name();
	char*			get_parent_path();
	char*			get_parent_uri();
	char*			get_partial_path();
	void			set_partial_path(char *);
	void			set_db_md5(char *md5);
	char*			get_db_md5();
	char*			get_md5(GError **error);

	// tests
	bool			is_folder();

	// refreshing modification date
	void			refresh();
};

#endif /* TC_FILE_WITH_INFO_H_ */
