/*
 * ac_logs.h
 *
 *  Created on: 17 mars 2010
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Logs accessor
 *
 */

#ifndef AC_LOGS_H_
#define AC_LOGS_H_

#include "ac_common.h"
#include "ac_config.h"

#include "../technical/tc_files_binomial.h"
#include "../technical/tc_misc.h"

#include "time.h"
#include <list>

typedef enum
{
	_context_pending,
	_context_scan,
	_context_process,
	_context_abort,
	_context_pending_question
} _log_context;

typedef enum
{
	_compare,
	_copy_from_to_to,
	_copy_to_to_from,
	_delete_from,
	_delete_to,
	_trash_from,
	_trash_to,
	_ask_both,
	_ask_both_trash,
	_ask_delete,
	_ask_trash,
	_abort,
	_start,
	_end,
	_none
} _log_actions_type;

typedef enum
{
	_answer_none,
	_answer_copy_from,
	_answer_copy_to,
	_answer_copy_recent,
	_answer_copy_older,
	_answer_yes,
	_answer_no,
	_answer_ignore,
	_answer_force_ignore_filesystem,
	_answer_force_ignore_new_sync
} _log_answers;

class ac_log_list;
class ac_log_action : ac_common
{
private:
	int						config_id;
	time_t					launch_time;
	long					no;

protected:
	_log_context			context;
	_log_actions_type		action;
	error_infos				*error;
	char					*partial_path;
	time_t					action_time;
	long					from_time;
	long					to_time;
	_log_answers			answer;

public:
							ac_log_action(int config_id, time_t launch_time, long no = 0);
							~ac_log_action();

	void					reset();
	bool					filesystem_modified();
	int						get_config_id();
	_log_actions_type		get_action();
	tc_files_binomial*		get_binomial();
	long					get_from_time_when_registered();
	long					get_to_time_when_registered();
	_log_answers			get_answer();
	char*					get_answer_string();
	time_t					get_action_time();
	time_t					get_launch_time();
	void					answer_question(_log_answers answer);
	bool					is_asking(_log_actions_type action);
	void					commit();

friend class ac_log_list;
};

typedef struct
{
	long			log_no;
	time_t			time;
	char			*message;
} log_timed_message;

class ac_log_list : ac_common
{
private:
	int							config_id;

public:
								ac_log_list(ac_config *config);
								ac_log_list(int config_id);
								~ac_log_list();

	list<time_t>*				get_time_launch_list();
	list<log_timed_message>*	get_errors(time_t time_launch);
	list<log_timed_message>*	get_actions(time_t time_launch);
	list<ac_log_action>*		get_questions(time_t time_launch);
};

void			purge_logs();

#endif /* AC_LOGS_H_ */
