/*
 * window_ask_at_the_end.cpp
 *
 *  Created on: 30 oct. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  show window of pending actions
 *
 */

#include "window_ask_at_the_end.h"

#include "../accessors/ac_config.h"

#include "../applicative/application.h"

#include <iostream>
#include <sstream>
#include "string.h"

window_ask_at_the_end :: window_ask_at_the_end(int config_id, time_t launch_time) : ask_at_the_end_widget(config_id, launch_time)
{
	if(this->ask_at_the_end_widget.widget != NULL)
	{
		// base of window
		ostringstream		title;
		title<<"Synchrorep - "<<gettext("Pending actions");
		this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_modal(GTK_WINDOW(this->window), true);
		gtk_window_set_title(GTK_WINDOW(this->window), title.str().c_str());
		gtk_window_set_icon_from_file(GTK_WINDOW(this->window), application :: get_file_path(ICON_FILE), NULL);

		// principal vbox
		GtkWidget				*main_vbox = gtk_vbox_new(false, 1);

		// add message label
		char					message_text[1000];
		char					*final_message;
		ac_config				config(config_id);
		char					time_string[100];
		strftime(time_string, 256, "%c", localtime(&launch_time));
		strcpy(message_text, gettext("Here are pending questions of synchronization between"));
		strcat(message_text, " '");
		strcat(message_text, config.get_from_folder_uri());
		strcat(message_text, "' ");
		strcat(message_text, gettext("and"));
		strcat(message_text, " '");
		strcat(message_text, config.get_to_folder_uri());
		strcat(message_text, "' ");
		strcat(message_text, gettext("started on "));
		strcat(message_text, time_string);
		final_message = cut_line_word(message_text, 100);
		GtkWidget				*message = gtk_label_new(final_message);
		gtk_box_pack_start(GTK_BOX(main_vbox), message, true, true, 1);

		// finish main vbox with ask at the end widget
		gtk_box_pack_start(GTK_BOX(main_vbox), this->ask_at_the_end_widget.widget, true, true, 1);

		// finish window
		gtk_container_add(GTK_CONTAINER(this->window), main_vbox);
		g_signal_connect(this->window, "destroy", G_CALLBACK(this->close_window), NULL);
		gtk_widget_show_all(this->window);
		gtk_main();
	}
}

window_ask_at_the_end :: ~window_ask_at_the_end()
{
}

void
window_ask_at_the_end :: close_window(GtkWindow *window, gpointer null_data)
{
	gtk_main_quit();
}
