/*
 * window_ask_at_the_end.h
 *
 *  Created on: 30 oct. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  show window of pending actions
 *
 */

#ifndef WINDOW_ASK_AT_THE_END_H_
#define WINDOW_ASK_AT_THE_END_H_

#include "widget_ask_at_the_end.h"

class window_ask_at_the_end
{
private:
	widget_ask_at_the_end		ask_at_the_end_widget;
	GtkWidget					*window;

	static void 				close_window(GtkWindow *window, gpointer null_data);

public:
								window_ask_at_the_end(int config_id, time_t launch_time);
								~window_ask_at_the_end();
};

#endif /* WINDOW_ASK_AT_THE_END_H_ */
