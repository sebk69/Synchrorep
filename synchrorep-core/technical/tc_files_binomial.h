/*
 * tc_file_binomial.h
 *
 *  Created on: 19 juil. 2009
 *      ©2009, 2010 - Sébastien Kus
 *  This source is under GNU/GLP V3 licence
 *
 *  Operations on binomial files
 */

#ifndef TC_FILE_BINOMIAL_H_
#define TC_FILE_BINOMIAL_H_

#include <gio/gio.h>

#include "../applicative/application.h"
#include "tc_file_with_infos.h"
#include "tc_thread_utils.h"

typedef enum _files_compare
{
	_files_not_modified,
	_from_modified,
	_to_modified,
	_from_deleted,
	_to_deleted,
	_both_deleted,
	_both_modified_from_recent,
	_both_modified_to_recent,
	_both_modified_equal,
	_comparison_error
} _files_compare;

typedef enum from_or_to_of_binome_type
{
	_binomial_from,
	_binomial_to
} from_or_to_of_binome_type;

class tc_files_binomial
{
private:
	tc_file_with_infos			*from;
	tc_file_with_infos			*to;


public:
	GError						*error;
	int							from_dif,
								to_dif;
								tc_files_binomial(tc_file_with_infos *pfrom, tc_file_with_infos *pto);
								~tc_files_binomial();

	_files_compare				md5sum_compare();
	_files_compare				sync_compare(bool md5sum_on_both_modified);
	_files_compare				copy_compare(bool md5sum_on_both_modified);

	// actions
	bool						copy(from_or_to_of_binome_type which, bool old_to_trash);
	bool						remove(from_or_to_of_binome_type which, bool put_to_trash, mutex_var<int> *decrement = NULL, mutex_var<bool> *cancel = NULL);
	tc_file_with_infos*			get_from();
	tc_file_with_infos*			get_to();
	void						set_db_times(GTimeVal from_time, GTimeVal to_time);
};

#endif /* TC_FILE_BINOMIAL_H_ */
