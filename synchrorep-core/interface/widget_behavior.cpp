/*
 * widget_behavior.cpp
 *
 *  Created on: 7 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010,2011 - Sébastien Kus
 *
 *      generic widget to define behavior
 *
 */

#include "widget_behavior.h"

widget_behavior :: widget_behavior(ac_behavior behavior, bool copy_mode) : ac_behavior(behavior)
{
	// define notebook
	this->widget = gtk_notebook_new();

	// define main widgets
	GtkWidget			*general_vbox = gtk_vbox_new(false, 1);
	GtkWidget			*technical_vbox = gtk_vbox_new(false, 1);
	gtk_notebook_append_page(GTK_NOTEBOOK(this->widget), general_vbox, gtk_label_new(gettext("How to manage questions ?")));
	gtk_notebook_append_page(GTK_NOTEBOOK(this->widget), technical_vbox, gtk_label_new(gettext("How to process ?")));


	// define ask at the end
	this->ask_at_the_end_checkbox = gtk_check_button_new_with_label(gettext("Ask all questions at the end of synchronization ?"));
	gtk_box_pack_start(GTK_BOX(general_vbox), this->ask_at_the_end_checkbox, false, false, 1);
	g_signal_connect(this->ask_at_the_end_checkbox, "clicked", G_CALLBACK(this->check_clicked), this);

	// define confirm deletions box
	this->confirm_delete_checkbox = gtk_check_button_new_with_label(gettext("Confirm removes ?"));
	gtk_box_pack_start(GTK_BOX(general_vbox), this->confirm_delete_checkbox, false, false, 1);
	g_signal_connect(this->confirm_delete_checkbox, "clicked", G_CALLBACK(this->check_clicked), this);

	// define put to trash on remove box
	this->delete_to_trash_checkbox = gtk_check_button_new_with_label(gettext("If a file has been removed, place the old version to trash instead of delete it ?"));
	gtk_box_pack_start(GTK_BOX(technical_vbox), this->delete_to_trash_checkbox, false, false, 1);
	g_signal_connect(this->delete_to_trash_checkbox, "clicked", G_CALLBACK(this->check_clicked), this);

	// define put to trash on replace box
	this->replace_to_trash_checkbox = gtk_check_button_new_with_label(gettext("If a file has been modified, place the old version to trash instead of crush it ?"));
	gtk_box_pack_start(GTK_BOX(technical_vbox), this->replace_to_trash_checkbox, false, false, 1);
	g_signal_connect(this->replace_to_trash_checkbox, "clicked", G_CALLBACK(this->check_clicked), this);

	// define what to do on errors
	GtkWidget*          errors_frame = gtk_frame_new(gettext("What to do in case of error ?"));
	gtk_box_pack_start(GTK_BOX(general_vbox), errors_frame, false, false, 1);
	GtkWidget			*errors_frame_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(errors_frame), errors_frame_vbox);
	// define list of possible answers
	this->error_ask = gtk_radio_button_new_with_label(NULL, gettext("Ask user"));
	this->error_ignore = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->error_ask), gettext("Ignore the error and continue"));
	this->error_stop = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->error_ignore), gettext("Stop processing"));
	gtk_box_pack_start(GTK_BOX(errors_frame_vbox), this->error_ask, false, false, 1);
	gtk_box_pack_start(GTK_BOX(errors_frame_vbox), this->error_ignore, false, false, 1);
	gtk_box_pack_start(GTK_BOX(errors_frame_vbox), this->error_stop, false, false, 1);
	// and connect radios to process
	g_signal_connect(this->error_ask, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->error_ignore, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->error_stop, "clicked", G_CALLBACK(this->radio_clicked), this);

	if(!copy_mode)
	{
		// define what to do when modification conflict
		GtkWidget*          modify_frame = gtk_frame_new(gettext("What to do in case of a file modified in both directories ?"));
		gtk_box_pack_start(GTK_BOX(general_vbox), modify_frame, false, false, 1);
		GtkWidget			*modify_frame_vbox = gtk_vbox_new(false, 1);
		gtk_container_add(GTK_CONTAINER(modify_frame), modify_frame_vbox);
		// define list of possible answers
		this->modify_ask = gtk_radio_button_new_with_label(NULL, gettext("Ask user"));
		this->modify_ignore = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->modify_ask), gettext("Do nothing and continue"));
		this->modify_recent = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->modify_ignore), gettext("Copy recent"));
		this->modify_older = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->modify_recent), gettext("Copy older"));
		this->modify_stop = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->modify_older), gettext("Stop processing"));
		gtk_box_pack_start(GTK_BOX(modify_frame_vbox), this->modify_ask, false, false, 1);
		gtk_box_pack_start(GTK_BOX(modify_frame_vbox), this->modify_ignore, false, false, 1);
		gtk_box_pack_start(GTK_BOX(modify_frame_vbox), this->modify_recent, false, false, 1);
		gtk_box_pack_start(GTK_BOX(modify_frame_vbox), this->modify_older, false, false, 1);
		gtk_box_pack_start(GTK_BOX(modify_frame_vbox), this->modify_stop, false, false, 1);
		g_signal_connect(this->modify_ask, "clicked", G_CALLBACK(this->radio_clicked), this);
		g_signal_connect(this->modify_ignore, "clicked", G_CALLBACK(this->radio_clicked), this);
		g_signal_connect(this->modify_recent, "clicked", G_CALLBACK(this->radio_clicked), this);
		g_signal_connect(this->modify_older, "clicked", G_CALLBACK(this->radio_clicked), this);
		g_signal_connect(this->modify_stop, "clicked", G_CALLBACK(this->radio_clicked), this);
	}
	else
	{
		this->modify_ask = NULL;
		this->modify_ignore = NULL;
		this->modify_recent = NULL;
		this->modify_older = NULL;
		this->modify_stop = NULL;
	}

	// define md5sum level
	GtkWidget*          md5sum_frame = gtk_frame_new(gettext("Use md5sum ?"));
	gtk_box_pack_start(GTK_BOX(technical_vbox), md5sum_frame, false, false, 1);
	GtkWidget			*md5sum_frame_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(md5sum_frame), md5sum_frame_vbox);
	// define list of possible answers
	this->md5sum_no = gtk_radio_button_new_with_label(NULL, gettext("Don't use md5sum (fast)"));
	this->md5sum_both_modify = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->md5sum_no), gettext("Use md5sum to check files if both modification date changed (slow)"));
	this->md5sum_database = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->md5sum_both_modify), gettext("Use md5sum to determine all modifications (very slow)"));
	gtk_box_pack_start(GTK_BOX(md5sum_frame_vbox), this->md5sum_no, false, false, 1);
	gtk_box_pack_start(GTK_BOX(md5sum_frame_vbox), this->md5sum_both_modify, false, false, 1);
	gtk_box_pack_start(GTK_BOX(md5sum_frame_vbox), this->md5sum_database, false, false, 1);
	// and connect radios to process
	g_signal_connect(this->md5sum_no, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->md5sum_both_modify, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->md5sum_database, "clicked", G_CALLBACK(this->radio_clicked), this);

	// define log level
	GtkWidget*          log_frame = gtk_frame_new(gettext("What do you want to log ?"));
	gtk_box_pack_start(GTK_BOX(technical_vbox), log_frame, false, false, 1);
	GtkWidget			*log_frame_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(log_frame), log_frame_vbox);
	// define list of possible answers
	this->log_nothing = gtk_radio_button_new_with_label(NULL, gettext("Log nothing"));
	this->log_errors = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->log_nothing), gettext("Log errors"));
	this->log_actions = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(this->log_errors), gettext("Log errors and actions (slow)"));
	gtk_box_pack_start(GTK_BOX(log_frame_vbox), this->log_nothing, false, false, 1);
	gtk_box_pack_start(GTK_BOX(log_frame_vbox), this->log_errors, false, false, 1);
	gtk_box_pack_start(GTK_BOX(log_frame_vbox), this->log_actions, false, false, 1);
	// and connect radios to process
	g_signal_connect(this->log_nothing, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->log_errors, "clicked", G_CALLBACK(this->radio_clicked), this);
	g_signal_connect(this->log_actions, "clicked", G_CALLBACK(this->radio_clicked), this);

	this->refresh();
}

