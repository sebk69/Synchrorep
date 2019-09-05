/*
 * application.cpp
 *
 *  Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009,2010,2011,2019 - Sébastien Kus
 *
 *  application common class
 *
 */

// TODO : idea : option to just analyze differences

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include "application.h"
#include <strings.h>
#include <stdlib.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include <sys/types.h>
#include <unistd.h>

#include <gio/gio.h>

#include "../technical/tc_file_with_infos.h"
#include "../technical/tc_files_binomial.h"
#include "../technical/tc_find_files.h"
#include "../technical/tc_misc.h"

#include "../accessors/ac_config.h"
#include "../accessors/ac_logs.h"

// config window
#include "../interface/window_config_main.h"
// log window
#include "../interface/window_log.h"
// pending questions
#include "../interface/window_ask_at_the_end.h"
// create new synchronization
#include "new_synchronization.h"
// execution of a synchronization
#include "execute.h"

char			**application :: files;

bool			new_config(ac_config *config);

// main entry of program
int
main(int argc, char **argv)
{
	// application initialization
	application			app(&argc, &argv);

	int					i;
	bool				config_action = false;
	bool				execute_action = false;
	bool				execute_group_action = false;
	bool				list_groups_action = false;
	bool				get_group_action = false;
	bool				log_action = false;
	bool				pending_questions = false;
	char				*folder;
	char				*launch_time;
	int					result = 1;

	for(i=1;i<argc;i++)
	{
		if(strcmp(argv[i], "--config") == 0)
		{
			config_action = true;
		}
		else if(strcmp(argv[i], "--execute") == 0)
		{
			if(i+1 < argc)
			{
				execute_action = true;
				i++;
				folder = argv[i];
			}
		}
		else if(strcmp(argv[i], "--execute-group") == 0)
		{
			if(i+1 < argc)
			{
				execute_group_action = true;
				i++;
				folder = argv[i];
			}
		}
		else if(strcmp(argv[i], "--list-groups") == 0)
		{
			list_groups_action = true;
		}
		else if(strcmp(argv[i], "--get-group") == 0)
		{
			if(i+1 < argc)
			{
				get_group_action = true;
				i++;
				folder = argv[i];
			}
		}
		else if(strcmp(argv[i], "--log") == 0)
			log_action = true;
		else if(strcmp(argv[i], "--pending-questions") == 0)
		{
			if(i+2 < argc)
			{
				pending_questions = true;
				i++;
				folder = argv[i];
				i++;
				launch_time = argv[i];
			}
		}
	}
	// configure
	if(config_action && !execute_action && !execute_group_action && !log_action)
	{
		window_config_main			config;
		result = 0;
	}

	// execute
	if(execute_action && !config_action && !execute_group_action && !log_action)
	{
		ac_config					*config = new ac_config(argv[2]);

		if(config->get_alteration() != _ac_create)
		{
			execute				sync;
			sync.new_folder(argv[2], config->get_mode());
			sync.start();
			result = 0;
		}
		else
		{
			new_synchronization			new_sync(config);
			if(new_sync.get_config_ok())
			{
				execute			sync;
				sync.new_folder(argv[2], config->get_mode());
				sync.start();
				result = 0;
			}
			result = 0;
		}
	}

	// execute group
	if(execute_group_action && !execute_action && !config_action && !log_action)
	{
		ac_config_list					group_list(folder);
		if(group_list.begin())
		{
			execute				sync;
			do
				sync.new_folder(group_list.get_current()->get_from_folder_uri(),
						group_list.get_current()->get_mode());
			while(group_list.next());
			sync.start();
		}
		else
		{
			dialog_info					message(NULL);
			message.asking(gettext("This group don't exists"));
		}
		result = 0;
	}

	// show log window
	if(log_action && !execute_action && !config_action && !execute_group_action)
	{
		window_log				log_window;
		result = 0;
	}

	// list groups
	if(list_groups_action)
	{
		ac_common			init_accessors_db;
		char				**groups = ac_config :: get_groups();
		int					i;

		if(groups[0] == NULL)
		{
			cerr<<gettext("No groups are defined")<<endl;
			result = 2;
		}
		else
		{
			for(i=0; groups[i] != NULL; i++)
				cout<<groups[i]<<endl;
			result = 0;
		}
	}

	// get group of a folder
	if(get_group_action)
	{
		ac_config					*config = new ac_config(argv[2]);
		char						*output;

		output = config->get_group();
		if(output[0] == '\0') // no group
		{
			cerr<<gettext("This folder is not a part of a group")<<endl;
			result = 2;
		}
		else
		{
			cout<<output<<endl;
			result = 0;
		}
	}

	// show pending questions
	if(pending_questions)
	{
		ac_config					config(folder);
		time_t						timed_launch_time = convert_string_to_int(launch_time);
		window_ask_at_the_end		window(config.get_id(), timed_launch_time);
		result = 0;
	}

	if(result == 1)
	{
		cout<<"Synchrorep usage :"<<endl;
		cout<<"--config                                   : open configuration window"<<endl;
		cout<<"--execute [folder]                         : execute synchronization containing [folder]"<<endl;
		cout<<"--execute-group [group name]               : execute all synchronizations of group [group name]"<<endl;
		cout<<"--list-groups                              : show list of all groups name"<<endl;
		cout<<"--get-group [folder]                       : show group of [folder] (if exists)"<<endl;
		cout<<"--pending-questions [folder] [time launch] : show pending questions about [folder] at [launch time] (if exists)"<<endl;
	}

	return result;
}

