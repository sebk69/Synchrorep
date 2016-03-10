/*
 * window_card.h
 *
 *  Created on: 9 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  setup a synchronization
 *
 */

#ifndef WINDOW_CARD_H_
#define WINDOW_CARD_H_

#include "../accessors/ac_config.h"
#include "widget_behavior.h"

class window_card
{
private:
	widget_behavior				behavior;
	GtkWidget					*window;
	GtkWidget					*behavior_frame;
	GtkWidget					*group_entry;
	char						*new_group;
	bool						default_behavior_set;
	ac_config					*original_config;

	static void 				default_behavior_clicked(GtkWidget *checkbox, gpointer THISP);
	static void 				close_window(GtkWindow *window, gpointer THIS);

public:
								window_card(ac_config *config);
								~window_card();
};

#endif /* WINDOW_CARD_H_ */
