/*
 * new_synchronization.cpp
 *
 *  Created on: 13 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010,2011 - Sébastien Kus
 *
 *  configuration creator
 *
 */

#include <sstream>
#include <string.h>

#include "new_synchronization.h"

#include "application.h"
#include "../interface/dialog_choose_mode.h"
#include "../interface/window_card.h"
#include "../technical/tc_misc.h"
#include "../technical/tc_mount.h"

new_synchronization :: new_synchronization(ac_config *config)
{
	ac_config_list				all_configs;

	this->config_created_correctly = false;

	// mount if not mounted
	tc_mount_from_file		mounting(config->get_from_folder());
	if(!mounting.is_mounted())
		if(!mounting.mount())
		{
			this->cant_mount(g_uri_unescape_string(config->get_from_folder_uri(), NULL));
			return;
		}

	// test if parameter is really a folder
	if(g_file_query_file_type(config->get_from_folder(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		this->not_a_folder_dialog(g_uri_unescape_string(config->get_from_folder_uri(), NULL));
		return;
	}

	// test if from_folder is configured
	if(all_configs.folder_configured(config->get_from_folder_uri()))
	{
		this->exists_dialog(g_uri_unescape_string(config->get_from_folder_uri(), NULL));
		return;
	}

	// ask mode
	dialog_choose_mode		dialog_mode;
	mode_choose				result;

	result = dialog_mode.asking();
	if(result == _mode_choose_cancel)
		return;
	else if(result == _mode_choose_sync)
		config->set_mode(_sync);
	else
		config->set_mode(_copy);

	// ask for companion folder
	ostringstream			title;
	if(config->get_mode() == _sync)
	{
		title<<gettext("Please choose the folder to synchronize with")<<" ";
		title<<cut_uri(g_uri_unescape_string(config->get_from_folder_uri(), NULL), 80);
	}
	else
		title<<gettext("Please choose the folder to copy into");
	GtkWidget				*dialog = gtk_file_chooser_dialog_new(title.str().c_str(), NULL,
							  GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER,
							  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							  NULL);
	gtk_file_chooser_set_local_only(GTK_FILE_CHOOSER(dialog), false);
	if(gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char				*new_folder = gtk_file_chooser_get_uri (GTK_FILE_CHOOSER (dialog));
		gtk_widget_destroy(dialog);
		// test if folder is already configured
		if(!all_configs.folder_configured(new_folder))
		{
			// and add folder to new configuration
			if(config->set_to_folder(new_folder))
			{
				// test if folders are not the same
				if(!g_file_equal(config->get_from_folder(), config->get_to_folder()))
				{
					if(config->commit())
					{
						this->config_created_correctly = true;
						window_card			card(config);
					}
				}
				else
				{
					this->exists_dialog(new_folder);
				}
			}
		}
		else
		{
			this->exists_dialog(new_folder);
		}
	}
	else
		gtk_widget_destroy(dialog);
}

new_synchronization :: ~new_synchronization()
{

}

void
new_synchronization :: exists_dialog(char *folder)
{
	ostringstream		title;
	title<<"Synchrorep - "<<gettext("Error");
	GtkWidget			*dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
								gettext("The choosed folder has already been registered"), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(dialog), application :: get_file_path(ICON_FILE), NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void
new_synchronization :: not_a_folder_dialog(char *folder)
{
	ostringstream		title;
	ostringstream		text;
	title<<"Synchrorep - "<<gettext("Error");
	text<<folder<<gettext(" is not a folder.");
	GtkWidget			*dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
									text.str().c_str(), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(dialog), application :: get_file_path(ICON_FILE), NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

void
new_synchronization :: cant_mount(char *folder)
{
	ostringstream		title;
	ostringstream		text;
	title<<"Synchrorep - "<<gettext("Error");
	text<<gettext("Impossible to mount disk/share of ")<<folder;
	GtkWidget			*dialog = gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_CANCEL,
									text.str().c_str(), NULL);
	gtk_window_set_title(GTK_WINDOW(dialog), title.str().c_str());
	gtk_window_set_icon_from_file(GTK_WINDOW(dialog), application :: get_file_path(ICON_FILE), NULL);
	gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);
}

bool
new_synchronization :: get_config_ok()
{
	return this->config_created_correctly;
}