// application class
// constructor
application :: application(int *argc, char ***argv)
{
	// main initializations ?
	if(argc != NULL)
	{
		gtk_set_locale();
		gtk_init(argc, argv);
		g_file_make_directory(g_file_new_for_path(this->get_work_folder()), NULL, NULL);
		purge_logs();
	}

	setlocale (LC_ALL, "");
	bindtextdomain ("synchrorep", "/usr/share/locale-langpack");
	textdomain ("synchrorep");


	// set files
	this->files = new char*[NUM_FILES+1];
	this->files[LICENCE_FILE] = strdup("/usr/share/doc/synchrorep/licence");
	this->files[COPYRIGHT_FILE] = strdup("/usr/share/doc/synchrorep/copyright");
	this->files[ICON_FILE] = strdup("/usr/share/pixmaps/synchrorep.png");
	this->files[LAUNCHER_FILE] = strdup("/usr/share/synchrorep/synchrorep.desktop");
	this->files[GROUP_MENU] = strdup("/usr/share/synchrorep/synchrorep.group.menu");
	this->files[GROUP_DIRECTORY] = strdup("/usr/share/synchrorep/synchrorep.group.directory");
	this->files[SYNC_ARROW] = strdup("/usr/share/synchrorep/sync_arrow.png");
	this->files[COPY_ARROW] = strdup("/usr/share/synchrorep/copy_arrow.png");
	this->files[NUM_FILES] = NULL;

	// test files presence
	int				i;
	for(i=0; i<NUM_FILES; i++)
	{
		GFile				*cur_file = g_file_new_for_path(this->files[i]);
		if(!g_file_query_exists(cur_file, NULL))
			application :: fatal_error("Some files are missing.\nPlease contact packager or reinstall.");
	}
}

application :: ~application()
{
	application :: exit_app();
}

void
application :: exit_app()
{
	application :: delete_work_folder();
	exit(0);
}

// application datas
char*
application :: whoami()
{
	// get current user
	FILE			*cmd_stdout = popen("whoami", "r");
	char			buffer[255];

	// lecture du stdout
	fgets(buffer, 254, cmd_stdout);
	pclose(cmd_stdout);

	// suppression du retour chariot
	*strchr(buffer, '\n')='\0';

	return strdup(buffer);
}

char*
application :: get_pid()
{
	char			pid[5];
	int				gpid = getpid();
	// get pid to string
	gcvt(gpid, 5, pid);
	return strdup(pid);
}

char*
application :: get_work_folder()
{
	// build working directory path
	const char		*home = g_get_home_dir();
	char			temp_work_folder[strlen(home) + 50];

	temp_work_folder[0] = '\0';
	strcat(temp_work_folder, home);
	strcat(temp_work_folder, "/.synchrorep.work");
	GFile			*work_folder = g_file_new_for_path(temp_work_folder);
	g_file_make_directory(work_folder, NULL, NULL);
	strcat(temp_work_folder, "/.");
	strcat(temp_work_folder, get_pid());
	return strdup(temp_work_folder);
}

