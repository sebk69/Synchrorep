/*
 * ac_behaviour.h
 * Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009-2010 - Sébastien Kus
 *
 *
 * accessor to behaviors
 */

#ifndef AC_BEHAVIOR_H_
#define AC_BEHAVIOR_H_

#include "ac_common.h"

// types
typedef enum
{
	_question_on_error,
	_ignore_error,
	_stop_on_error
} to_do_on_error_type;
typedef enum
{
	_question_on_modify,
	_ignore_modifications,
	_stop_on_modified,
	_get_older_modified,
	_get_recent_modified
} to_do_on_modify_type;
typedef enum
{
	_no_md5sum,
	_both_modified_md5sum,
	_database_md5sum
} md5sum_level_type;
typedef enum
{
	_log_nothing,
	_log_errors,
	_log_actions
} log_level_type;

// accessor of behavior
class ac_behavior : public ac_common
{
private:
	// database properties
	char					*id;
	bool					alert_when_file_deletion,
							move_to_trash_when_delete,
							move_to_trash_when_overwrite;
	to_do_on_error_type		to_do_on_error;
	to_do_on_modify_type	to_do_on_modify_conflict;
	md5sum_level_type		md5sum_level;
	log_level_type			log_level;
	bool					ask_at_the_end;

	// real constructor
	void					_construct(char *from_folder_uri);

	// convert properties from and to database
	char*					convert_to_database(void *property);

	// database modifications
	void					create();
	void					update();
	void					remove();

public:
	// constructor of default behavior
							ac_behavior();
	// constructor (from_folder_uri must be converted to database format)
							ac_behavior(char *from_folder_uri);
	// duplication of constructor
							ac_behavior(const ac_behavior &from_behaviour);
							~ac_behavior();

	// affectation
	ac_behavior&			operator=(const ac_behavior& rhs);

	static ac_behavior		default_behavior;

	bool					exists();

	bool					get_alert_when_file_deletion();
	bool					get_move_to_trash_when_delete();
	bool					get_move_to_trash_when_overwrite();
	to_do_on_error_type		get_to_do_on_error();
	to_do_on_modify_type	get_to_do_on_modify_conflict();
	md5sum_level_type		get_md5sum_level();
	log_level_type			get_log_level();
	bool					get_ask_at_the_end();

	void					set_alert_when_file_deletion(bool value);
	void					set_move_to_trash_when_delete(bool value);
	void					set_move_to_trash_when_overwrite(bool value);
	void					set_to_do_on_error(to_do_on_error_type value);
	void					set_to_do_on_modify_conflict(to_do_on_modify_type value);
	void					set_md5sum_level(md5sum_level_type value);
	void					set_log_level(log_level_type value);
	void					set_ask_at_the_end(bool value);

	bool					commit();

	// debugging options
	void					dump();
};

#endif
