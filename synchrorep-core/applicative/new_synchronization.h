/*
 * new_synchronization.h
 *
 *  Created on: 13 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  configuration creator
 *
 */

#ifndef NEW_SYNCHRONIZATION_H_
#define NEW_SYNCHRONIZATION_H_

#include "../accessors/ac_config.h"

class new_synchronization
{
private:
	bool			config_created_correctly;

	void			exists_dialog(char *folder);
	void			not_a_folder_dialog(char *folder);
	void			cant_mount(char *folder);
public:
					new_synchronization(ac_config *new_config);
					~new_synchronization();

	bool			get_config_ok();
};

#endif /* NEW_SYNCHRONIZATION_H_ */
