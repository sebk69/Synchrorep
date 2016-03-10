/*
 * ac_config.h
 *
 *  Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011 - Sébastien Kus
 *
 *
 *  driving synchronization settings
 */

#ifndef AC_CONFIG_H_
#define AC_CONFIG_H_

#include <list>

#include <gio/gio.h>

#include "ac_common.h"
#include "ac_behavior.h"

#include "../technical/tc_find_files.h"
#include "../technical/tc_file_with_infos.h"
#include "../technical/tc_thread_utils.h"

#include "../applicative/application.h"

using namespace std;

// types
typedef enum {_unknown, _path, _uri} path_mode_type;
typedef enum {_from_folder_missing, _to_folder_missing, _both_missing, _both_accessible} folders_accessible_type;
typedef enum {_from, _to} from_or_to_type;
typedef enum {_unknown_mode, _sync, _copy} config_mode;


// synchronization setting
class ac_config_list;
class ac_config : public ac_common
{
private:
	int								id;
	char							*from_folder_of_db;
	char							*from_folder_uri;
	char							*to_folder_uri;

	bool							init(char *sql);

protected:
	GFile							*from_folder,
									*to_folder;
	path_mode_type					path_mode;
	char							*group;
	config_mode						mode;

public:
	ac_behavior						*behavior;

									ac_config(char *folder);
									ac_config(int config_id);
									ac_config(const ac_config& rhs);
									~ac_config();

	ac_config&						operator=(const ac_config& rhs);

	// properties accessors
	GFile*							get_from_folder();
	GFile*							get_to_folder();
	char*							get_from_folder_uri();
	char*							get_to_folder_uri();
	int								get_id();
	GTimeVal						get_from_file_db_time(char *partial_path);
	GTimeVal						get_to_file_db_time(char *partial_path);

	// avoid same synchronization in same time
	bool							trylock();
	void							unlock();

	// to_folder can be changed on creation
	bool							set_to_folder(char *folder); // possible only on creation
	bool							set_to_folder(GFile *folder); // possible only on creation

	// group accessors
	char*							get_group();
	bool							set_group(char *new_group);
	static char**					get_groups();

	// mode accessor
	config_mode						get_mode();
	void							set_mode(config_mode mode);

	// are folders accessible
	folders_accessible_type			accessibility();

	// configuration is ready for synchronization ?
	bool							ready_to_start();

	// commit modifications (not for files)
	bool							commit();

	// configuration files accessors
	void							files(tc_find_files **from, tc_find_files **to, mutex_var<bool> *cancel);
	bool							refresh_file_datas(char *partial_path, tc_file_with_infos *from_infos, tc_file_with_infos *to_infos);
	void							clean_deleted_files();

	// get last launch time
	time_t							last_launch_time();

	// dumping data to standard output
	void							dump();

friend class ac_config_list;
};

// overall list of synchronizations set
class ac_config_list : ac_common
{
private:
	list<ac_config*>				config_list;
	list<ac_config*> :: iterator	config_list_it;
	int								config_list_pos;

public:
									ac_config_list(char *group = NULL);
									~ac_config_list();

	bool							begin();
	bool							next();
	bool							previous();
	bool							end();
	bool							goto_position(unsigned int index);
	void							add(ac_config new_config);
	void							remove();
	ac_config*						get_current();
	bool							find(char *folder);
	bool							find_by_id(char *id);
	bool							folder_configured(char *folder);
};

#endif /* AC_CONFIG_H_ */
