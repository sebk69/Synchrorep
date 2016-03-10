/*
 * plugin.c
 *
 *  Created on: 8 janv. 2010
 *      This file is a part of synchrorep under GPL V3 licence
 *      ©2010 - Sébastien Kus
 *
 *  Miscellaneous utilities
 *
 */

#include <glib.h>
/* Nautilus extension headers */
#include <libnautilus-extension/nautilus-extension-types.h>
#include <libnautilus-extension/nautilus-file-info.h>
#include <libnautilus-extension/nautilus-info-provider.h>
#include <libnautilus-extension/nautilus-menu-provider.h>
#include <libnautilus-extension/nautilus-property-page-provider.h>

#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <libintl.h>

static GType provider_types[1];
static GType synchrorep_extension_type;
static GObjectClass *parent_class;

typedef struct {
	GObject parent_slot;
} SynchrorepExtension;

typedef struct {
	GObjectClass parent_slot;
} SynchrorepExtensionClass;

/* nautilus extension interface */
void nautilus_module_initialize (GTypeModule  *module);
void nautilus_module_shutdown (void);
void nautilus_module_list_types (const GType **types, int *num_types);
GType synchrorep_get_type (void);

static void synchrorep_extension_register_type (GTypeModule *module);

/* menu filler */
static GList * synchrorep_extension_get_file_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                GList *files);
/*static GList * synchrorep_extension_get_background_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                NautilusFileInfo *current_folder);*/

/* commands callback */
static void launch_synchrorep (NautilusMenuItem *item, gpointer user_data);
static void launch_synchrorep_group (NautilusMenuItem *item,
                         gpointer user_data);

void nautilus_module_initialize (GTypeModule  *module)
{
        synchrorep_extension_register_type (module);

        provider_types[0] = synchrorep_get_type ();
}

void nautilus_module_shutdown (void)
{
        /* Any module-specific shutdown */
}

void nautilus_module_list_types (const GType **types,
                                 int *num_types)
{
        *types = provider_types;
        *num_types = G_N_ELEMENTS (provider_types);
}

GType synchrorep_get_type (void)
{
        return synchrorep_extension_type;
}

static void synchrorep_extension_instance_init (SynchrorepExtension *object)
{
}

static void synchrorep_extension_class_init(SynchrorepExtensionClass *class)
{
	parent_class = g_type_class_peek_parent (class);
}

static void synchrorep_extension_menu_provider_iface_init(
		NautilusMenuProviderIface *iface)
{
	iface->get_file_items = synchrorep_extension_get_file_items;
}

static void synchrorep_extension_register_type (GTypeModule *module)
{
        static const GTypeInfo info = {
                sizeof (SynchrorepExtensionClass),
                (GBaseInitFunc) NULL,
                (GBaseFinalizeFunc) NULL,
                (GClassInitFunc) synchrorep_extension_class_init,
                NULL,
                NULL,
                sizeof (SynchrorepExtension),
                0,
                (GInstanceInitFunc) synchrorep_extension_instance_init,
        };

	static const GInterfaceInfo menu_provider_iface_info = {
		(GInterfaceInitFunc) synchrorep_extension_menu_provider_iface_init,
		NULL,
		NULL
	};

       synchrorep_extension_type = g_type_module_register_type (module,
                             G_TYPE_OBJECT,
                             "synchrorep",
                             &info, 0);

	g_type_module_add_interface (module,
				     synchrorep_extension_type,
				     NAUTILUS_TYPE_MENU_PROVIDER,
				     &menu_provider_iface_info);
}


static GList * synchrorep_extension_get_file_items (NautilusMenuProvider *provider,
                GtkWidget *window,
                GList *files)
{
	NautilusMenuItem *item;
	GList *l;
	GList *ret;

	bindtextdomain ("synchrorep", "/usr/share/locale");
	textdomain ("synchrorep");

	// test if activated
	// set gfile for contextual menu configuration
	char				*name;
	char				*home_dir = strdup(g_get_home_dir());
	char				file_name[500];
	char				**groups = malloc(sizeof(char*) * 100);
	int					igroup = 0;

	strcpy(file_name, home_dir);
	strcat(file_name, "/.synchrorep.menu");
	GFile				*contextual_menu_file_param = g_file_new_for_commandline_arg(file_name);
	free(home_dir);
	if(!g_file_query_exists(contextual_menu_file_param, NULL))
		return NULL;

	groups[0] = NULL;

	for (l = files; l != NULL; l = l->next)
	{
		if(!nautilus_file_info_is_directory(NAUTILUS_FILE_INFO (l->data)))
			return NULL;

		// get group name (if so)
    	NautilusFileInfo *file = NAUTILUS_FILE_INFO(l->data);
        name = nautilus_file_info_get_uri (file);
        char				cmd[1500];
        gchar				*output;
        gchar				*outerr;
        int					status;

        strcpy(cmd, "synchrorep --get-group ");
        strcat(cmd, name);
		g_spawn_command_line_sync(cmd, &output, &outerr, &status, NULL);
		if(outerr[0] == '\0')
		{
			output[strlen(output)-1] = '\0';
			groups[igroup] = strdup(output);
			groups[igroup + 1] = NULL;
			igroup++;
		}
	}

	// add synchronize folder menu
	item = nautilus_menu_item_new ("synchrorep::synchronize",
								   gettext("Synchronize..."),
								   gettext("Synchronize folders with Synchrorep"),
								   "synchrorep");
	g_signal_connect (item, "activate", G_CALLBACK (launch_synchrorep), provider);
	g_object_set_data_full ((GObject*) item, "synchrorep_extension_files",
							nautilus_file_info_list_copy (files),
							(GDestroyNotify)nautilus_file_info_list_free);
	ret = g_list_append (NULL, item);

	// add groups menu
	for(igroup=0; groups[igroup] != NULL; igroup++)
	{
		char				menu_id[150];
		char				menu_label[150];
		strcpy(menu_id, "synchrorep::synchrorep_group_");
		strcat(menu_id, groups[igroup]);

		strcpy(menu_label, gettext("Synchronize group "));
		strcat(menu_label, groups[igroup]);
		strcat(menu_label, "...");

		item = nautilus_menu_item_new(menu_id, menu_label, gettext("Synchronize all group content"), "synchrorep.group");
		g_signal_connect (item, "activate", G_CALLBACK (launch_synchrorep_group), groups[igroup]);
		g_object_set_data_full ((GObject*) item, menu_id,
								groups[igroup],
								(GDestroyNotify)free);
		ret = g_list_append (ret, item);
	}

	return ret;
}

static void launch_synchrorep (NautilusMenuItem *item,
                         gpointer user_data)
{
        GList 			*files;
        GList 			*l;
        GString			*cmd;
    	char			*name;

        files = g_object_get_data ((GObject *) item, "synchrorep_extension_files");

        for (l = files; l != NULL; l = l->next)
        {
        	cmd = g_string_new("synchrorep --execute \"");
        	NautilusFileInfo *file = NAUTILUS_FILE_INFO(l->data);
            name = nautilus_file_info_get_uri (file);
    		g_string_append_printf (cmd, "%s\"", name);
            g_spawn_command_line_async(cmd->str, NULL);
            g_string_free (cmd, TRUE);
        }
}

static void launch_synchrorep_group (NautilusMenuItem *item,
                         gpointer user_data)
{
	char			cmd[1000];
	strcpy(cmd, "synchrorep --execute-group ");
	strcat(cmd, (char*)user_data);
	g_spawn_command_line_async(cmd, NULL);
}

