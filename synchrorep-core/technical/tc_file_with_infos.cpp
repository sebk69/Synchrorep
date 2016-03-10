/*
 * tc_file_with_infos.cpp
 *
 *  Created on: 16 juil. 2009
 *      ©2009, 2010 - Sébastien Kus
 *
 *  This source is under GNU/GLP V3 licence
 */

#include <string.h>
#include <stdlib.h>

#include "tc_file_with_infos.h"

#include "md5.hh"

tc_file_with_infos :: tc_file_with_infos(GFile *file)
{
	this->file = file;
	this->file_path = g_file_get_uri(this->file);
	this->partial_path = NULL;
	this->base_name = NULL;

	this->refresh();

	unsigned int				i;

	if(this->get_path() != NULL)
	{
		char						*temp = this->get_path();
		for(i=strlen(temp);i > 0 && temp[i] != '/';i--);
		this->parent_folder_path = strdup(temp);
		this->parent_folder_path[i+1] = '\0';
	}
	else
		this->parent_folder_path = NULL;

	this->database_modification_time.tv_sec = 0;
	this->database_modification_time.tv_usec = 0;
	this->modification_time.tv_sec = 0;
	this->modification_time.tv_usec = 0;
	this->md5 = NULL;
	this->db_md5 = NULL;
}

tc_file_with_infos :: tc_file_with_infos(char *file_uri)
{
	this->file = g_file_new_for_commandline_arg(file_uri);
	this->refresh();

	this->file_path = NULL;
	this->partial_path = NULL;
	this->base_name = NULL;

	unsigned int				i;
	char						*temp = this->get_path();
	for(i=strlen(temp);i > 0 && temp[i] != '/';i--);
	this->parent_folder_path = strdup(temp);
	this->parent_folder_path[i+1] = '\0';

	this->database_modification_time.tv_sec = 0;
	this->database_modification_time.tv_usec = 0;
	this->modification_time.tv_sec = 0;
	this->modification_time.tv_usec = 0;
	this->md5 = NULL;
	this->db_md5 = NULL;
}


tc_file_with_infos :: tc_file_with_infos(GFile *file, char *parent_folder_uri)
{
	this->file = file;
	this->file_path = g_file_get_uri(this->file);
	this->partial_path = NULL;
	this->base_name = NULL;

	this->refresh();

	if(parent_folder_uri[0] == '/')
		this->parent_folder_path = strdup(parent_folder_uri);
	else
	{
		unsigned int				i;
		char						*temp = this->get_path();
		for(i=strlen(temp);i > 0 && temp[i] != '/';i--);
		this->parent_folder_path = strdup(temp);
		this->parent_folder_path[i+1] = '\0';
	}

	this->database_modification_time.tv_sec = 0;
	this->database_modification_time.tv_usec = 0;
	this->modification_time.tv_sec = 0;
	this->modification_time.tv_usec = 0;
	this->md5 = NULL;
	this->db_md5 = NULL;
}

tc_file_with_infos :: tc_file_with_infos(char *file_uri, char *parent_folder_uri)
{
	this->file = g_file_new_for_commandline_arg(file_uri);
	this->refresh();
	this->partial_path = NULL;
	this->base_name = NULL;

	if(file_uri[0] == '/')
	{
		this->file_path = strdup(file_uri);
	}
	else
	{
		this->file_path = NULL;
	}

	if(parent_folder_uri[0] == '/')
	{
		this->parent_folder_path = strdup(parent_folder_uri);
	}
	else
	{
		unsigned int				i;
		char						*temp = this->get_path();
		for(i=strlen(temp);i > 0 && temp[i] != '/';i--);
		this->parent_folder_path = strdup(temp);
		this->parent_folder_path[i+1] = '\0';
	}

	this->database_modification_time.tv_sec = 0;
	this->database_modification_time.tv_usec = 0;
	this->modification_time.tv_sec = 0;
	this->modification_time.tv_usec = 0;
	this->md5 = NULL;
	this->db_md5 = NULL;
}

tc_file_with_infos :: tc_file_with_infos(tc_file_with_infos &to_dup)
{
	this->file = to_dup.file;
	this->file_path = to_dup.file_path;
	this->base_name = to_dup.base_name;
	this->parent_folder_path = to_dup.parent_folder_path;
	this->modification_time = to_dup.modification_time;
	this->database_modification_time = to_dup.database_modification_time;
	this->partial_path = to_dup.partial_path;
	this->md5 = to_dup.md5;
	this->db_md5 = to_dup.db_md5;
}

tc_file_with_infos :: ~tc_file_with_infos()
{
}

char*
tc_file_with_infos :: get_path()
{
	if(this->file_path == NULL)
		this->file_path = g_file_get_uri(this->file);
	return strdup(this->file_path);
}

char*
tc_file_with_infos :: get_partial_path()
{
	return strdup(this->partial_path);
}

