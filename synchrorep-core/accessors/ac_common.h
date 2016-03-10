/*
 * ac_common.h
 *
 *  Created on: 8 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  Common declarations for accessors
 */

#ifndef AC_COMMON_H_
#define AC_COMMON_H_

#include "db_access.h"

// list of possible alterations
typedef enum
{
	_ac_create,
	_ac_modified,
	_ac_deleted,
	_ac_unchanged,
	_ac_invalid
} alteration_type;

// common accessor
class ac_common
{
protected:
	alteration_type					alteration;
	bool							deletable;
	static db_access				*db;


public:
									ac_common();
									ac_common(const ac_common &from);

	alteration_type					get_alteration();
	bool							set_to_delete();
	bool							exists();
	void							modified();
	char*							get_error_msg();
};

#endif /* AC_COMMON_H_ */