widget_behavior :: ~widget_behavior()
{
}

void
widget_behavior :: refresh()
{
	// setup ask for delete
	if(this->get_alert_when_file_deletion())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->confirm_delete_checkbox), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->confirm_delete_checkbox), false);
	// setup move to trash on deletion
	if(this->get_move_to_trash_when_delete())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->delete_to_trash_checkbox), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->delete_to_trash_checkbox), false);
	// setup move to trash on deletion
	if(this->get_move_to_trash_when_overwrite())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->replace_to_trash_checkbox), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->replace_to_trash_checkbox), false);
	// setup move to trash on deletion
	if(this->get_ask_at_the_end())
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->ask_at_the_end_checkbox), true);
	else
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->ask_at_the_end_checkbox), false);
	// setup what to do on error
	switch(this->get_to_do_on_error())
	{
		case _question_on_error:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->error_ask), true);
		break;
		case _ignore_error:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->error_ignore), true);
		break;
		case _stop_on_error:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->error_stop), true);
		break;
	}
	if(this->modify_ask != NULL)
		// setup what to do on modify conflict
		switch(this->get_to_do_on_modify_conflict())
		{
			case _question_on_modify:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->modify_ask), true);
			break;
			case _ignore_modifications:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->modify_ignore), true);
			break;
			case _get_recent_modified:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->modify_recent), true);
			break;
			case _get_older_modified:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->modify_older), true);
			break;
			case _stop_on_modified:
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->modify_stop), true);
			break;
		}
	// setup what how use md5sum
	switch(this->get_md5sum_level())
	{
		case _no_md5sum:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->md5sum_no), true);
		break;
		case _both_modified_md5sum:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->md5sum_both_modify), true);
		break;
		case _database_md5sum:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->md5sum_database), true);
		break;
	}
	// setup what how use md5sum
	switch(this->get_log_level())
	{
		case _log_nothing:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->log_nothing), true);
		break;
		case _log_errors:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->log_errors), true);
		break;
		case _log_actions:
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->log_actions), true);
		break;
	}
}

