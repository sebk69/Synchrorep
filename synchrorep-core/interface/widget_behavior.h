/*
 * widget_behaviour.h
 *
 *  Created on: 7 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010,2011 - Sébastien Kus
 *
 *      generic widget to define behavior
 *
 */

#ifndef WIDGET_BEHAVIOR_H_
#define WIDGET_BEHAVIOR_H_

#include "../applicative/application.h"

#include "../accessors/ac_behavior.h"

class widget_behavior : public ac_behavior
{
private:
	GtkWidget				*confirm_delete_checkbox,
							*delete_to_trash_checkbox,
							*replace_to_trash_checkbox,
							*ask_at_the_end_checkbox,
							*error_ask,
							*error_ignore,
							*error_stop,
							*modify_ask,
							*modify_ignore,
							*modify_recent,
							*modify_older,
							*modify_stop,
							*md5sum_no,
							*md5sum_both_modify,
							*md5sum_database,
							*log_nothing,
							*log_errors,
							*log_actions;

	static void				radio_clicked(GtkWidget *btn, gpointer THISP);
	static void				check_clicked(GtkWidget *btn, gpointer THISP);
	void					refresh();

public:
	GtkWidget				*widget;

							widget_behavior(ac_behavior behavior, bool copy_mode);
							~widget_behavior();
};

#endif /* WIDGET_BEHAVIOR_H_ */
