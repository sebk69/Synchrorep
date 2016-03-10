/*
 * dialog_file_conflict.cpp
 *
 *  Created on: 22 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask what to do if two files are modified
 *
 */
#include <iostream>
#include <sstream>
#include <string.h>

#include "dialog_file_conflict.h"

dialog_file_conflict :: dialog_file_conflict(GtkWidget *parent)
{
	int				i;
	for(i=0;i<=5;i++)
		this->responses[i] = i+1;
	this->always = false;
	this->choosen = _conflict_choose_recent;
	this->parent = parent;
}

dialog_file_conflict :: ~dialog_file_conflict()
{

}

void
dialog_file_conflict :: no_confirmation(conflict_choose default_answer)
{
	this->always = true;
	this->choosen = default_answer;
}

conflict_choose
dialog_file_conflict :: asking(tc_files_binomial *files)
{
	if(this->always)
		return this->choosen;

	// new dialog
	GtkWidget			*dialog = gtk_dialog_new();
	//gtk_widget_set_parent(dialog, this->parent);
	ostringstream		title;
	title<<"Synchrorep";
	gtk_window_set_title(GTK_WINDOW(dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(dialog), application :: get_file_path(ICON_FILE), NULL);
	gtk_dialog_set_has_separator(GTK_DIALOG(dialog), false);

	// define information zone
	ostringstream		msg;
	tc_file_with_infos	*source = files->get_from();
	tc_file_with_infos	*target = files->get_to();
	time_t				time_val_source = source->get_modification_time().tv_sec + files->from_dif;
	time_t				time_val_target = target->get_modification_time().tv_sec+ files->to_dif;
	char				source_modification_string[256];
	char				target_modification_string[256];
	char				*source_folder = g_uri_unescape_string(source->get_parent_uri(), NULL);
	char				*target_folder = g_uri_unescape_string(target->get_parent_uri(), NULL);
	char				*file_name = source->get_base_name();

	strftime(source_modification_string, 256, "%c", localtime(&time_val_source));
	strftime(target_modification_string, 256, "%c", localtime(&time_val_target));

	msg<<gettext("The file ")<<file_name<<gettext(" has been modified in both directories since last synchronization")<<"\n\n";
	msg<<gettext("Source folder")<<" : "<<source_folder<<"\n("<<gettext("Modified on")<<" "<<source_modification_string<<")\n\n";

	// from warning
	char				prior_after[256];
	unsigned int		non_negative_dif;
	if(files->from_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)files->from_dif;
	if(non_negative_dif != 0)
	{
		msg<<gettext("Warning : The remote system time is ")<<non_negative_dif<<gettext(" seconds ")<<prior_after<<gettext(" your system time.")<<"\n";
		msg<<gettext("The displayed time has been converted to your system time.")<<"\n";
	}
	msg<<gettext("Target folder")<<" : "<<target_folder<<"\n("<<gettext("Modified on")<<" "<<target_modification_string<<")\n";
	// to warning
	if(files->to_dif < 0)
		strcpy(prior_after, "prior");
	else
		strcpy(prior_after, "after");
	non_negative_dif = (unsigned int)files->to_dif;
	if(non_negative_dif != 0)
	{
		msg<<gettext("Warning : The remote system time is ")<<non_negative_dif<<gettext(" seconds ")<<prior_after<<gettext(" your system time.")<<"\n";
		msg<<gettext("The displayed time has been converted to your system time.")<<"\n";
	}

	delete source;
	delete target;
	free(source_folder);
	free(target_folder);
	free(file_name);

	GtkWidget			*content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkWidget			*content_text = gtk_label_new(msg.str().c_str());
	gtk_box_pack_start(GTK_BOX(content_area), content_text, true, true, 1);

	// define buttons
	GtkWidget			*check_button =  gtk_check_button_new_with_label(gettext("Always do this action"));
	gtk_box_pack_start(GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), check_button, true, true, 1);
	g_signal_connect(G_OBJECT(check_button), "clicked", (GCallback)dialog_file_conflict :: always_hook, this);

	GtkWidget			*action = gtk_dialog_get_action_area(GTK_DIALOG(dialog));

	GtkWidget			*col1 = gtk_vbutton_box_new();
	GtkWidget			*col2 = gtk_vbutton_box_new();
	GtkWidget			*col3 = gtk_vbutton_box_new();

	gtk_box_pack_start(GTK_BOX(action), col1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(action), col2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(action), col3, false, false, 1);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(col1), GTK_BUTTONBOX_START);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(col2), GTK_BUTTONBOX_START);
	gtk_button_box_set_layout(GTK_BUTTON_BOX(col3), GTK_BUTTONBOX_START);

	GtkWidget			*copy_recent_btn = gtk_button_new_with_label(gettext("Copy recent"));
	gtk_box_pack_start(GTK_BOX(col1), copy_recent_btn, false, false, 1);
	g_signal_connect(G_OBJECT(copy_recent_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[0]);

	GtkWidget			*copy_older_btn = gtk_button_new_with_label(gettext("Copy older"));
	gtk_box_pack_start(GTK_BOX(col1), copy_older_btn, false, false, 1);
	g_signal_connect(G_OBJECT(copy_older_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[2]);

	GtkWidget			*copy_source_btn = gtk_button_new_with_label(gettext("Copy from source"));
	gtk_box_pack_start(GTK_BOX(col2), copy_source_btn, false, false, 1);
	g_signal_connect(G_OBJECT(copy_source_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[1]);

	GtkWidget			*copy_target_btn = gtk_button_new_with_label(gettext("Copy from target"));
	gtk_box_pack_start(GTK_BOX(col2), copy_target_btn, false, false, 1);
	g_signal_connect(G_OBJECT(copy_target_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[3]);

	GtkWidget			*nothing_btn = gtk_button_new_with_label(gettext("Do nothing"));
	gtk_box_pack_start(GTK_BOX(col3), nothing_btn, false, false, 1);
	g_signal_connect(G_OBJECT(nothing_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[4]);

	GtkWidget			*stop_btn = gtk_button_new_with_label(gettext("Stop process"));
	gtk_box_pack_start(GTK_BOX(col3), stop_btn, false, false, 1);
	g_signal_connect(G_OBJECT(stop_btn), "clicked", (GCallback)dialog_file_conflict :: response_hook, &this->responses[5]);

	// run dialog and interpret result
	gtk_widget_show_all(dialog);
	switch(gtk_dialog_run(GTK_DIALOG(dialog)))
	{
		case 1:
			this->choosen = _conflict_choose_recent;
		break;

		case 3:
			this->choosen = _conflict_choose_older;
		break;

		case 2:
			this->choosen = _conflict_choose_source;
		break;

		case 4:
			this->choosen = _conflict_choose_target;
		break;

		case 5:
			this->choosen = _conflict_choose_ignore;
		break;

		case 6:
			this->choosen = _conflict_choose_cancel;
		break;

		default:
			this->choosen = _conflict_choose_ignore;
		break;
	}

	gtk_widget_destroy(dialog);

	return this->choosen;
}

void
dialog_file_conflict :: response_hook(GtkWidget *btn, gpointer response)
{
	GtkWidget				*window = gtk_widget_get_ancestor(btn, GTK_TYPE_WINDOW);
	gtk_dialog_response(GTK_DIALOG(window), *(int*)response);
}

void
dialog_file_conflict :: always_hook(GtkWidget *btn, gpointer dialog_class)
{
	 if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)))
		 ((dialog_file_conflict*)dialog_class)->always = true;
	 else
		 ((dialog_file_conflict*)dialog_class)->always = false;
}
