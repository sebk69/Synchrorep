/*
 * tc_find_files.cpp
 *
 *  Created on: 15 juil. 2009
 *      ©2009, 2010 - Sébastien Kus
 *  This source is under GNU/GLP V3 licence
 *
 *  Technical class to retrieve list of files in a folder
 */

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdlib.h>

#include "../applicative/application.h"
#include "tc_find_files.h"

tc_find_files :: tc_find_files(GFile *base_folder, mutex_var<bool> *pcancel)
{
	this->base_folder = new tc_file_with_infos(base_folder);
	this->base_folder_path = g_file_get_path(base_folder);
	this->num_files = 0;
	this->error = NULL;
	if(pcancel != NULL)
		this->cancel = pcancel;
	else
	{
		this->cancel = new mutex_var<bool>;
		this->cancel->set(false);
	}
}

tc_find_files :: ~tc_find_files()
{
}

GThread*
tc_find_files :: process_t()
{
	return g_thread_create(&this->process, (gpointer)this, true, NULL);
}

gpointer
tc_find_files :: process(gpointer THIS_P)
{
	bool				*result = new bool;
	tc_find_files		*THIS = (tc_find_files*)THIS_P;

	// check base folder exists
	if(!g_file_query_exists(THIS->base_folder->get_file(), NULL))
	{
		*result = false;
		return result;
	}

	// register base_folder
	THIS->base = new tc_find_files_cell;
	THIS->base->data = THIS->base_folder;
	THIS->base->up = NULL;
	THIS->base->next = NULL;
	THIS->base->down = NULL;

	// process as sub folder
	if(!THIS->cancel->get())
		THIS->sub_folder(THIS->base);

	*result = true;
	return (gpointer)result;
}

bool
tc_find_files :: process()
{
	return *(bool*)this->process(this);
}

