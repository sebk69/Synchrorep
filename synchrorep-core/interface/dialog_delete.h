/*
 * dialog_delete.h
 *
 *  Created on: 18 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask if delete
 *
 */

#ifndef DELETE_DIALOG_H_
#define DELETE_DIALOG_H_

#include "../applicative/application.h"

typedef enum
{
	_delete_choose_no,
	_delete_choose_yes,
	_delete_choose_cancel
} delete_choose;

class dialog_delete
{
protected:
	GtkWidget			*parent;
	bool				always;
	delete_choose		choosen;

	static void			always_hook(GtkWidget *window_widnget, gpointer dialog_class);

public:
						dialog_delete(GtkWidget *parent);
						~dialog_delete();

	void				no_confirmation(delete_choose choosen);
	delete_choose		asking(char *file);
};

#endif /* DELETE_DIALOG_H_ */