void
widget_behavior :: check_clicked(GtkWidget *btn, gpointer THISP)
{
	widget_behavior		*THIS = (widget_behavior*)THISP;
	bool				active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));

	if(btn == THIS->confirm_delete_checkbox)
		THIS->set_alert_when_file_deletion(active);
	if(btn == THIS->delete_to_trash_checkbox)
		THIS->set_move_to_trash_when_delete(active);
	if(btn == THIS->replace_to_trash_checkbox)
		THIS->set_move_to_trash_when_overwrite(active);
	if(btn == THIS->ask_at_the_end_checkbox)
		THIS->set_ask_at_the_end(active);
}

void
widget_behavior :: radio_clicked(GtkWidget *btn, gpointer THISP)
{
	widget_behavior		*THIS = (widget_behavior*)THISP;
	if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)))
		return;

	if(btn == THIS->error_ask)
		THIS->set_to_do_on_error(_question_on_error);
	if(btn == THIS->error_ignore)
		THIS->set_to_do_on_error(_ignore_error);
	if(btn == THIS->error_stop)
		THIS->set_to_do_on_error(_stop_on_error);
	if(btn == THIS->modify_ask)
		THIS->set_to_do_on_modify_conflict(_question_on_modify);
	if(btn == THIS->modify_ignore)
		THIS->set_to_do_on_modify_conflict(_ignore_modifications);
	if(btn == THIS->modify_recent)
		THIS->set_to_do_on_modify_conflict(_get_recent_modified);
	if(btn == THIS->modify_older)
		THIS->set_to_do_on_modify_conflict(_get_older_modified);
	if(btn == THIS->modify_stop)
		THIS->set_to_do_on_modify_conflict(_stop_on_modified);
	if(btn == THIS->md5sum_no)
		THIS->set_md5sum_level(_no_md5sum);
	if(btn == THIS->md5sum_both_modify)
		THIS->set_md5sum_level(_both_modified_md5sum);
	if(btn == THIS->md5sum_database)
		THIS->set_md5sum_level(_database_md5sum);
	if(btn == THIS->log_nothing)
		THIS->set_log_level(_log_nothing);
	if(btn == THIS->log_errors)
		THIS->set_log_level(_log_errors);
	if(btn == THIS->log_actions)
		THIS->set_log_level(_log_actions);
}
