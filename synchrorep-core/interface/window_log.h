/*
 * window_log.h
 *
 *  Created on: 18 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *      log management window
 *
 */

#ifndef WINDOW_LOG_H_
#define WINDOW_LOG_H_

#include "../applicative/application.h"

#include "widget_synchronization_log.h"
#include "widget_ask_at_the_end.h"
#include "../accessors/ac_preferences.h"

typedef struct
{
	int				config_key;
	time_t			launch_time;
} log_key;

class window_log : ac_preferences
{
private:
	GtkWidget						*window;
	GtkWidget						*main_hbox;
	GtkWidget						*scrolled_tree;
	GtkWidget						*launches_tree;
	GtkWidget						*log_duration_btn;
	widget_synchronization_log		*log_widget;
	widget_ask_at_the_end			*questions_widget;
	GtkWidget						*notebook;

	log_key							**keys;

	static void						select_log(GtkTreeSelection *selection, gpointer data);
	static void						change_log_duration(GtkWidget *widget, gpointer data);
public:
									window_log();
									~window_log();
};

#endif /* WINDOW_LOG_H_ */
