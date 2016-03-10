/*
 * dialog_file_conflict.h
 *
 *  Created on: 22 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask what to do if two files are modified
 *
 */

#ifndef DIALOG_FILE_CONFLICT_H_
#define DIALOG_FILE_CONFLICT_H_

#include "../applicative/application.h"
#include "../technical/tc_files_binomial.h"

typedef enum
{
	_conflict_choose_recent,
	_conflict_choose_older,
	_conflict_choose_source,
	_conflict_choose_target,
	_conflict_choose_ignore,
	_conflict_choose_cancel
} conflict_choose;

class dialog_file_conflict
{
private:
	int						responses[6];
	GtkWidget				*parent;

	static void				response_hook(GtkWidget *widnget, gpointer response);
	static void				always_hook(GtkWidget *widnget, gpointer dialog);

protected:
	conflict_choose			choosen;
	bool					always;

public:
							dialog_file_conflict(GtkWidget *parent);
							~dialog_file_conflict();

	void					no_confirmation(conflict_choose default_answer);
	conflict_choose			asking(tc_files_binomial *file);
};

#endif /* DIALOG_FILE_CONFLICT_H_ */
