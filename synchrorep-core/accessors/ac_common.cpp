/*
 * ac_common.cpp
 *
 *  Created on: 8 juil. 2009
 *
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2009 - Sébastien Kus
 *
 *  Common declarations for accessors
 */

#include "ac_common.h"

// connexion
db_access*				ac_common :: db = NULL;

// constructor
ac_common :: ac_common()
{
	// by default, object is unchanged
	this->alteration = _ac_unchanged;

	// create connection if not exists
	if(this->db == NULL)
		this->db = new db_access;

	// by default, object is delatable
	this->deletable = true;
}

// duplication constructor
ac_common :: ac_common(const ac_common &from)
{
	this->alteration = from.alteration;
	this->deletable = from.deletable;
}

// alteration is read-only for public
alteration_type
ac_common :: get_alteration()
{
	return this->alteration;
}

// set object to be deleted on next commit
bool
ac_common :: set_to_delete()
{
	if(this->deletable)
	{
		this->alteration = _ac_deleted;
		return true;
	}

	return false;
}

// is object exists in database ? (even if not commited again)
bool
ac_common :: exists()
{
	return (this->alteration != _ac_create && this->alteration != _ac_deleted);
}

// set alteration when object is modified
void
ac_common :: modified()
{
	switch(this->alteration)
	{
		// unchanged alterations
		case _ac_create:
		case _ac_modified:
		case _ac_deleted:
		case _ac_invalid:
		break;

		case _ac_unchanged:
			this->alteration = _ac_modified;
		break;
	}
}
