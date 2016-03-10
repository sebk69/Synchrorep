/*
 * window_default_behavior.cpp
 *
 *  Created on: 7 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010,2011 - Sébastien Kus
 *
 *  setup default behavior
 *
 */

#include "window_default_behavior.h"

#include "../applicative/application.h"

#include <iostream>
#include <sstream>

window_default_behavior :: window_default_behavior() : behavior(*(new ac_behavior), false)
{
	ostringstream		title;
	title<<"Synchrorep - "<<gettext("Default behavior");

	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(this->window), true);
	gtk_window_set_title(GTK_WINDOW(this->window), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(this->window), application :: get_file_path(ICON_FILE), NULL);

	GtkWidget			*main_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(this->window), main_vbox);
	gtk_box_pack_start(GTK_BOX(main_vbox), this->behavior.widget, true, true, 1);

	GtkWidget			*validate_button = gtk_button_new_with_label(gettext("Done"));;
	gtk_box_pack_start(GTK_BOX(main_vbox), validate_button, true, true, 1);

	g_signal_connect(validate_button, "clicked", G_CALLBACK(this->close_window), this->window);
	g_signal_connect(this->window, "destroy", G_CALLBACK(this->close_window), this->window);

	gtk_widget_show_all(this->window);
	gtk_main();
}

window_default_behavior :: ~window_default_behavior()
{
	this->behavior.commit();
}

void
window_default_behavior :: close_window(GtkWidget *widget, gpointer window)
{
	gtk_widget_destroy((GtkWidget*)window);
	gtk_main_quit();
}
