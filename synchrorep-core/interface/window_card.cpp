/*
 * window_default_behavior.cpp
 *
 *  Created on: 7 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010-2011 - Sébastien Kus
 *
 *  setup default behavior
 *
 */

#include "window_card.h"
#include "dialog_info.h"

#include "../applicative/application.h"

#include "../accessors/ac_preferences.h"

#include "../technical/tc_misc.h"

#include <iostream>
#include <sstream>
#include "string.h"

window_card :: window_card(ac_config *config) : behavior(*config->behavior, config->get_mode() == _copy)
{
	this->original_config = config;
	ostringstream		title;
	title<<"Synchrorep - "<<gettext("Card");

	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_modal(GTK_WINDOW(this->window), true);
	gtk_window_set_title(GTK_WINDOW(this->window), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(this->window), application :: get_file_path(ICON_FILE), NULL);
	// main vbox
	GtkWidget				*main_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(this->window), main_vbox);
	// Folders to synchronize
	GtkWidget			*folders_frame = gtk_frame_new(gettext("Folders to synchronize"));
	GtkWidget			*folders_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(folders_frame), folders_vbox);
	gtk_box_pack_start(GTK_BOX(main_vbox), folders_frame, true, true, 1);
	// from button
	GtkWidget			*from_btn = gtk_label_new(cut_uri(g_uri_unescape_string(config->get_from_folder_uri(), NULL), 80));
	char				*markup;
	markup = g_markup_printf_escaped("<span foreground=\"#005000\">%s</span>", cut_uri(g_uri_unescape_string(config->get_from_folder_uri(), NULL), 80));
	gtk_label_set_markup (GTK_LABEL(from_btn), markup);
	gtk_box_pack_start(GTK_BOX(folders_vbox), from_btn, true, true, 1);
	// copy or synchronize
	GtkWidget				*copy_sync_frame;
	if(config->get_mode() == _sync)
	{
		copy_sync_frame = gtk_label_new(gettext("Synchronize with"));
		ostringstream		label;
		label<<"<b>"<<gettext("Synchronize with")<<"</b>";
		gtk_label_set_markup(GTK_LABEL(copy_sync_frame), label.str().c_str());
	}
	else
	{
		copy_sync_frame = gtk_label_new(gettext("Copy to"));
		ostringstream		label;
		label<<"<b>"<<gettext("Copy to")<<"</b>";
		gtk_label_set_markup (GTK_LABEL(copy_sync_frame), label.str().c_str());
	}
	gtk_box_pack_start(GTK_BOX(folders_vbox), copy_sync_frame, true, true, 1);
	// to button
	GtkWidget			*to_btn = gtk_label_new(cut_uri(g_uri_unescape_string(config->get_to_folder_uri(), NULL), 80));
	markup = g_markup_printf_escaped("<span foreground=\"#005000\">%s</span>", cut_uri(g_uri_unescape_string(config->get_to_folder_uri(), NULL), 80));
	gtk_label_set_markup (GTK_LABEL(to_btn), markup);
	gtk_box_pack_start(GTK_BOX(folders_vbox), to_btn, true, true, 1);

	// group entry
	GtkWidget			*group_hbox = gtk_hbox_new(false, 1);
	GtkWidget			*group_label = gtk_label_new(gettext("Group name"));
	char				**groups = config->get_groups();
	int					i;
	int					j = 0;
	GtkTreeIter			current;
	GtkListStore		*datas = gtk_list_store_new(1, G_TYPE_STRING, NULL);
	for(i=0; groups[i] != NULL; i++)
	{
		if(groups[i][0] != '\0')
			j++;
		gtk_list_store_append(datas, &current);
		gtk_list_store_set(datas, &current, 0, groups[i], -1);
	}
	// create entry combo
	GtkWidget			*group_combo = gtk_combo_box_entry_new_with_model(GTK_TREE_MODEL(datas), 0);
	this->group_entry = gtk_bin_get_child(GTK_BIN(group_combo));
	gtk_entry_set_text(GTK_ENTRY(this->group_entry), config->get_group());
	gtk_box_pack_start(GTK_BOX(group_hbox), group_label, false, false, 1);
	gtk_box_pack_start(GTK_BOX(group_hbox), group_combo, true, true, 1);
	gtk_box_pack_start(GTK_BOX(folders_vbox), group_hbox, true, true, 1);
	this->new_group = NULL;
	// show help message if no group is defined
	if(j == 0)
		gtk_box_pack_start(GTK_BOX(folders_vbox), gtk_label_new(gettext("Group name is used to synchronize more that one synchronization at a time.\nIf you set a group name identical for several synchronizations, a new menu item\nwill appear in main application menu to launch all synchronization of this group.")), true, true, 1);

	ac_preferences				prefs;
	GtkWidget				*is_default_behavior = gtk_check_button_new_with_label(gettext("Default behavior"));
	if(prefs.get_expert_mode())
	{
		// default behavior checkbox
		gtk_box_pack_start(GTK_BOX(folders_vbox), is_default_behavior, true, true, 1);
		g_signal_connect(is_default_behavior, "clicked", G_CALLBACK(this->default_behavior_clicked), this);

		// default behavior widget
		this->behavior_frame = gtk_frame_new(gettext("Behaviour of this setting"));
		gtk_container_add(GTK_CONTAINER(this->behavior_frame), this->behavior.widget);
		gtk_box_pack_start(GTK_BOX(folders_vbox), this->behavior_frame, true, true, 1);
	}


	// add done button
	gtk_box_pack_start(GTK_BOX(main_vbox), this->behavior.widget, true, true, 1);
	GtkWidget			*validate_button = gtk_button_new_with_label(gettext("Done"));;
	gtk_box_pack_start(GTK_BOX(main_vbox), validate_button, true, true, 1);

	// kill signals
	g_signal_connect(validate_button, "clicked", G_CALLBACK(this->close_window), (gpointer)this);
	g_signal_connect(this->window, "destroy", G_CALLBACK(this->close_window), (gpointer)this);

	// show all
	gtk_widget_show_all(this->window);

	// set behavior frame
	if(prefs.get_expert_mode())
	{
		// set initial state
		if(this->behavior.get_alteration() == _ac_create)
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(is_default_behavior), true);
			gtk_widget_hide(this->behavior_frame);
			this->default_behavior_set = true;
		}
		else
		{
			gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(is_default_behavior), false);
			gtk_widget_show(this->behavior_frame);
			this->default_behavior_set = false;
		}
	}

	// execute window
	gtk_window_resize(GTK_WINDOW(this->window), 1, 1);
	gtk_main();
}

