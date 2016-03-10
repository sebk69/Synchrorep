/*
 * log.h
 *
 *  Created on: 26 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009-2010 - Sébastien Kus
 *
 *  log events class
 */

#ifndef LOG_H_
#define LOG_H_

#include <gio/gio.h>
#include "../technical/tc_files_binomial.h"
#include "../interface/dialog_error.h"

#include "../accessors/ac_config.h"
#include "../accessors/ac_logs.h"

typedef enum
{
	_silent,
	_verbose,
	_ultra_verbose
} _verbosing;

class logging : private ac_log_action
{
private:
	tc_files_binomial				*current_binomial;
	ac_config						*config;

public:
	_verbosing						verbose;

									logging(ac_config *current_config, time_t launch_time);
									~logging();

	void							set_context(_log_context context);
	void							start_action(_log_actions_type action, tc_files_binomial *files);
	void							answer_question(_log_answers answer);
	void							close_action();
	error_infos						get_error_context(GError *error);
	error_infos						get_error();
};

#endif /* LOG_H_ */
