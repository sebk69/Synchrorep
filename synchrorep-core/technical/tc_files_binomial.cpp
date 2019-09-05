/*
 * tc_file_binomial.cpp
 *
 *  Created on: 19 juil. 2009
 *      ©2009, 2010, 2019 - Sébastien Kus
 *
 *  This source is under GNU/GLP V3 licence
 *
 *  Operations on binomial files
 */

#include <iostream>

#include <string.h>

#include "tc_files_binomial.h"
#include "tc_find_files.h"

// constructor
tc_files_binomial :: tc_files_binomial(tc_file_with_infos *pfrom, tc_file_with_infos *pto)
{
	this->from = new tc_file_with_infos(*pfrom);
	this->to = new tc_file_with_infos(*pto);
	this->error = NULL;
}

tc_files_binomial :: ~tc_files_binomial()
{
	delete this->from;
	delete this->to;
}

// compare modifications from filesystem and database using md5sum and return status
_files_compare
tc_files_binomial :: md5sum_compare()
{
	if(g_file_query_file_type(this->from->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY
			&& g_file_query_file_type(this->to->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		// get md5sum
		char				*from_md5 = this->from->get_md5(&this->error);
		if(this->error != NULL)
			return _comparison_error;
		if(from_md5 == NULL)
			from_md5 = strdup("");
		char				*to_md5 = this->to->get_md5(&this->error);
		if(this->error != NULL)
			return _comparison_error;
		if(to_md5 == NULL)
			to_md5 = strdup("");
		char				*from_db_md5 = this->from->get_db_md5();
		if(from_db_md5 == NULL)
			from_db_md5 = strdup("");
		char				*to_db_md5 = this->to->get_db_md5();
		if(to_db_md5 == NULL)
			to_db_md5 = strdup("");
		// files are not modified
		if(strcmp(from_md5, from_db_md5) == 0 &&
				strcmp(to_md5, to_db_md5) == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// both files have been deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec == 0)
			return _both_deleted;
		// from has been deleted
		else if(this->from->get_database_modification_time().tv_sec != 0 &&
				this->from->get_modification_time().tv_sec == 0)
			return _from_deleted;
		// to has been deleted
		else if(this->to->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0)
			return _to_deleted;
		// both files have been modified and from is more recent than to
		else if(strcmp(from_md5, from_db_md5) != 0 &&
				strcmp(to_md5, to_db_md5) != 0 &&
				this->from->get_modification_time().tv_sec + this->from_dif >
						this->to->get_modification_time().tv_sec + this->to_dif)
		{
			// test if matching md5
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _both_modified_from_recent;
		}
		// both files have been modified and to is more recent than from
		else if(strcmp(from_md5, from_db_md5) != 0 &&
				strcmp(to_md5, to_db_md5) != 0 &&
				this->to->get_modification_time().tv_sec + this->to_dif >
						this->from->get_modification_time().tv_sec + this->from_dif)
		{
			// test if matching md5
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _both_modified_to_recent;
		}
		// both files have been modified and from is same md5 as to
		else if(strcmp(from_md5, from_db_md5) != 0 &&
				strcmp(to_md5, to_db_md5) != 0 &&
				strcmp(from_md5, to_md5) == 0)
			return _both_modified_equal;
		// from has been modified and not to
		else if(strcmp(from_md5, from_db_md5) != 0 &&
				strcmp(to_md5, to_db_md5) == 0)
		{
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _from_modified;
		}
		// to has been modified and not from
		else if(strcmp(from_md5, from_db_md5) == 0 &&
				strcmp(to_md5, to_db_md5) != 0)
		{
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _to_modified;
		}
		free(from_md5);
		free(to_md5);
		free(from_db_md5);
		free(to_db_md5);
	}
	else
	{
		// files are not modified
		if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// files are not modified
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _both_modified_equal;
		// from modified
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _from_modified;
		// to modified
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _to_modified;

		// from deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _from_deleted;
		// to deleted
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _to_deleted;
		// both deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _both_deleted;
	}

	// in case of none matching return _files_not_modified by security
	cout<<this->from->get_partial_path()<<"From file : "<<this->from->get_modification_time().tv_sec
			<<", From db : "<<this->from->get_database_modification_time().tv_sec
			<<"/To file : "<<this->to->get_modification_time().tv_sec
			<<", To db : "<<this->to->get_database_modification_time().tv_sec<<endl;
	return _files_not_modified;
}


// compare modifications dates from filesystem and database and return status
_files_compare
tc_files_binomial :: sync_compare(bool md5sum_on_both_modified)
{
	if(g_file_query_file_type(this->from->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY
			&& g_file_query_file_type(this->to->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		// files are not modified
		if(this->from->get_modification_time().tv_sec == this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec == this->to->get_database_modification_time().tv_sec &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// both files have been deleted
		else if(this->from->get_modification_time().tv_sec == 0 && this->to->get_modification_time().tv_sec == 0)
			return _both_deleted;
		// from has been deleted
		else if(this->from->get_database_modification_time().tv_sec != 0 &&
				this->from->get_modification_time().tv_sec == 0)
			return _from_deleted;
		// to has been deleted
		else if(this->to->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0)
			return _to_deleted;
		// both files have been modified and from is more recent than to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->from->get_modification_time().tv_sec + this->from_dif >
						this->to->get_modification_time().tv_sec + this->to_dif)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _both_modified_from_recent;
		}
		// both files have been modified and to is more recent than from
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec + this->to_dif >
						this->from->get_modification_time().tv_sec + this->from_dif)
		{
			// test if matching md5
			char				*from_md5 = this->from->get_md5(&this->error);
			if(this->error != NULL)
				return _comparison_error;
			char				*to_md5 = this->to->get_md5(&this->error);
			if(this->error != NULL)
				return _comparison_error;
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _both_modified_to_recent;
		}
		// both files have been modified and from is same date as to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec + this->to_dif ==
						this->from->get_modification_time().tv_sec + this->from_dif)
			return _both_modified_equal;
		// from has been modified and not to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec <= this->to->get_database_modification_time().tv_sec)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _from_modified;
		}
		// to has been modified and not from
		else if(this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->from->get_modification_time().tv_sec <= this->from->get_database_modification_time().tv_sec)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _to_modified;
		}
	}
	else
	{
		// files are not modified
		if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// files are not modified
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _both_modified_equal;
		// from modified
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _from_modified;
		// to modified
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _to_modified;

		// from deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _from_deleted;
		// to deleted
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _to_deleted;
		// both deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _both_deleted;
	}

	// in case of none matching return _files_not_modified by security
	cout<<this->from->get_partial_path()<<"From file : "<<this->from->get_modification_time().tv_sec
			<<", From db : "<<this->from->get_database_modification_time().tv_sec
			<<"/To file : "<<this->to->get_modification_time().tv_sec
			<<", To db : "<<this->to->get_database_modification_time().tv_sec<<endl;
	return _files_not_modified;
}


