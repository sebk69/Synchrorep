/*
 * window_config_main.cpp
 *
 *  Created on: 4 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010,2011 - Sébastien Kus
 *
 *  Main window of configuration :
 *  - let choose synchronization to configure
 *  - let choose configuring default behavior
 *  - let choose mode of use of behavior
 *
 */

#include "window_config_main.h"

#include "window_default_behavior.h"
#include "window_card.h"
#include "window_log.h"

#include "../technical/tc_misc.h"

#include <iostream>

#include <string.h>

window_config_main :: window_config_main()
{
	// define window
	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	char				title[255] = "Synchrorep - ";
	strcat(title, gettext("Settings"));
	gtk_window_set_title (GTK_WINDOW(this->window), title);
	gtk_window_set_icon_from_file(GTK_WINDOW(this->window), application :: get_file_path(ICON_FILE), NULL);

	// add vbox
	GtkWidget			*main_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(this->window), main_vbox);

	// add preferences frame
	GtkWidget*          preferences_frame = gtk_frame_new(gettext("Preferences"));
	gtk_box_pack_start(GTK_BOX(main_vbox), preferences_frame, false, false, 1);
	GtkWidget			*preferences_vbox = gtk_vbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(preferences_frame), preferences_vbox);
	GtkWidget			*preferences_top_hbox = gtk_hbox_new(false, 1);
	gtk_box_pack_start(GTK_BOX(preferences_vbox), preferences_top_hbox, false, false, 1);
	GtkWidget			*preferences_bottom_hbox = gtk_hbox_new(false, 1);
	gtk_box_pack_start(GTK_BOX(preferences_vbox), preferences_bottom_hbox, false, false, 1);

	// add preferences buttons
	this->activate_contextual = gtk_check_button_new_with_label(gettext("Activate contextual menu"));
	g_signal_connect(this->activate_contextual, "toggled", G_CALLBACK(this->contextual_toggled), this);
	gtk_box_pack_start(GTK_BOX(preferences_top_hbox), this->activate_contextual, false, false, 10);

	this->activate_expert = gtk_check_button_new_with_label(gettext("I am an expert user"));
	g_signal_connect(this->activate_expert, "toggled", G_CALLBACK(this->expert_toggled), this);
	gtk_box_pack_start(GTK_BOX(preferences_bottom_hbox), this->activate_expert, false, false, 10);

	GtkWidget			*launcher_button = gtk_button_new_with_label(gettext("Create a launcher icon"));
	g_signal_connect(launcher_button, "clicked", G_CALLBACK(this->create_launcher), NULL);
	gtk_box_pack_start(GTK_BOX(preferences_top_hbox), launcher_button, true, true, 1);

	this->behavior_button = gtk_button_new_with_label(gettext("Default behavior"));
	g_signal_connect(this->behavior_button, "clicked", G_CALLBACK(this->open_default_behavior), NULL);
	gtk_box_pack_start(GTK_BOX(preferences_bottom_hbox), this->behavior_button, true, true, 1);


	// list of configurations
	// insert data
	GtkTreeIter			current;
	GtkListStore		*datas = gtk_list_store_new(3, G_TYPE_STRING, GDK_TYPE_PIXBUF, G_TYPE_STRING, NULL);
	GdkPixbuf			*sync_arrow = gdk_pixbuf_new_from_file(application :: get_file_path(SYNC_ARROW), NULL);
	GdkPixbuf			*copy_arrow = gdk_pixbuf_new_from_file(application :: get_file_path(COPY_ARROW), NULL);
	if(this->list_of_configs.begin())
	{
		do
		{
			gtk_list_store_append(datas, &current);
			GdkPixbuf			*arrow;
			if(this->list_of_configs.get_current()->get_mode() == _sync)
				arrow = sync_arrow;
			else
				arrow = copy_arrow;
			gtk_list_store_set(datas, &current,
					0, cut_uri(g_uri_unescape_string(this->list_of_configs.get_current()->get_from_folder_uri(), NULL), 40),
					1, arrow,
                    2, cut_uri(g_uri_unescape_string(this->list_of_configs.get_current()->get_to_folder_uri(), NULL), 40),
					-1);
		}
		while(this->list_of_configs.next());

		// create configs list and actions
		GtkWidget			*config_hbox = gtk_hbox_new(false, 1);
		gtk_box_pack_start(GTK_BOX(main_vbox), config_hbox, true, true, 1);

		// create list view
		this->list_view = gtk_tree_view_new();
		// append first column
		GtkCellRenderer		*renderer = gtk_cell_renderer_text_new();
		GtkTreeViewColumn	*column1 = gtk_tree_view_column_new();
		gtk_tree_view_column_pack_start(column1, renderer, TRUE);
		gtk_tree_view_column_add_attribute(column1, renderer, "text", 0);
		gtk_tree_view_append_column(GTK_TREE_VIEW(this->list_view), column1);
		// append arrow column
		GtkCellRenderer		*renderer_pixbuf = gtk_cell_renderer_pixbuf_new();
		GtkTreeViewColumn	*column_arrow = gtk_tree_view_column_new();
		gtk_tree_view_column_pack_start(column_arrow, renderer_pixbuf, TRUE);
		gtk_tree_view_column_add_attribute(column_arrow, renderer_pixbuf, "pixbuf", 1);
		gtk_tree_view_append_column(GTK_TREE_VIEW(this->list_view), column_arrow);
		// append second column
		GtkCellRenderer		*renderer2 = gtk_cell_renderer_text_new();
		GtkTreeViewColumn	*column2 = gtk_tree_view_column_new();
		gtk_tree_view_column_pack_start(column2, renderer2, TRUE);
		gtk_tree_view_column_add_attribute(column2, renderer2, "text", 2);
		gtk_tree_view_append_column(GTK_TREE_VIEW(this->list_view), column2);
		// setup properties
		gtk_tree_view_set_model(GTK_TREE_VIEW(this->list_view), GTK_TREE_MODEL(datas));
		gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(this->list_view), false);
		// create scroll buttons
		GtkWidget			*scrolled_window = gtk_scrolled_window_new(NULL, NULL);
		gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled_window), this->list_view);
		gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_window), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
		// add to vbox
		gtk_box_pack_start(GTK_BOX(config_hbox), scrolled_window, true, true, 1);

		// create actions for configs
		GtkWidget*          actions_frame = gtk_frame_new(gettext("Actions"));
		gtk_box_pack_start(GTK_BOX(config_hbox), actions_frame, false, false, 1);
		GtkWidget			*action_frame_vbox = gtk_vbox_new(false, 1);
		gtk_container_add(GTK_CONTAINER(actions_frame), action_frame_vbox);
		// create modify button
		GtkWidget			*modify_button = gtk_button_new_with_label(gettext("Modify"));
		g_signal_connect(modify_button, "clicked", G_CALLBACK(this->modify_clicked), this);
		gtk_box_pack_start(GTK_BOX(action_frame_vbox), modify_button, false, false, 1);
		// create delete button
		GtkWidget			*delete_button = gtk_button_new_with_label(gettext("Delete"));
		g_signal_connect(delete_button, "clicked", G_CALLBACK(this->delete_clicked), this);
		gtk_box_pack_start(GTK_BOX(action_frame_vbox), delete_button, false, false, 1);
		// create logs button
		GtkWidget			*logs_button = gtk_button_new_with_label(gettext("View logs"));
		g_signal_connect(logs_button, "clicked", G_CALLBACK(this->logs_clicked), this);
		gtk_box_pack_start(GTK_BOX(action_frame_vbox), logs_button, false, false, 1);
	}
	else
	{
		// no configuration, print help message
		GtkWidget			*label = gtk_label_new(gettext("No synchronisations are set.\nPlease choose a synchronisation mode in preferences.\n- \"Activate contextual menu\" allow you to set a synchronisation\n   in a right click on the folder to synchronize\n- \"Create a desktop icon\" will place an icon on the desktop. You\n   will only have to drag a folder on it to start a synchronization."));
		gtk_box_pack_start(GTK_BOX(main_vbox), label, false, false, 1);
	}

	// add about button
	GtkWidget			*about_button = gtk_button_new_with_label(gettext("About"));
	g_signal_connect(about_button, "clicked", G_CALLBACK(this->about_clicked), NULL);
	gtk_box_pack_start(GTK_BOX(main_vbox), about_button, false, false, 1);

	// connect close window
	g_signal_connect(this->window, "destroy", G_CALLBACK(this->close_window), NULL);

	// refresh content
	gtk_widget_show_all(this->window);
	this->refresh();

	// and execute window
	gtk_main();
}

