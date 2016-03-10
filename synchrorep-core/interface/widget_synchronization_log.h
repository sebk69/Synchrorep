/*
 * widget_synchronization_log.h
 *
 *  Created on: 19 mars 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *      generic widget to show content of a log for a synchronization
 *
 */

#ifndef WIDGET_SYNCHRONIZATION_LOG_H_
#define WIDGET_SYNCHRONIZATION_LOG_H_

#include "../applicative/application.h"

#include "time.h"

class widget_synchronization_log
{
private:
	int						config_id;
	time_t					time_of_launch;
	GtkWidget				*scroll_box;

public:
	GtkWidget				*widget;

							widget_synchronization_log(int config_id, time_t time_of_launch);
							~widget_synchronization_log();

	void					build();
};

#endif /* WIDGET_SYNCHRONIZATION_LOG_H_ */