// compare modifications dates from filesystem and return status
_files_compare
tc_files_binomial :: copy_compare(bool md5sum_on_both_modified)
{
	if(g_file_query_file_type(this->from->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY
			&& g_file_query_file_type(this->to->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
	{
		// files are not modified
		if(this->from->get_modification_time().tv_sec == this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec == this->to->get_database_modification_time().tv_sec &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// both files have been deleted
		else if(this->from->get_modification_time().tv_sec == 0 && this->to->get_modification_time().tv_sec == 0)
			return _both_deleted;
		// from has been deleted
		else if(this->from->get_modification_time().tv_sec == 0)
			return _from_deleted;
		// to has been deleted
		else if(this->to->get_modification_time().tv_sec == 0)
			return _to_deleted;
		// both files have been modified and from is more recent than to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->from->get_modification_time().tv_sec + this->from_dif >
						this->to->get_modification_time().tv_sec + this->to_dif)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _both_modified_from_recent;
		}
		// both files have been modified and to is more recent than from
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec + this->to_dif >
						this->from->get_modification_time().tv_sec + this->from_dif)
		{
			// test if matching md5
			char				*from_md5 = this->from->get_md5(&this->error);
			if(this->error != NULL)
				return _comparison_error;
			char				*to_md5 = this->to->get_md5(&this->error);
			if(this->error != NULL)
				return _comparison_error;
			if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
				return _both_modified_equal;
			return _both_modified_to_recent;
		}
		// both files have been modified and from is same date as to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec + this->to_dif ==
						this->from->get_modification_time().tv_sec + this->from_dif)
			return _both_modified_equal;
		// from has been modified and not to
		else if(this->from->get_modification_time().tv_sec > this->from->get_database_modification_time().tv_sec &&
				this->to->get_modification_time().tv_sec <= this->to->get_database_modification_time().tv_sec)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _from_modified;
		}
		// to has been modified and not from
		else if(this->to->get_modification_time().tv_sec > this->to->get_database_modification_time().tv_sec &&
				this->from->get_modification_time().tv_sec <= this->from->get_database_modification_time().tv_sec)
		{
			if(md5sum_on_both_modified)
			{
				// test if matching md5
				char				*from_md5 = this->from->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				char				*to_md5 = this->to->get_md5(&this->error);
				if(this->error != NULL)
					return _comparison_error;
				if(from_md5 != NULL && to_md5 != NULL && strcmp(from_md5, to_md5) == 0)
					return _both_modified_equal;
			}
			return _to_modified;
		}
	}
	else
	{
		// files are not modified
		if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _files_not_modified;
		// from deleted
		else if(this->from->get_modification_time().tv_sec == 0)
			return _from_deleted;
		// to deleted
		else if(this->to->get_modification_time().tv_sec == 0)
			return _to_deleted;
		// both deleted
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec != 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec != 0)
			return _both_deleted;
		// files are both modified and equal
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _both_modified_equal;
		// from modified
		else if(this->from->get_modification_time().tv_sec != 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec == 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _from_modified;
		// to modified
		else if(this->from->get_modification_time().tv_sec == 0 &&
				this->from->get_database_modification_time().tv_sec == 0 &&
				this->to->get_modification_time().tv_sec != 0 &&
				this->to->get_database_modification_time().tv_sec == 0)
			return _to_modified;
	}

	// in case of none matching return _files_not_modified by security
	return _files_not_modified;
}