bool
tc_find_files :: ftp_copy(GFile *target, GError **error)
{
	char				*ctarget = g_file_get_uri(target);

	if(g_file_query_file_type(this->base->data->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY)
	{
		if(!g_file_make_directory(target, NULL, error))
			return false;
		if(this->begin())
			do
			{
				char			path[10000];
				strcpy(path, ctarget);
				strcat(path, "/");
				strcat(path, this->current_file->data->get_partial_path());
				GFile			*to_copy = g_file_new_for_uri(path);
				if(g_file_query_file_type(this->current_file->data->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
				{
					if(!g_file_copy(this->current_file->data->get_file(), to_copy, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
						return false;
				}
				else
				{
					if(!g_file_make_directory(to_copy, NULL, error))
						return false;
				}
			} while(this->next() && !this->cancel->get());
		}
		else
		{
			if(!g_file_copy(this->base->data->get_file(), target, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, error))
				return false;
		}

	return true;
}

bool
tc_find_files :: ftp_delete(GError **error, mutex_var<int> *decrement)
{
	tc_find_files_cell			*scan;
	tc_find_files_cell			*previous;

	scan = this->base;
	while(scan != NULL && !this->cancel->get())
	{
		// case it's a leaf ==> delete
		if(scan->down == NULL)
		{
			if(!g_file_delete(scan->data->get_file(), NULL, error))
				return false;
			if(decrement != NULL)
			{
				int			num = decrement->get()-1;
				decrement->set(num);
			}
			// reconstruct up level
			if(scan->up != NULL)
				scan->up->down = scan->next;
			else
				return true;

			// scan next
			previous = scan->up;
			scan = scan->next;
			if(scan == NULL)
				scan = previous;
		}
		else
			scan = scan->down;
	}
	if(this->cancel->get())
		return false;
	return true;
}

void
tc_find_files :: sub_folder(tc_find_files_cell *node)
{
	GFileEnumerator			*list_of_childs;
	GFileInfo				*info;
	tc_file_with_infos		*file;
	const char				*temp_name;
	char					temp3[10000];
	char					*folder_path = node->data->get_path();
	GFileType				finfo;

	list_of_childs = g_file_enumerate_children(node->data->get_file(), "*", G_FILE_QUERY_INFO_NONE, NULL, &error);
	if(list_of_childs != NULL)
	{
		do
		{
			error = NULL;
			// TODO : vérif uri
			info = g_file_enumerator_next_file(list_of_childs, NULL, &this->error);
			if(this->error == NULL && info != NULL)
			{
				// add new file
				temp_name = g_file_info_get_name(info);
				temp3[0]='\0';
				strcat(temp3, folder_path);
				strcat(temp3, "/");
				strcat(temp3, temp_name);
				file = new tc_file_with_infos(temp3, folder_path);
				finfo = g_file_info_get_file_type(info);
				if(finfo == G_FILE_TYPE_DIRECTORY || finfo == G_FILE_TYPE_REGULAR)
					this->add(node, file);
				switch(finfo)
				{
					case G_FILE_TYPE_DIRECTORY:
						if(!this->cancel->get())
							this->sub_folder(this->current_file);
					break;

					default:
					break;
				}
				g_object_unref(info);
			}
			else if(this->error != NULL)
			{
				this->cancel->set(true);
				cerr<<this->error->message<<endl;
			}
		} while(info != NULL && !this->cancel->get());
		g_object_unref(list_of_childs);
	}
	else
	{
		cerr<<this->error->message<<endl;
	}

	free(folder_path);
}

// get base folder path
char*
tc_find_files :: get_base_folder_path()
{
	char			*path = this->base_folder->get_path();
	char			*result = strdup(path);

	g_free(path);
	return result;
}

// get current time
GTimeVal
tc_find_files :: get_time(GError **error)
{
	char			*base_uri = this->base_folder->get_uri();
	char			*local = strdup(base_uri);
	GTimeVal		result = {0, 0};

	local[7] = '\0';
	/*if(strcmp(local, "file://") == 0)
	{
		// file is local ==> get machine current time
		g_get_current_time(&result);
	}
	else*/
	{
		// file is not local ==> create a empty file and get creation date
		char			*new_uri = new char[strlen(base_uri) + 11];

		new_uri[0] = '\0';
		strcat(new_uri, base_uri);
		strcat(new_uri, "/temp.time");
		GFile			*temp_time_file = g_file_new_for_uri(new_uri);
		GFileOutputStream	*create_stream = g_file_create(temp_time_file, G_FILE_CREATE_NONE, NULL, error);
		if(create_stream == NULL)
		{
			cerr<<"Error creating test file : "<<(*error)->message<<endl;
			return result;
		}
		else
		{
			// release output
			g_object_unref(create_stream);
			// get date
			GFileInfo			*info;
			info = g_file_query_info(temp_time_file, G_FILE_ATTRIBUTE_TIME_MODIFIED, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, error);
			if(info != NULL)
			{
				g_file_info_get_modification_time(info, &result);
				g_object_unref(info);
				// and remove it
				if(!g_file_delete(temp_time_file, NULL, error))
				{
					cerr<<"Error deleting test file : "<<(*error)->message<<endl;
					return result;
				}
			}
			else
			{
				cerr<<"Error get infos of test file : "<<(*error)->message<<endl;
				return result;
			}
		}
		delete new_uri;
	}

	free(local);
	free(base_uri);

	return result;
}

// scan files
bool
tc_find_files :: begin()
{
	this->current_file = this->base;
	return this->next();
}

bool
tc_find_files :: next(bool scan_down)
{
	if(this->current_file->down != NULL && scan_down)
		this->current_file = this->current_file->down;
	else if(this->current_file->next != NULL)
		this->current_file = this->current_file->next;
	else if(this->current_file->up != this->base && this->current_file->up != NULL)
	{
		this->current_file = this->current_file->up;
		return this->next(false);
	}
	else
		return false;
	return true;
}

// dumping datas
void
tc_find_files :: dump()
{
	char							*path;

	cout<<"dumping files from : "<<this->base_folder_path<<endl;
	this->begin();
	while(this->next())
	{
		path = this->current_file->data->get_path();
		cout<<path<<":"<<this->current_file->data->get_database_modification_time().tv_sec<<":"<<this->current_file->data->get_modification_time().tv_sec<<endl;
		free(path);
	}
}

// get datas
GFile*
tc_find_files :: get_current()
{
	return this->current_file->data->get_file();
}

GTimeVal
tc_find_files :: get_current_modification_time()
{
	return this->current_file->data->get_modification_time();
}

GTimeVal
tc_find_files :: get_current_database_modification_time()
{
	return this->current_file->data->get_database_modification_time();
}

void
tc_find_files :: set_current_database_modification_time(GTimeVal time)
{
	this->current_file->data->set_database_modification_time(time);
}

tc_file_with_infos*
tc_find_files :: get_file_with_info()
{
	return this->current_file->data;
}

char*
tc_find_files :: get_current_partial_path()
{
	return this->current_file->data->get_partial_path();
}

void
tc_find_files :: refresh_current()
{
	this->refresh_current(NULL);
}

void
tc_find_files :: refresh_current(tc_find_files_cell *from)
{
	if(from == NULL)
	{
		from = this->current_file;
		from->data->refresh();
		if(from->down != NULL)
			this->refresh_current(from->down);
	}
	else
	{
		while(from != NULL)
		{
			from->data->refresh();
			if(from->down != NULL)
				this->refresh_current(from->down);
			from = from->next;
		}
	}
}

// adding
void
tc_find_files :: add(tc_find_files_cell *node, tc_file_with_infos *file)
{
	tc_find_files_cell			*current,
								*temp,
								*temp2;
	// set partial path
	char						*base_path = this->base->data->get_path();
	char						*full_path = file->get_path();
	char						*partial_path = new char[strlen(full_path) + 1];

	strcpy(partial_path, (char*)(full_path + strlen(base_path) + 1));
	file->set_partial_path(partial_path);
	free(base_path);
	free(full_path);
	delete partial_path;

	// add file to node
	this->num_files++;
	if(node->down == NULL)
	{
		node->down = new tc_find_files_cell;
		node->down->data = file;
		node->down->down = NULL;
		node->down->next = NULL;
		node->down->up = node;
		current = node->down;
	}
	else
	{
		temp = node->down;
		temp2 = NULL;
		char			*temp_base,
						*file_base;
		temp_base = temp->data->get_base_name();
		file_base = file->get_base_name();
		while(temp != NULL && strcmp(temp_base, file_base) < 0)
		{
			temp2 = temp;
			temp = temp->next;
			if(temp != NULL)
			{
				free(temp_base);
				temp_base = temp->data->get_base_name();
			}
		}
		free(temp_base);
		free(file_base);
		if(temp2 == NULL)
		{
			current = new tc_find_files_cell;
			current->data = file;
			current->next = temp;
			current->up = node;
			current->down = NULL;
			node->down = current;
		}
		else
		{
			current = new tc_find_files_cell;
			current->data = file;
			current->next = temp;
			temp2->next = current;
			current->up = node;
			current->down = NULL;
		}
	}
	this->current_file = current;
}
void
tc_find_files :: add_db_data(char *partial_path, GTimeVal modification_date)
{
	char				*base_path = this->base->data->get_path();
	char				*current_partial = new char[strlen(partial_path) + 1];
	char				*scan_path;
	int					i=0,
						j=0;
	tc_find_files_cell	*current,
						*previous;
	char				*path = new char[strlen(base_path) + strlen(partial_path) + 2];
	char				*full_current_partial_path = new char[strlen(partial_path) + 2];
	int					cmp;

	// in case of empty string
	while(partial_path[i] != '\0' && partial_path[i] == '/')
		i++;
	if(partial_path[j] == '\0')
		return;

	// searching
	current = base->down;
	previous = base;
	do
	{
		// get next portion of path
		j = 0;
		while(partial_path[i] != '/' && partial_path[i] != '\0')
		{
			current_partial[j] = partial_path[i];
			i++;
			j++;
		}
		current_partial[j] = '\0';

		if(current != NULL)
		{
			// find if portion exists in this
			scan_path = current->data->get_base_name();
			cmp = strcmp(scan_path, current_partial);
			while(current != NULL && cmp < 0)
			{
				current = current->next;
				if(current != NULL)
				{
					free(scan_path);
					scan_path = current->data->get_base_name();
					cmp = strcmp(scan_path, current_partial);
				}
			}
			free(scan_path);
		}

		// this portion is not found
		if(current == NULL || cmp != 0)
		{
			strcpy(full_current_partial_path, partial_path);
			full_current_partial_path[i+1] = '\0';
			path[0] = '\0';
			strcat(path, base_path);
			strcat(path, "/");
			strcat(path, full_current_partial_path);
			tc_file_with_infos	*temp_file = new tc_file_with_infos(path);
			this->add(previous, temp_file);
			previous = this->current_file;
			current = NULL;
		}
		// portion found
		else
		{
			previous = current;
			current = previous->down;
		}

		// else skip separators and continue searching at down point
		while(partial_path[i] == '/')
			i++;
	}
	while(partial_path[i] != '\0');

	// at this point folder is found and current_file is the file searched
	this->current_file = previous;
	this->current_file->data->set_database_modification_time(modification_date);

	free(base_path);
	delete path;
	delete full_current_partial_path;
}

void
tc_find_files :: remove_down()
{
	if(this->current_file != NULL)
		this->current_file->down = NULL;
}

int
tc_find_files :: count()
{
	return this->num_files;
}
