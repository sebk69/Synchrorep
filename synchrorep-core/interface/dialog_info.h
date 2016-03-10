/*
 * dialog_info.h
 *
 *  Created on: 24 jan. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2010 - Sébastien Kus
 *
 *  User dialog to show info message
 *
 */

#ifndef INFO_DIALOG_H_
#define INFO_DIALOG_H_

#include "../applicative/application.h"

class dialog_info
{
private:
	GtkWidget			*parent;

protected:
	bool				choosen;

public:
						dialog_info(GtkWidget *parent);
						~dialog_info();

	bool				asking(char *infos);
};

#endif /* INFO_DIALOG_H_ */
