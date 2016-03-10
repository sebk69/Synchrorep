/*
 * window_log.cpp
 *
 *  Created on: 18 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *      log management window
 *
 */


#include "window_log.h"

#include "dialog_info.h"

#include "../accessors/ac_config.h"
#include "../accessors/ac_logs.h"

#include "../technical/tc_misc.h"

#include "string.h"
#include <iostream>
#include <sstream>

window_log :: window_log()
{
	// define window
	this->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	char				title[255] = "Synchrorep - ";
	strcat(title, gettext("Logs"));
	gtk_window_set_title (GTK_WINDOW(this->window), title);
	gtk_window_set_icon_from_file(GTK_WINDOW(this->window), application :: get_file_path(ICON_FILE), NULL);

	// add vbox
	this->main_hbox = gtk_hbox_new(false, 1);
	gtk_container_add(GTK_CONTAINER(this->window), this->main_hbox);

	// build navigation tree
	GtkTreeStore 		*store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_POINTER);
	GtkTreeIter			configs_iter;
	GtkTreeIter			launchs_iter;

	// scan configs
	ac_config_list			configs;
	bool					logs_exists = false;
	if(configs.begin())
		do
		{
			ac_log_list				launch_list(configs.get_current());
			list<time_t>			*configs_launchs_times = launch_list.get_time_launch_list();
			if(configs_launchs_times != NULL)
			{
				logs_exists = true;
				// create config iter
				ostringstream			config_string;
				config_string<<"1 : "<<cut_uri(g_uri_unescape_string(configs.get_current()->get_from_folder_uri(), NULL), 80)<<"\n"<<
						"2 : "<<cut_uri(g_uri_unescape_string(configs.get_current()->get_to_folder_uri(), NULL), 80);
				gtk_tree_store_append(store, &configs_iter, NULL);
				gtk_tree_store_set(store, &configs_iter, 0, config_string.str().c_str(), 1, NULL, -1);

				// create launches iters
				list<time_t> :: iterator	cur_time = configs_launchs_times->begin();
				while(cur_time != configs_launchs_times->end())
				{
					gtk_tree_store_append(store, &launchs_iter, &configs_iter);
					char				time_string[100];
					strftime(time_string, 256, "%c", localtime(&*cur_time));
					log_key				*key = new log_key;
					key->config_key = configs.get_current()->get_id();
					key->launch_time = *cur_time;
					gtk_tree_store_set (store, &launchs_iter, 0, time_string, 1, key, -1);

					cur_time++;
				}
			}
		} while(configs.next());

	if(!logs_exists)
	{
		dialog_info				no_logs_info(NULL);
		no_logs_info.asking(gettext("There is no log to view for the moment"));
		return;
	}
	// build visual of tree
	this->launches_tree = gtk_tree_view_new();
	// append column
	GtkCellRenderer		*renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn	*column1 = gtk_tree_view_column_new();
	gtk_tree_view_column_pack_start(column1, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column1, renderer, "text", 0);
	gtk_tree_view_append_column(GTK_TREE_VIEW(this->launches_tree), column1);
	// setup properties
	gtk_tree_view_set_model(GTK_TREE_VIEW(this->launches_tree), GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(this->launches_tree), false);
	// create scroll buttons
	this->scrolled_tree = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(this->scrolled_tree), this->launches_tree);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(this->scrolled_tree), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);

	GtkWidget			*tree_vbox = gtk_vbox_new(false, 1);
	GtkWidget			*dur_hbox = gtk_hbox_new(false, 1);
	GtkWidget			*dur_label1 = gtk_label_new(gettext("Keep logs :"));
	GtkWidget			*dur_label2 = gtk_label_new(gettext("Days"));
	this->log_duration_btn = gtk_spin_button_new_with_range(1, 999, 1);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(this->log_duration_btn), this->get_log_duration());
	gtk_box_pack_start(GTK_BOX(dur_hbox), dur_label1, false, false, 1);
	gtk_box_pack_start(GTK_BOX(dur_hbox), this->log_duration_btn, false, false, 1);
	gtk_box_pack_start(GTK_BOX(dur_hbox), dur_label2, false, false, 1);
	gtk_box_pack_start(GTK_BOX(tree_vbox), this->scrolled_tree, true, true, 1);
	gtk_box_pack_start(GTK_BOX(tree_vbox), dur_hbox, false, false, 1);
	// add to hbox
	gtk_box_pack_start(GTK_BOX(this->main_hbox), tree_vbox, false, false, 1);
	// for the moment no log
	this->log_widget = NULL;
	this->questions_widget = NULL;
	this->notebook = NULL;

	// connect when log is selected
	g_signal_connect(G_OBJECT(gtk_tree_view_get_selection(GTK_TREE_VIEW(this->launches_tree))), "changed", G_CALLBACK(window_log :: select_log), (gpointer)this);
	g_signal_connect(G_OBJECT(this->log_duration_btn), "changed", G_CALLBACK(window_log :: change_log_duration), (gpointer)this);
	g_signal_connect(this->window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

	gtk_widget_show_all(this->window);
	gtk_window_maximize(GTK_WINDOW(this->window));
	gtk_main();
}


window_log :: ~window_log()
{
}

void
window_log :: select_log(GtkTreeSelection *selection, gpointer data)
{
	window_log			*THIS = (window_log*)data;

	if(gtk_tree_selection_get_tree_view(selection) != GTK_TREE_VIEW(THIS->launches_tree))
		return;

	GtkTreeIter 		iter;
	GtkTreeModel		*model;
	log_key				*sel_key;

	if(THIS->log_widget != NULL)
	{
		delete THIS->log_widget;
		THIS->log_widget = NULL;
	}

	if(THIS->questions_widget != NULL)
	{
		delete THIS->questions_widget;
		THIS->questions_widget = NULL;
	}
	if(THIS->notebook != NULL)
		gtk_widget_destroy(THIS->notebook);

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		gtk_tree_model_get(model, &iter, 1, &sel_key, -1);
		if(sel_key != NULL)
		{
			THIS->log_widget = new widget_synchronization_log(sel_key->config_key, sel_key->launch_time);
			THIS->questions_widget = new widget_ask_at_the_end(sel_key->config_key, sel_key->launch_time, THIS->log_widget);
			THIS->notebook = gtk_notebook_new();
			if(THIS->log_widget->widget != NULL)
				gtk_notebook_append_page(GTK_NOTEBOOK(THIS->notebook), THIS->log_widget->widget, gtk_label_new(gettext("Logs")));
			if(THIS->questions_widget->widget != NULL)
				gtk_notebook_append_page(GTK_NOTEBOOK(THIS->notebook), THIS->questions_widget->widget, gtk_label_new(gettext("Pending questions")));
			gtk_box_pack_start(GTK_BOX(THIS->main_hbox), THIS->notebook, true, true, 1);
			gtk_widget_show_all(THIS->notebook);
		}
	}
}
void
window_log :: change_log_duration(GtkWidget *widget, gpointer data)
{
	window_log			*THIS = (window_log*)data;

	THIS->set_log_duration(gtk_spin_button_get_value(GTK_SPIN_BUTTON(THIS->log_duration_btn)));
}
