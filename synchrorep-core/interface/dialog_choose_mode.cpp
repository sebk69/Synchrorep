/*
 * dialog_choose_mode.cpp
 *
 *  Created on: 15 apr. 2011
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2011 - Sébastien Kus
 *
 *  User dialog to ask mode (synchronization, copy)
 *
 */

#include <iostream>
#include <sstream>

#include "dialog_choose_mode.h"

#include "../technical/tc_misc.h"

dialog_choose_mode :: dialog_choose_mode()
{
}

dialog_choose_mode :: ~dialog_choose_mode()
{
}

mode_choose
dialog_choose_mode :: asking()
{
	// ask user to which mode to create configuration
	ostringstream		msg, title;
	mode_choose			result;


	title<<"Synchrorep - "<<gettext("New configuration");
	msg<<cut_line_word(gettext("Do you want to synchronize this folder with another folder or copy it to another folder ?"), 255);
	GtkWidget			*ask_dialog = gtk_message_dialog_new(NULL,
										(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_NONE,
										msg.str().c_str());
	gtk_window_set_title(GTK_WINDOW(ask_dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(ask_dialog), application :: get_file_path(ICON_FILE), NULL);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), gettext("Cancel"), 1);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), gettext("Synchronize"), 2);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), gettext("Copy"), 3);
	gtk_widget_show_all(ask_dialog);
	switch(gtk_dialog_run(GTK_DIALOG(ask_dialog)))
	{
		case 1:
			result = _mode_choose_cancel;
		break;

		case 2:
			result = _mode_choose_sync;
		break;

		case 3:
			result = _mode_choose_copy;
		break;

		default:
			result = _mode_choose_cancel;
	}
	gtk_widget_destroy(ask_dialog);

	return result;
}
