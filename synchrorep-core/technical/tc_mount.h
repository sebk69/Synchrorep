/*
 * tc_mount.h
 *
 *  Created on: 4 june 2011
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009, 2010, 2011 - Sébastien Kus
 *
 *  mount place classes
 */

#ifndef TC_MOUNT_H_
#define TC_MOUNT_H_

#include <gio/gio.h>

class tc_mount_from_file
{
private:
	bool				mounted;
	bool				error;
	GFile				*file;

	static void			finishing(GObject *source, GAsyncResult *res, gpointer data);

public:
						tc_mount_from_file(GFile *file);
						~tc_mount_from_file();

	bool				is_mounted();
	bool				mount();
};

#endif /* TC_MOUNT_H_ */
