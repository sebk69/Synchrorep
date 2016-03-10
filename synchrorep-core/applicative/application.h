/*
 * application.h
 *
 *  Created on: 7 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2010 - Sébastien Kus
 *
 *  application common class
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <stdlib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <libintl.h>

using namespace std;

// list of files
#define			LICENCE_FILE			0
#define			COPYRIGHT_FILE			LICENCE_FILE + 1
#define			ICON_FILE				COPYRIGHT_FILE + 1
#define			LAUNCHER_FILE			ICON_FILE + 1
#define			GROUP_MENU				LAUNCHER_FILE + 1
#define			GROUP_DIRECTORY			GROUP_MENU + 1
#define			SYNC_ARROW				GROUP_DIRECTORY + 1
#define			COPY_ARROW				SYNC_ARROW + 1

// number of files
#define			NUM_FILES				8


// class application
class application
{
private:
	static char			**files;

	static void			delete_work_folder();

public:
						application(int *argc = NULL, char ***argv = NULL);
						~application();

	// application datas
	static char*		whoami();
	static char*		get_pid();
	static char*		get_work_folder();
	static char*		get_file_path(int _file);

	// dialogs
	static void			about();
	static void			fatal_error(const char *message);

	// build groups menu
	static void			rebuild_group_menu();

	// exit program
	static void			exit_app();
};

#endif /* APPLICATION_H_ */