window_card :: ~window_card()
{
	if(this->default_behavior_set)
		this->behavior.set_to_delete();
	if(this->new_group != NULL)
		if(!this->original_config->set_group(this->new_group))
		{
			dialog_info			message(NULL);
			message.asking(gettext("Can't update group name.\nThe group name must be alphanumeric (without space) and must have less then 100 chars."));
		}
	this->original_config->commit();
	this->behavior.commit();
	*this->original_config->behavior = this->behavior;

	// update groups menu
	application :: rebuild_group_menu();
}

void
window_card :: default_behavior_clicked(GtkWidget *checkbox, gpointer THISP)
{
	window_card				*THIS = (window_card*)THISP;

	THIS->default_behavior_set = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(checkbox));
	if(THIS->default_behavior_set)
		gtk_widget_hide(THIS->behavior_frame);
	else
		gtk_widget_show(THIS->behavior_frame);
	gtk_window_resize(GTK_WINDOW(THIS->window), 1, 1);
}

void
window_card :: close_window(GtkWindow *window, gpointer THISP)
{
	window_card				*THIS = (window_card*)THISP;

	THIS->new_group = strdup((char*)gtk_entry_get_text(GTK_ENTRY(THIS->group_entry)));
	gtk_widget_destroy(THIS->window);
	gtk_main_quit();
}
