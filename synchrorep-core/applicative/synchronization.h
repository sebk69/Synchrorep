/*
 * synchronization.h
 *
 *  Created on: 3 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009-2010 - Sébastien Kus
 *
 *  Synchronization process
 *
 */

#ifndef SYNCHRONIZATION_H_
#define SYNCHRONIZATION_H_

#include "application.h"
#include "logging.h"
#include "../interface/dialog_info.h"
#include "../interface/dialog_delete.h"
#include "../interface/dialog_file_conflict.h"
#include "../interface/dialog_error.h"
#include "../technical/tc_thread_utils.h"

// exchange class for synchronizing process treads

// steps of synchronization
typedef enum
{
	_pending,
	_analyse_step,
	_synchronize_step,
	_terminated,
	_window_closed,
	_cancel
} _sync_steps;

// define ask dialogs classes
#define			MUTEX_DIALOG_DELETE			dialog_mutex<dialog_delete, delete_choose, char*>
#define			MUTEX_DIALOG_FILE_CONFLICT	dialog_mutex<dialog_file_conflict, conflict_choose, tc_files_binomial*>
#define			MUTEX_DIALOG_ERROR			dialog_mutex<dialog_error, error_choose, error_infos>
#define			MUTEX_DIALOG_INFO			dialog_mutex<dialog_info, bool, char*>

class synchronize_exchanges
{
private:
	GMutex						*ask_deletion_mutex;
	GMutex						*ask_file_conflict_mutex;
	GCond						*answer_cond;

public:
	// variables
	mutex_var<int>				num_files;
	mutex_var<int>				num_files_scanned;
	mutex_var<char*>			*message;
	mutex_var<_sync_steps>		sync_step;
	mutex_var<bool>				cancel;
	mutex_var<char*>			command_line_parameter;
	MUTEX_DIALOG_DELETE			*asking_deletion;
	MUTEX_DIALOG_FILE_CONFLICT	*asking_file_conflict;
	MUTEX_DIALOG_ERROR			*asking_error;
	MUTEX_DIALOG_INFO			*asking_info;

	GMutex						*synchronizing;
	GtkWidget					*progress;
	GtkWidget					*progress_name;

								synchronize_exchanges();
								~synchronize_exchanges();
	void						init(char *command_line_parameter);
};

// public synchronization process

class synchronization
{
private:
	GThread 						**syncs,
									*progress;
	synchronize_exchanges			**exchanges;
	static GMutex					*ask_mutex;  // only one dialog at a time

	static gpointer		synchronize(gpointer exchanges);
	static gpointer		synchronize_window(gpointer exchanges);
	static gboolean		synchronize_window_hook(gpointer exchanges);
	static void			synchronize_cancel(GtkWidget *window_widnget, gpointer exchanges);
	static void			synchronize_window_close(GtkWidget *window_widnget, gpointer exchanges);

public:
						synchronization();
	void				new_folder(char *command_line_parameter);
	void				start();
						~synchronization();
};

#endif /* SYNCHRONIZATION_H_ */
