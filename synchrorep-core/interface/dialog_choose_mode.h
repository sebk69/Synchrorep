/*
 * dialog_choose_mode.h
 *
 *  Created on: 26 apr. 2011
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2011 - Sébastien Kus
 *
 *  User dialog to ask which mode (synchronization, copy)
 *
 */

#ifndef DELETE_CHOOSE_MODE_H_
#define DELETE_CHOOSE_MODE_H_

#include "../applicative/application.h"

typedef enum
{
	_mode_choose_sync,
	_mode_choose_copy,
	_mode_choose_cancel
} mode_choose;

class dialog_choose_mode
{
private:

public:
						dialog_choose_mode();
						~dialog_choose_mode();

	mode_choose			asking();
};

#endif /* DELETE_CHOOSE_MODE_H_ */