window_config_main :: ~window_config_main()
{
}

void
window_config_main :: about_clicked(GtkButton *btn, gpointer null_data)
{
	application :: about();
}

void
window_config_main :: delete_clicked(GtkButton *btn, gpointer THISP)
{
	window_config_main			*THIS = (window_config_main*)THISP;
	if(THIS->list_of_configs.goto_position(THIS->get_selected()))
	{
		THIS->list_of_configs.get_current()->set_to_delete();
		THIS->list_of_configs.get_current()->commit();
		THIS->list_of_configs.remove();
		THIS->remove_selected();
		application :: rebuild_group_menu();
	}
}

void
window_config_main :: logs_clicked(GtkButton *btn, gpointer THISP)
{
	window_log			view_logs;
}

void
window_config_main :: modify_clicked(GtkButton *btn, gpointer THISP)
{
	window_config_main			*THIS = (window_config_main*)THISP;
	if(THIS->list_of_configs.goto_position(THIS->get_selected()))
		window_card					card(THIS->list_of_configs.get_current());
}

void
window_config_main :: expert_toggled(GtkCheckButton *btn, gpointer THISP)
{
	window_config_main			*THIS = (window_config_main*)THISP;

	THIS->prefs.set_expert_mode(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)));
	THIS->refresh();
}

