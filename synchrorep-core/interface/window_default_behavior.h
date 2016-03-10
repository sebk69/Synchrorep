/*
 * window_default_behavior.h
 *
 *  Created on: 7 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  setup default behavior
 *
 */

#ifndef WINDOW_DEFAULT_BEHAVIOR_H_
#define WINDOW_DEFAULT_BEHAVIOR_H_

#include "widget_behavior.h"

class window_default_behavior
{
private:
	widget_behavior				behavior;
	GtkWidget					*window;

	static void 				close_window(GtkWidget *widget, gpointer window);

public:
								window_default_behavior();
								~window_default_behavior();
};

#endif /* WINDOW_DEFAULT_BEHAVIOR_H_ */
