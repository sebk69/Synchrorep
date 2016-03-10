/*
 * widget_ask_at_the_end.h
 *
 *  Created on: 26 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2010 - Sébastien Kus
 *
 *  Widget to ask questions of a synchronization at the end
 *
 */

#ifndef WIDGET_ASK_AT_THE_END_H_
#define WIDGET_ASK_AT_THE_END_H_

#include "../applicative/application.h"
#include "../accessors/ac_logs.h"
#include "widget_synchronization_log.h"


class widget_ask_at_the_end;
typedef struct
{
	GtkWidget					*answer_label;
	GtkWidget					*btn_area;
	ac_log_action				*action;
	widget_ask_at_the_end		*THIS;
} answering_data;


class widget_ask_at_the_end
{
private:
	widget_synchronization_log		*log_widget;
	guint							bg_id;

	list<answering_data*>				answer_datas;
	list<answering_data*> :: iterator	answer_datas_it;

	GtkWidget*						build_question(ac_log_action *action);
	static void						answer_delete_yes(GtkWidget *btn, gpointer pdata);
	static void						answer_delete_no(GtkWidget *btn, gpointer pdata);
	static void						answer_copy_recent(GtkWidget *btn, gpointer pdata);
	static void						answer_copy_older(GtkWidget *btn, gpointer pdata);
	static void						answer_copy_from(GtkWidget *btn, gpointer pdata);
	static void						answer_copy_to(GtkWidget *btn, gpointer pdata);
	static void						answer_ignore_modifications(GtkWidget *btn, gpointer pdata);
	static bool						check_filesystem(gpointer data);

public:
	GtkWidget						*widget;

									widget_ask_at_the_end(int config_id, time_t launch_time, widget_synchronization_log *log = NULL);
									~widget_ask_at_the_end();
};

#endif /* WIDGET_ASK_AT_THE_END_H_ */