void
application :: delete_work_folder()
{
	GFile					*work_folder = g_file_new_for_path(application :: get_work_folder());
	if(g_file_query_exists(work_folder, NULL))
	{
		tc_find_files			work_folder_find(work_folder);
		work_folder_find.process();
		work_folder_find.ftp_delete(NULL);
	}
}

// get application file name
char*
application :: get_file_path(int _file)
{
	return strdup(application :: files[_file]);
}

// about box
void
application :: about()
{
	GtkWidget			*about_box = gtk_about_dialog_new();

	// program infos
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_box), "Synchrorep");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_box), "1.5.5");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(about_box), "https://small-iceberg.dev");

	// logo
	GdkPixbuf			*icon = gdk_pixbuf_new_from_file(application :: files[ICON_FILE], NULL);
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(about_box), icon);
	gtk_window_set_icon_from_file(GTK_WINDOW(about_box), application :: get_file_path(ICON_FILE), NULL);

	// credits
	char				translators[1000] = "";
	strcat(translators, "lusmanko\n");
	strcat(translators, "Andrey Smolenkov\n");
	strcat(translators, "DiegoJ\n");
	strcat(translators, "Lunix\n");
	strcat(translators, "Artur Rona\n");
	strcat(translators, "Federico Vera\n");
	strcat(translators, "Borja López Soilán\n");
	strcat(translators, "Aljosha Papsch\n");
	strcat(translators, "Maxim Strukov\n");
	strcat(translators, "Luis Alejandro Rangel Sánchez\n");
	strcat(translators, "Yvan Ysh\n");
	strcat(translators, "Jan-Christoph Borchardt\n");
	strcat(translators, "Jarosław Ogrodnik\n");
	strcat(translators, "0guma\n");
	strcat(translators, "jedelwey\n");
	strcat(translators, "Aiguanachein\n");
	strcat(translators, "Paulomorales\n");
	strcat(translators, "Vyacheslav Sharmanov\n");
	strcat(translators, "Hector A. Mantellini\n");
	strcat(translators, "Dennis Baudys\n");
	strcat(translators, "Jan\n");
	strcat(translators, "Jaroslaw\n");
	strcat(translators, "Ogrodnik\n");
	strcat(translators, "Julian Gehring\n");
	strcat(translators, "Lunix\n");
	strcat(translators, "Gamgster\n");
	gtk_about_dialog_set_translator_credits(GTK_ABOUT_DIALOG(about_box), translators);
	char				author[100] = "Sébastien Kus <seb@small-iceberg.dev>";
	char				**authors = new char*[2];
	authors[0] = author;
	authors[1] = NULL;
	gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_box), (const char**)authors);

	// licence datas
	GFile					*licence_file = g_file_new_for_path(application :: files[LICENCE_FILE]);
	GFileInfo				*licence_infos = g_file_query_info(licence_file,
								G_FILE_ATTRIBUTE_STANDARD_SIZE,
								G_FILE_QUERY_INFO_NONE,
								NULL, NULL);
	size_t					licence_size = g_file_info_get_size(licence_infos);
	char					*licence_text = new char[licence_size + 1];
	if(g_file_load_contents(licence_file,
						 NULL,
						 (char**)&licence_text,
						 NULL,
						 NULL,
						 NULL))
	gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_box), licence_text);

	// copyright datas
	GFile					*copyright_file = g_file_new_for_path(application :: files[COPYRIGHT_FILE]);
	GFileInfo				*copyright_infos = g_file_query_info(copyright_file,
								G_FILE_ATTRIBUTE_STANDARD_SIZE,
								G_FILE_QUERY_INFO_NONE,
								NULL, NULL);
	size_t					copyright_size = g_file_info_get_size(copyright_infos);
	char					*copyright_text = new char[copyright_size + 1];
	if(g_file_load_contents(copyright_file,
						 NULL,
						 (char**)&copyright_text,
						 NULL,
						 NULL,
						 NULL))
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_box), copyright_text);

	gtk_dialog_run(GTK_DIALOG(about_box));
	gtk_widget_hide(about_box);
	gtk_main_iteration();
	gtk_main_iteration();
}

void
application :: fatal_error(const char *message)
{
	cout<<message<<endl;
	exit(1);
}

