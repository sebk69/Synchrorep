/*
 * dialog_info.cpp
 *
 *  Created on: 24 jan. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2010 - Sébastien Kus
 *
 *  User dialog to show info message
 *
 */

#include <iostream>
#include <sstream>

#include "dialog_info.h"

dialog_info :: dialog_info(GtkWidget *parent)
{
	this->parent = parent;
}

dialog_info :: ~dialog_info()
{
}

bool
dialog_info :: asking(char *infos)
{
	ostringstream		msg, title;
	title<<"Synchrorep - "<<gettext("Information");
	GtkWidget			*ask_dialog = gtk_message_dialog_new(GTK_WINDOW(this->parent),
										(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_NONE,
										infos);
	gtk_window_set_title(GTK_WINDOW(ask_dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(ask_dialog), application :: get_file_path(ICON_FILE), NULL);


	gtk_widget_show_all(ask_dialog);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), GTK_STOCK_OK, 1);
	gtk_dialog_run(GTK_DIALOG(ask_dialog));
	gtk_widget_destroy(ask_dialog);

	this->choosen = true;

	return true;
}
