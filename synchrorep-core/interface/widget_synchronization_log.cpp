/*
 * widget_synchronization_log.cpp
 *
 *  Created on: 19 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *      generic widget to show content of a log for a synchronization
 *
 */

#include "widget_synchronization_log.h"

#include "../accessors/ac_logs.h"
#include "../technical/tc_misc.h"

#include <iostream>
#include <sstream>

widget_synchronization_log :: widget_synchronization_log(int config_id, time_t time_of_launch)
{
	this->config_id = config_id;
	this->time_of_launch = time_of_launch;
	this->widget = gtk_vbox_new(false, 1);
	this->scroll_box = NULL;
	this->build();
}

widget_synchronization_log :: ~widget_synchronization_log()
{
	gtk_widget_destroy(this->widget);
}

void
widget_synchronization_log :: build()
{
	gdouble					scroll_value = 0;
	if(this->scroll_box != NULL)
	{
		GtkWidget				*old_vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(this->scroll_box));
		scroll_value = gtk_range_get_value(GTK_RANGE(old_vscroll));
		gtk_container_remove(GTK_CONTAINER(this->widget), this->scroll_box);
	}

	GtkWidget							*vbox = gtk_vbox_new(false, 1);
	ac_log_list							log(this->config_id);

	list<log_timed_message>				*errors = log.get_errors(this->time_of_launch);
	list<log_timed_message>				*actions = log.get_actions(this->time_of_launch);
	list<log_timed_message> :: iterator	errors_it;
	list<log_timed_message> :: iterator	actions_it;
	bool								errors_ended = false;
	bool								actions_ended = false;
	if(errors != NULL)
		errors_it = errors->begin();
	else
		errors_ended = true;
	if(actions != NULL)
		actions_it = actions->begin();
	else
		actions_ended = true;
	while(!(errors_ended && actions_ended))
	{
		if(!errors_ended && (actions_ended || (*errors_it).time <= (*actions_it).time))
		{
			ostringstream		message;
			char				time_string[100];
			strftime(time_string, 256, "%c", localtime(&(*errors_it).time));
			message<<underline(time_string)<<" : "<<(*errors_it).message;
			GtkWidget			*label = gtk_label_new(cut_line_word((char*)message.str().c_str(), 100));
			gtk_label_set_use_underline(GTK_LABEL(label), true);
			GdkColor			color;
			gdk_color_parse("red", &color);
			gtk_widget_modify_fg (label, GTK_STATE_NORMAL, &color);
			GtkWidget			*hbox = gtk_hbox_new(false, 1);
			gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 1);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 1);
			errors_it++;
		}
		if(!actions_ended && (errors_ended || (*errors_it).time >= (*actions_it).time))
		{
			ostringstream		message;
			char				time_string[100];
			strftime(time_string, 256, "%c", localtime(&(*actions_it).time));
			message<<underline(time_string)<<" : "<<(*actions_it).message;
			GtkWidget			*label = gtk_label_new(cut_line_word((char*)message.str().c_str(), 100));
			gtk_label_set_use_underline(GTK_LABEL(label), true);
			GtkWidget			*hbox = gtk_hbox_new(false, 1);
			gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 1);
			gtk_box_pack_start(GTK_BOX(vbox), hbox, false, false, 1);
			actions_it++;
		}
		if(errors_it == errors->end())
			errors_ended = true;
		if(actions_it == actions->end())
			actions_ended = true;
	}
	delete errors;
	delete actions;

	//gtk_box_pack_start(GTK_BOX(hbox), label, false, false, 1);
	this->scroll_box = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(this->scroll_box), vbox);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scroll_box), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);

	gtk_box_pack_start(GTK_BOX(this->widget), this->scroll_box, true, true, 1);
	gtk_widget_show_all(this->widget);
	GtkWidget				*new_vscroll = gtk_scrolled_window_get_vscrollbar(GTK_SCROLLED_WINDOW(this->scroll_box));
	gtk_range_set_value(GTK_RANGE(new_vscroll), scroll_value);
}