void
application :: rebuild_group_menu()
{
	// create menu
	GFile				*menu_source = g_file_new_for_path(application :: get_file_path(GROUP_MENU));
	char				*home_path = (char*)g_get_home_dir();
	char				*menu_dest_path = new char[strlen(home_path) + 200];
	strcpy(menu_dest_path, home_path);
	strcat(menu_dest_path, "/.config/menus/applications-merged");
	GFile				*dir_menu = g_file_new_for_path(menu_dest_path);
	g_file_make_directory_with_parents(dir_menu, NULL, NULL);
	strcat(menu_dest_path, "/synchrorep.group.menu");
	GFile				*menu_dest = g_file_new_for_path(menu_dest_path);
	g_file_copy(menu_source, menu_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);
	delete menu_dest_path;

	// create directory file
	GFile				*directory_source = g_file_new_for_path(application :: get_file_path(GROUP_DIRECTORY));
	char				*directory_dest_path = new char[strlen(home_path) + 200];
	strcpy(directory_dest_path, home_path);
	strcat(directory_dest_path, "/.local/share/desktop-directories");
	GFile				*dir_directory = g_file_new_for_path(directory_dest_path);
	g_file_make_directory_with_parents(dir_directory, NULL, NULL);
	strcat(directory_dest_path, "/synchrorep.group.directory");
	GFile				*directory_dest = g_file_new_for_path(directory_dest_path);
	g_file_copy(directory_source, directory_dest, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, NULL);

	// delete old groups desktop files
	char				desktop_dir_name[10000];
	char				*desktop_file_name;
	char				*temp_name;
	char				desktop_path[10000];
	GFile				*dir_desktop;
	GFile				*temp_gfile;
	GFileInfo			*info;

	strcpy(desktop_dir_name, home_path);
	strcat(desktop_dir_name, "/.local/share/applications");

	dir_desktop = g_file_new_for_path(desktop_dir_name);
	g_file_make_directory_with_parents(dir_desktop, NULL, NULL);
	GFileEnumerator			*list_of_childs;
	list_of_childs = g_file_enumerate_children(dir_desktop, G_FILE_ATTRIBUTE_STANDARD_NAME, G_FILE_QUERY_INFO_NONE, NULL, NULL);
	if(list_of_childs != NULL)
	{
		info = g_file_enumerator_next_file(list_of_childs, NULL, NULL);
		while(info != NULL)
		{
			desktop_file_name = strdup(g_file_info_get_name(info));
			temp_name = strdup(desktop_file_name);
			if(strlen(temp_name) > 17)
			{
				temp_name[17] = '\0';
				if(strcmp(temp_name, "synchrorep.group.") == 0)
				{
					strcpy(desktop_path, desktop_dir_name);
					strcat(desktop_path, "/");
					strcat(desktop_path, desktop_file_name);
					temp_gfile = g_file_new_for_path(desktop_path);
					g_file_delete(temp_gfile, NULL, NULL);
				}
			}
			info = g_file_enumerator_next_file(list_of_childs, NULL, NULL);
		}
	}

	// create a desktop file for each group
	char				**groups = ac_config :: get_groups();
	int					i = 0;
	while(groups[i] != NULL)
	{
		if(strcmp(groups[i], "") != 0)
		{
			char				*desktop_file_name = new char[strlen(home_path) + 350];
			strcpy(desktop_file_name, home_path);
			strcat(desktop_file_name, "/.local/share/applications");
			strcat(desktop_file_name, "/synchrorep.group.");
			strcat(desktop_file_name, groups[i]);
			strcat(desktop_file_name, ".desktop");
			ofstream			out_file_stream(desktop_file_name);
			out_file_stream<<"[Desktop Entry]"<<endl;
			out_file_stream<<"Encoding=UTF-8"<<endl;
			out_file_stream<<"Version=1.0"<<endl;
			out_file_stream<<"Type=Application"<<endl;
			out_file_stream<<"Name="<<gettext("Synchronize group ")<<groups[i]<<"..."<<endl;
			out_file_stream<<"Icon=synchrorep.group"<<endl;
			out_file_stream<<"Exec=synchrorep --execute-group "<<groups[i]<<endl;
			out_file_stream<<"Categories=synchrorep"<<endl;
			out_file_stream.close();
		}
		// next group
		i++;
	}
}

// for resources files translation
void			only_for_translation();
void
only_for_translation()
{
	gettext("Drag folder to synchronize");
	gettext("Configure synchrorep");
	gettext("Execute a synchronization");
	gettext("Synchronize two folders");
	gettext("Synchronize a group");
}
