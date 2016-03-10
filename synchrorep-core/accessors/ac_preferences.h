/*
 * ac_behaviour.h
 *
 * Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 * accessor to preferences
 *
 */

#ifndef AC_PREFERENCES_H_
#define AC_PREFERENCES_H_

#include "ac_common.h"

// accessor of preferences
class ac_preferences : public ac_common
{
private:
	bool					expert_mode;
	GFile					*contextual_menu_file_param;
	unsigned int			log_duration;

public:
							ac_preferences();
							~ac_preferences();

	bool					get_expert_mode();
	void					set_expert_mode(bool activate);

	bool					get_contextual_menu();
	void					set_contextual_menu(bool activate);

	unsigned int			get_log_duration();
	void					set_log_duration(unsigned int duration);
};

#endif
