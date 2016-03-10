/*
 * window_config_main.h
 *
 *  Created on: 4 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Main window of configuration :
 *  - let choose synchronization to configure
 *  - let choose configuring default behavior
 *  - let choose mode of use of behavior
 *
 */

#ifndef WINDOW_CONFIG_MAIN_H_
#define WINDOW_CONFIG_MAIN_H_

#include "../applicative/application.h"
#include "../accessors/ac_config.h"
#include "../accessors/ac_preferences.h"

class window_config_main
{
private:
	ac_config_list			list_of_configs;
	ac_preferences			prefs;
	GtkWidget				*window;
	GtkWidget				*activate_expert;
	GtkWidget				*activate_contextual;
	GtkWidget				*behavior_button;
	GtkWidget				*list_view;

	int						get_selected();
	void					remove_selected();

	static void				about_clicked(GtkButton *btn, gpointer null_data);
	static void				expert_toggled(GtkCheckButton *btn, gpointer THISP);
	static void				contextual_toggled(GtkCheckButton *btn, gpointer THISP);
	static void				create_launcher(GtkButton *btn, gpointer null_data);
	static void				delete_clicked(GtkButton *btn, gpointer THISP);
	static void				logs_clicked(GtkButton *btn, gpointer THISP);
	static void				modify_clicked(GtkButton *btn, gpointer THISP);
	static void				open_default_behavior(GtkButton *btn, gpointer null_data);
	static void				close_window(GtkWindow *window, gpointer null_data);

	void					refresh();

public:
							window_config_main();
							~window_config_main();
};

#endif /* WINDOW_CONFIG_MAIN_H_ */
