/*
 * dialog_delete.cpp
 *
 *  Created on: 18 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask what to to if error occured
 *
 */

#include <iostream>
#include <sstream>

#include "dialog_error.h"

dialog_error :: dialog_error(GtkWidget *parent)
{
	// by default we ignore
	this->choosen = _error_choose_ignore;

	// if no confirmation, do always the same response without dialog
	this->always = false;

	this->parent = parent;
}

dialog_error :: ~dialog_error()
{
}

void
dialog_error :: no_confirmation(error_choose choosen)
{
	this->always = true;
	this->choosen = choosen;
}

error_choose
dialog_error :: asking(error_infos infos)
{
	// do always the same response without dialog if set
	if(this->always)
		return this->choosen;

	// else, ask user to what to do
	ostringstream		msg, title;
	title<<"Synchrorep - "<<gettext("Error");
	GtkWidget			*ask_dialog = gtk_message_dialog_new(GTK_WINDOW(this->parent),
										(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
										GTK_MESSAGE_QUESTION,
										GTK_BUTTONS_NONE,
										infos.context);
	gtk_window_set_title(GTK_WINDOW(ask_dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(ask_dialog), application :: get_file_path(ICON_FILE), NULL);

	// if precised, show reason
	if(infos.reason != NULL)
	{
		GtkTreeIter			toplevel, current;
		GtkTreeStore		*datas = gtk_tree_store_new(1, G_TYPE_STRING);
		gtk_tree_store_append(datas, &toplevel, NULL);
		gtk_tree_store_set(datas, &toplevel, 0, gettext("Show detail"), -1);
		gtk_tree_store_append(datas, &current, &toplevel);
		gtk_tree_store_set(datas, &current, 0, infos.reason, -1);

		GtkWidget			*reason_tree = gtk_tree_view_new();
		GtkCellRenderer		*renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn	*column1 = gtk_tree_view_column_new();
		gtk_tree_view_column_pack_start(column1, renderer, TRUE);
		gtk_tree_view_column_add_attribute(column1, renderer, "text", 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(reason_tree), column1);
		gtk_tree_view_set_model(GTK_TREE_VIEW(reason_tree), GTK_TREE_MODEL(datas));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(reason_tree), false);

		gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(ask_dialog))), reason_tree, false, false, 1);
	}

	GtkWidget			*check_button =  gtk_check_button_new_with_label(gettext("Always do this action"));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(ask_dialog))), check_button, false, false, 1);
	gtk_widget_show_all(ask_dialog);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), gettext("Ignore the error and continue"), 1);
	gtk_dialog_add_button(GTK_DIALOG(ask_dialog), GTK_STOCK_STOP, 2);
	g_signal_connect(G_OBJECT(check_button), "clicked", (GCallback)dialog_error :: always_hook, this);
	switch(gtk_dialog_run(GTK_DIALOG(ask_dialog)))
	{
		case 1:
			this->choosen = _error_choose_ignore;
		break;

		case 2:
			this->choosen = _error_choose_cancel;
		break;

		default:
			this->choosen = _error_choose_ignore;
	}
	gtk_widget_destroy(ask_dialog);

	return this->choosen;
}

void
dialog_error :: always_hook(GtkWidget *btn, gpointer dialog_class)
{
	 if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)))
		 ((dialog_error*)dialog_class)->always = true;
	 else
		 ((dialog_error*)dialog_class)->always = false;
}