bool
tc_files_binomial :: copy(from_or_to_of_binome_type which, bool old_to_trash)
{
	tc_file_with_infos		*source;
	tc_file_with_infos		*target;

	// choose which one to copy
	switch(which)
	{
		case _binomial_from:
			source = this->from;
			target = this->to;
		break;

		case _binomial_to:
			source = this->to;
			target = this->from;
		break;
	}

	// if it is a directory, just create it
	if(g_file_query_file_type(source->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY)
	{
		if(!g_file_query_exists(target->get_file(), NULL))
		{
			this->error = NULL;
			if(g_file_make_directory(target->get_file(), NULL, &this->error))
			{
				target->refresh();
				return true;
			}
			else
				return false;
		}
		else
			return true;
	}

	// make temporary file

	// - get common path
	char				*parent_target;

	// choose target parent
	parent_target = target->get_parent_path();

	// - build temp base name
	char				*the_pid = application :: get_pid();
	char				base_name[strlen(the_pid) + 6];

	base_name[0] = '\0';
	strcat(base_name, the_pid);
	strcat(base_name, ".tmp");
	free(the_pid);

	// - build gfile
	GFile				*temp_file;
	char				temp_full_path[strlen(parent_target) + strlen(base_name) + 1];

	temp_full_path[0] = '\0';
	strcat(temp_full_path, parent_target);
	strcat(temp_full_path, "/");
	strcat(temp_full_path, base_name);
	temp_file = g_file_new_for_uri(temp_full_path);

	// first copy file to temporary file
	this->error = NULL;
	if(g_file_copy(source->get_file(), temp_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &this->error))
	{
		// delete target (put to trash if needed
		switch(which)
		{
			case _binomial_from:
				if(g_file_query_exists(target->get_file(), NULL))
					if(!this->remove(_binomial_to, old_to_trash))
						return false;
			break;

			case _binomial_to:
				if(g_file_query_exists(target->get_file(), NULL))
					if(!this->remove(_binomial_from, old_to_trash))
						return false;
			break;
		}

		// and move temporary file to target
		if(!g_file_move(temp_file, target->get_file(), G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &this->error))
			return false;
	}
	else
		return false;

	// at this point all is ok
	target->refresh();
	return true;
}

bool
tc_files_binomial :: remove(from_or_to_of_binome_type which, bool put_to_trash, mutex_var<int> *decrement, mutex_var<bool> *cancel)
{
	tc_file_with_infos			*target;
	bool						local,
								ftp;

	// choose which one to copy
	switch(which)
	{
		case _binomial_from:
			target = this->from;
		break;

		case _binomial_to:
			target = this->to;
		break;
	}

	// put to trash if necessary
	if(put_to_trash)
	{
		// test if file is local
		char				uri_type[8];
		char				*uri = target->get_uri();

		// if first is / ==> it is local
		ftp = false;
		if(uri[0] == '/')
			local = true;
		else
		{
			strncpy(uri_type, uri, 7);
			uri_type[7] = 0;
			if(strcmp(uri_type, "file://") == 0)
				local = true;
			else
			{
				local = false;
				// test if it is ftp file
				uri_type[7] = '\0';
				if(strcmp(uri_type, "ftp://"))
				{
					ftp = true;
				}
			}
		}

		// if it is not a local uri => copy it to local work folder
		if(!local)
		{
			// - make temporary file
			GFile			*temp_file;
			char			*basename = g_file_get_basename(target->get_file());
			char			*work_folder = application :: get_work_folder();
			char			temp_path[strlen(work_folder) + strlen(basename) + 3];

			temp_path[0] = '\0';
			strcat(temp_path, work_folder);
			strcat(temp_path, "/");
			strcat(temp_path, basename);
			temp_file = g_file_new_for_path(temp_path);
			g_free(basename);
			free(work_folder);

			// make copy
			this->error = NULL;
			if(!ftp || g_file_query_file_type(target->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) != G_FILE_TYPE_DIRECTORY)
			{
				if(!g_file_copy(target->get_file(), temp_file, G_FILE_COPY_OVERWRITE, NULL, NULL, NULL, &this->error))
					return false;
			}
			else
			{
				// if it is ftp ==> must delete all content before
				tc_find_files		files_to_copy(target->get_file(), cancel);
				files_to_copy.process();
				if(!files_to_copy.ftp_copy(temp_file, &this->error))
					return false;
			}

			// move temp_file to trash
			if(!g_file_trash(temp_file, NULL, &this->error))
				return false;

			if(g_file_query_file_type(target->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY)
			{
				// delete distant file
				tc_find_files		files_to_delete(target->get_file(), cancel);
				files_to_delete.process();
				if(!files_to_delete.ftp_delete(&this->error, decrement))
					return false;
			}
			else
				if(!g_file_delete(target->get_file(), NULL, &this->error))
					return false;
		}
		else
		{
			// file is local, put to trash
			this->error = NULL;
			if(!g_file_trash(target->get_file(), NULL, &this->error))
				return false;
		}
	}
	else
	{
		this->error = NULL;
		// must delete all content before
		if(g_file_query_file_type(target->get_file(), G_FILE_QUERY_INFO_NOFOLLOW_SYMLINKS, NULL) == G_FILE_TYPE_DIRECTORY)
		{
			// delete distant file
			tc_find_files		files_to_delete(target->get_file(), cancel);
			files_to_delete.process();
			if(!files_to_delete.ftp_delete(&this->error, decrement))
				return false;
		}
		else
			if(!g_file_delete(target->get_file(), NULL, &this->error))
				return false;
	}
	return true;
}

tc_file_with_infos*
tc_files_binomial :: get_from()
{
	tc_file_with_infos			*temp = new tc_file_with_infos(*this->from);
	return temp;
}

tc_file_with_infos*
tc_files_binomial :: get_to()
{
	tc_file_with_infos			*temp = new tc_file_with_infos(*this->to);
	return temp;
}

void
tc_files_binomial :: set_db_times(GTimeVal from_time, GTimeVal to_time)
{
	this->from->set_database_modification_time(from_time);
	this->to->set_database_modification_time(to_time);
}
