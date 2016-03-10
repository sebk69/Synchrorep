/*
 * dialog_delete.cpp
 *
 *  Created on: 18 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask if delete
 *
 */

#include <iostream>
#include <sstream>

#include "dialog_delete.h"

dialog_delete :: dialog_delete(GtkWidget *parent)
{
	// by default we delete
	this->choosen = _delete_choose_yes;

	// if no confirmation, do always the same response without dialog
	this->always = false;

	this->parent = parent;
}

dialog_delete :: ~dialog_delete()
{
}

void
dialog_delete :: no_confirmation(delete_choose choosen)
{
	this->always = true;
	this->choosen = choosen;
}

delete_choose
dialog_delete :: asking(char *file)
{
	// do always the same response without dialog if set
	if(this->always)
		return this->choosen;

	// else, ask user to what to do
	ostringstream		msg, title;
	title<<"Synchrorep - "<<gettext("Delete");
	msg<<file<<" "<<gettext("has been removed since last synchronization.\n Delete the other ?");
	GtkWidget			*ask_dialog = gtk_message_dialog_new(GTK_WINDOW(this->parent),
										(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_NONE,
										msg.str().c_str());
	gtk_window_set_title(GTK_WINDOW(ask_dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(ask_dialog), application :: get_file_path(ICON_FILE), NULL);
	GtkWidget			*check_button =  gtk_check_button_new_with_label(gettext("Always do this action"));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(ask_dialog))), check_button, false, false, 1);
	gtk_widget_show_all(ask_dialog);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), GTK_STOCK_YES, 1);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), GTK_STOCK_NO, 2);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), GTK_STOCK_STOP, 3);
	g_signal_connect(G_OBJECT(check_button), "clicked", (GCallback)dialog_delete :: always_hook, this);
	switch(gtk_dialog_run(GTK_DIALOG(ask_dialog)))
	{
		case 1:
			this->choosen = _delete_choose_yes;
		break;

		case 2:
			this->choosen = _delete_choose_no;
		break;

		case 3:
			this->choosen = _delete_choose_cancel;
		break;

		default:
			this->choosen = _delete_choose_no;
	}
	gtk_widget_destroy(ask_dialog);

	return this->choosen;
}

void
dialog_delete :: always_hook(GtkWidget *btn, gpointer dialog_class)
{
	 if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)))
		 ((dialog_delete*)dialog_class)->always = true;
	 else
		 ((dialog_delete*)dialog_class)->always = false;
}