void
tc_file_with_infos :: set_partial_path(char *partial_path)
{
	if(this->partial_path != NULL)
		free(this->partial_path);
	this->partial_path = strdup(partial_path);
}

char*
tc_file_with_infos :: get_parent_path()
{
	return strdup(this->parent_folder_path);
}

char*
tc_file_with_infos :: get_parent_uri()
{
	return g_file_get_uri(g_file_get_parent(this->file));
}

// refreshing modification date
void
tc_file_with_infos :: refresh()
{
	GFileInfo				*info;

	if(g_file_query_exists(this->file, NULL))
	{
		info = g_file_query_info(this->file, G_FILE_ATTRIBUTE_TIME_MODIFIED, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, NULL);
		g_file_info_get_modification_time(info, &this->modification_time);
		g_object_unref(info);
	}
	else
	{
		this->modification_time.tv_sec = 0;
		this->modification_time.tv_usec = 0;
	}
}

// duplication by operator =
tc_file_with_infos&
tc_file_with_infos :: operator=(const tc_file_with_infos& to_dup)
{
	if(this == &to_dup)
			return *this;

	this->file = to_dup.file;
	this->file_path = to_dup.file_path;
	this->parent_folder_path = to_dup.parent_folder_path;
	this->modification_time = to_dup.modification_time;
	this->database_modification_time = to_dup.database_modification_time;
	this->partial_path = to_dup.partial_path;

	return *this;
}

char*
tc_file_with_infos :: get_uri()
{
	return g_file_get_uri(this->file);
}

// comparators
bool
tc_file_with_infos :: operator<(const tc_file_with_infos& compared)
{
	return (strcmp(this->file_path, compared.file_path) < 0);
}

bool
tc_file_with_infos :: operator>(const tc_file_with_infos& compared)
{
	return (strcmp(this->file_path, compared.file_path) > 0);
}

int
tc_file_with_infos :: compare_with_file(GFile *compared)
{
	char				*cmp1 = g_file_get_uri(this->file),
						*cmp2 = g_file_get_uri(compared);
	int					result;
	result = strcmp(cmp1, cmp2);

	g_free(cmp1);
	g_free(cmp2);

	return result;
}

char*
tc_file_with_infos :: get_base_name()
{
	if(this->base_name == NULL)
		this->base_name = g_uri_escape_string(g_file_get_basename(this->file), NULL, false);

	return strdup(this->base_name);
}

GFile*
tc_file_with_infos :: get_file()
{
	return this->file;
}

GTimeVal
tc_file_with_infos :: get_modification_time()
{
	if(this->modification_time.tv_sec == 0)
	{
		GFileInfo			*info = g_file_query_info(this->file, "time::modified", G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL, NULL);

		if(info != NULL)
			g_file_info_get_modification_time(info, &this->modification_time);
		else
			this->modification_time = {0, 0};
	}
	return this->modification_time;
}

GTimeVal
tc_file_with_infos :: get_database_modification_time()
{
	return this->database_modification_time;
}

void
tc_file_with_infos :: set_database_modification_time(GTimeVal time)
{
	this->database_modification_time = time;
}

char*
tc_file_with_infos :: get_db_md5()
{
	if(this->db_md5 != NULL)
		return strdup(this->db_md5);
	return NULL;
}

void
tc_file_with_infos :: set_db_md5(char *md5)
{
	if(this->db_md5 == NULL)
		free(this->db_md5);
	this->db_md5 = md5;
}

char*
tc_file_with_infos :: get_md5(GError **error)
{
	if(error == NULL)
	{
		error = (GError**)malloc(sizeof(GError**));
		*error = NULL;
	}
	if(g_file_query_exists(this->file, NULL) &&
			g_file_query_file_type(this->file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		if(this->md5 == NULL)
		{
			MD5					outmd5;
			char				buffer[1025];
			GFileInputStream	*stream;
			gssize				size;

			stream = g_file_read(this->file, NULL, error);
			if(*error != NULL)
			{
				cerr<<"Failed to read md5sum : "<<(*error)->message<<endl;
				return NULL;
			}
			while((size = g_input_stream_read(G_INPUT_STREAM(stream), buffer, 1024, NULL, error)) != 0)
			{
				outmd5.update((unsigned char *)buffer, (unsigned int)size);
				if(*error != NULL)
				{
					cerr<<"Failed to read md5sum : "<<(*error)->message<<endl;
					return NULL;
				}
			}
			outmd5.finalize();

			this->md5 = outmd5.hex_digest();

			g_input_stream_close(G_INPUT_STREAM(stream), NULL, NULL);
		}

		return strdup(this->md5);
	}
	else
	{
		this->md5 = NULL;
		return NULL;
	}
}


bool
tc_file_with_infos :: is_folder()
{
	return (g_file_query_file_type(this->file, G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY);
}
