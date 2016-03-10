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

using namespace std;
#include <iostream>

#include <gtk/gtk.h>

#include "tc_mount.h"

tc_mount_from_file :: tc_mount_from_file(GFile *file)
{
	this->file = file;
	this->mounted = false;
	this->error = false;
}

tc_mount_from_file :: ~tc_mount_from_file()
{
}

void
tc_mount_from_file :: finishing(GObject *source, GAsyncResult *res, gpointer data)
{
	tc_mount_from_file			*THIS = (tc_mount_from_file*)data;
	GError						*error = NULL;
	if(g_file_mount_enclosing_volume_finish(THIS->file, res, &error))
		THIS->mounted = true;
	else
		THIS->error = true;
	gtk_main_quit();
}

bool
tc_mount_from_file :: is_mounted()
{
	GMount			*mount;
	GError			*error = NULL;
	if(g_file_query_file_type(this->file, G_FILE_QUERY_INFO_NONE, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		mount = g_file_find_enclosing_mount(this->file, NULL, &error);
		if(mount == NULL)
			return false;
		else
			return true;
	}
	else
		return true;
}

bool
tc_mount_from_file :: mount()
{
	// mount if not mounted
	if(g_file_find_enclosing_mount(this->file, NULL, NULL) == NULL)
	{
		this->mounted = false;
		GMountOperation *mop = gtk_mount_operation_new (NULL);
		g_file_mount_enclosing_volume(file, G_MOUNT_MOUNT_NONE, mop, NULL, (GAsyncReadyCallback)tc_mount_from_file :: finishing, (gpointer)this);
		g_object_unref (mop);
		gtk_main();
	}
	else
		this->mounted = true;

	return this->mounted;
}