void
window_config_main :: contextual_toggled(GtkCheckButton *btn, gpointer THISP)
{
	window_config_main			*THIS = (window_config_main*)THISP;

	THIS->prefs.set_contextual_menu(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn)));
	THIS->refresh();
}

void
window_config_main :: create_launcher(GtkButton *btn, gpointer null_data)
{
	GFile			*from = g_file_new_for_path(application :: get_file_path(LAUNCHER_FILE));
	char			*desktop = strdup(g_get_user_special_dir(G_USER_DIRECTORY_DESKTOP));
	char			*to_string = new char[strlen(desktop) + 50];
	strcpy(to_string, desktop);
	strcat(to_string, "/synchrorep.desktop");
	GFile			*to = g_file_new_for_path(to_string);
	g_file_copy(from, to, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
	free(desktop);
	delete to_string;
}

void
window_config_main :: open_default_behavior(GtkButton *btn, gpointer null_data)
{
	window_default_behavior		define_behaviour;
}

void
window_config_main :: close_window(GtkWindow *window, gpointer null_data)
{
	gtk_widget_destroy(GTK_WIDGET(window));
	gtk_main_quit();
}

void
window_config_main :: refresh()
{
	// set expert mode
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->activate_expert), this->prefs.get_expert_mode());

	// show behavior button only if expert
	gtk_widget_set_visible(this->behavior_button, this->prefs.get_expert_mode());

	// setup contextual menu checkbox
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(this->activate_contextual), this->prefs.get_contextual_menu());
}

int
window_config_main :: get_selected()
{
	GtkTreePath			*path;
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(this->list_view), &path, NULL);
	if(path == NULL)
		return -1;
	int					*result;
	result = gtk_tree_path_get_indices(path);

	return result[0];
}

void
window_config_main :: remove_selected()
{
	GtkTreePath			*path;
	gtk_tree_view_get_cursor(GTK_TREE_VIEW(this->list_view), &path, NULL);
	if(path == NULL)
		return;
	GtkTreeIter			iter;
	gtk_tree_model_get_iter(gtk_tree_view_get_model(GTK_TREE_VIEW(this->list_view)), &iter, path);
	gtk_list_store_remove(GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(this->list_view))), &iter);
}
