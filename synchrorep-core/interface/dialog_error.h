/*
 * dialog_delete.h
 *
 *  Created on: 18 déc. 2009
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  User dialog to ask what to to if error occured
 *
 */

#ifndef ERROR_DIALOG_H_
#define ERROR_DIALOG_H_

#include "../applicative/application.h"

#include "../technical/tc_misc.h"

typedef enum
{
	_error_choose_ignore,
	_error_choose_cancel
} error_choose;

class dialog_error
{
private:
	GtkWidget			*parent;

protected:
	bool				always;
	error_choose		choosen;

	static void			always_hook(GtkWidget *window_widnget, gpointer dialog_class);

public:
						dialog_error(GtkWidget *parent);
						~dialog_error();

	void				no_confirmation(error_choose choosen);
	error_choose		asking(error_infos infos);
};

#endif /* ERROR_DIALOG_H_ */
