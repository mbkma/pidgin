/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <gnt.h>

#include "gntplugin.h"
#include "gntrequest.h"
#include "libfinch.h"

typedef struct
{
	FinchPluginPrefFrameCb pref_frame_cb;
} FinchPluginInfoPrivate;

enum
{
	PROP_0,
	PROP_GNT_PREF_FRAME_CB,
	PROP_LAST
};

static struct
{
	GntWidget *tree;
	GntWidget *window;
	GntWidget *aboot;
	GntWidget *conf;
} plugins;

typedef struct
{
	enum
	{
		FINCH_PLUGIN_UI_DATA_TYPE_WINDOW,
		FINCH_PLUGIN_UI_DATA_TYPE_REQUEST
	} type;

	union
	{
		GntWidget *window;
		gpointer request_handle;
	} u;
} FinchPluginUiData;

G_DEFINE_TYPE_WITH_PRIVATE(FinchPluginInfo, finch_plugin_info,
		PURPLE_TYPE_PLUGIN_INFO);

/* Set method for GObject properties */
static void
finch_plugin_info_set_property(GObject *obj, guint param_id, const GValue *value,
		GParamSpec *pspec)
{
	FinchPluginInfoPrivate *priv = finch_plugin_info_get_instance_private(
			FINCH_PLUGIN_INFO(obj));

	switch (param_id) {
		case PROP_GNT_PREF_FRAME_CB:
			priv->pref_frame_cb = g_value_get_pointer(value);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

/* Get method for GObject properties */
static void
finch_plugin_info_get_property(GObject *obj, guint param_id, GValue *value,
		GParamSpec *pspec)
{
	FinchPluginInfoPrivate *priv = finch_plugin_info_get_instance_private(
			FINCH_PLUGIN_INFO(obj));

	switch (param_id) {
		case PROP_GNT_PREF_FRAME_CB:
			g_value_set_pointer(value, priv->pref_frame_cb);
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
finch_plugin_info_init(G_GNUC_UNUSED FinchPluginInfo *info)
{
}

/* Class initializer function */
static void finch_plugin_info_class_init(FinchPluginInfoClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	/* Setup properties */
	obj_class->get_property = finch_plugin_info_get_property;
	obj_class->set_property = finch_plugin_info_set_property;

	g_object_class_install_property(obj_class, PROP_GNT_PREF_FRAME_CB,
		g_param_spec_pointer("gnt-pref-frame-cb",
		                     "GNT preferences frame callback",
		                     "Callback that returns a GNT preferences frame",
		                     G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY |
		                     G_PARAM_STATIC_STRINGS));
}

GPluginPluginInfo *
finch_plugin_info_new(const char *first_property, ...)
{
	GObject *info;
	va_list var_args;

	/* at least ID is required */
	if (!first_property)
		return NULL;

	va_start(var_args, first_property);
	info = g_object_new_valist(FINCH_TYPE_PLUGIN_INFO, first_property,
	                           var_args);
	va_end(var_args);

	return GPLUGIN_PLUGIN_INFO(info);
}

static gboolean
has_prefs(PurplePlugin *plugin)
{
	PurplePluginInfo *info = purple_plugin_get_info(plugin);
	FinchPluginInfoPrivate *priv = NULL;
	gboolean ret;

	g_return_val_if_fail(plugin != NULL, FALSE);

	if (!purple_plugin_is_loaded(plugin))
		return FALSE;

	if (FINCH_IS_PLUGIN_INFO(info))
		priv = finch_plugin_info_get_instance_private(
				FINCH_PLUGIN_INFO(info));

	ret = (priv && priv->pref_frame_cb);

	return ret;
}

static void
decide_conf_button(PurplePlugin *plugin)
{
	if (has_prefs(plugin))
		gnt_widget_set_visible(plugins.conf, TRUE);
	else
		gnt_widget_set_visible(plugins.conf, FALSE);

	gnt_box_readjust(GNT_BOX(plugins.window));
	gnt_widget_draw(plugins.window);
}

static void
finch_plugin_pref_close(PurplePlugin *plugin)
{
	PurplePluginInfo *info;
	FinchPluginUiData *ui_data;

	g_return_if_fail(plugin != NULL);

	info = purple_plugin_get_info(plugin);
	ui_data = g_object_get_data(G_OBJECT(info), "finch-ui-data");

	if (ui_data == NULL) {
		return;
	}

	if (ui_data->type == FINCH_PLUGIN_UI_DATA_TYPE_REQUEST) {
		purple_request_close(PURPLE_REQUEST_FIELDS,
			ui_data->u.request_handle);
		return;
	}

	g_return_if_fail(ui_data->type == FINCH_PLUGIN_UI_DATA_TYPE_WINDOW);

	gnt_widget_destroy(ui_data->u.window);

	g_object_set_data(G_OBJECT(info), "finch-ui-data", NULL);
}

static void
plugin_toggled_cb(GntWidget *tree, PurplePlugin *plugin,
                  G_GNUC_UNUSED gpointer data)
{
	GError *error = NULL;

	if (gnt_tree_get_choice(GNT_TREE(tree), plugin))
	{
		if (!purple_plugin_load(plugin, &error)) {
			purple_notify_error(NULL, _("ERROR"), _("loading plugin failed"), error->message, NULL);
			gnt_tree_set_choice(GNT_TREE(tree), plugin, FALSE);
			g_error_free(error);
		}
	}
	else
	{
		if (!purple_plugin_unload(plugin, &error)) {
			purple_notify_error(NULL, _("ERROR"), _("unloading plugin failed"), error->message, NULL);
			purple_plugin_disable(plugin);
			gnt_tree_set_choice(GNT_TREE(tree), plugin, TRUE);
			g_error_free(error);
		}

		finch_plugin_pref_close(plugin);
	}
	decide_conf_button(plugin);
	finch_plugins_save_loaded();
}

/* Xerox */
void
finch_plugins_save_loaded(void)
{
	purple_plugins_save_loaded("/finch/plugins/loaded");
}

static void
selection_changed(GntWidget *widget, G_GNUC_UNUSED gpointer old,
                  gpointer current, G_GNUC_UNUSED gpointer data)
{
	PurplePlugin *plugin = current;
	const gchar *filename;
	GPluginPluginInfo *info;
	char *text, *authors = NULL;
	const char * const *authorlist;
	GList *list = NULL, *iter = NULL;

	if (!plugin)
		return;

	filename = gplugin_plugin_get_filename(GPLUGIN_PLUGIN(plugin));
	info = GPLUGIN_PLUGIN_INFO(purple_plugin_get_info(plugin));
	authorlist = gplugin_plugin_info_get_authors(info);

	if (authorlist)
		authors = g_strjoinv(", ", (gchar **)authorlist);

	/* If the selected plugin was unseen before, mark it as seen. But save the list
	 * only when the plugin list is closed. So if the user enables a plugin, and it
	 * crashes, it won't get marked as seen so the user can fix the bug and still
	 * quickly find the plugin in the list.
	 * I probably mean 'plugin developers' by 'users' here. */
	list = g_object_get_data(G_OBJECT(widget), "seen-list");
	if (list) {
		iter = g_list_find_custom(list, filename, (GCompareFunc)strcmp);
	}
	if (!iter) {
		list = g_list_prepend(list, g_strdup(filename));
		g_object_set_data(G_OBJECT(widget), "seen-list", list);
	}

	/* XXX: Use formatting and stuff */
	gnt_text_view_clear(GNT_TEXT_VIEW(plugins.aboot));
	text = g_strdup_printf(
	        (authorlist && g_strv_length((gchar **)authorlist) > 1
	                 ? _("Name: %s\nVersion: %s\nDescription: %s\nAuthors: "
	                     "%s\nWebsite: %s\nFilename: %s\n")
	                 : _("Name: %s\nVersion: %s\nDescription: %s\nAuthor: "
	                     "%s\nWebsite: %s\nFilename: %s\n")),
	        _(gplugin_plugin_info_get_name(info)),
	        _(gplugin_plugin_info_get_version(info)),
	        _(gplugin_plugin_info_get_description(info)), authors,
	        _(gplugin_plugin_info_get_website(info)), filename);

	gnt_text_view_append_text_with_flags(GNT_TEXT_VIEW(plugins.aboot),
			text, GNT_TEXT_FLAG_NORMAL);
	gnt_text_view_scroll(GNT_TEXT_VIEW(plugins.aboot), 0);

	g_free(text);
	g_free(authors);

	decide_conf_button(plugin);
}

static void
reset_plugin_window(G_GNUC_UNUSED GntWidget *window,
                    G_GNUC_UNUSED gpointer data)
{
	GList *list = g_object_get_data(G_OBJECT(plugins.tree), "seen-list");
	purple_prefs_set_path_list("/finch/plugins/seen", list);
	g_list_free_full(list, g_free);

	plugins.window = NULL;
	plugins.tree = NULL;
	plugins.aboot = NULL;
}

static int
plugin_compare(PurplePlugin *p1, PurplePlugin *p2)
{
	char *s1 =
	        g_utf8_strup(gplugin_plugin_info_get_name(GPLUGIN_PLUGIN_INFO(
	                             purple_plugin_get_info(p1))),
	                     -1);
	char *s2 =
	        g_utf8_strup(gplugin_plugin_info_get_name(GPLUGIN_PLUGIN_INFO(
	                             purple_plugin_get_info(p2))),
	                     -1);
	int ret = g_utf8_collate(s1, s2);
	g_free(s1);
	g_free(s2);

	return ret;
}

static void
remove_confwin(G_GNUC_UNUSED GntWidget *window, gpointer _plugin)
{
	PurplePlugin *plugin = _plugin;
	PurplePluginInfo *info = purple_plugin_get_info(plugin);

	g_object_set_data(G_OBJECT(info), "finch-ui-data", NULL);
}

static void
configure_plugin_cb(G_GNUC_UNUSED GntWidget *button,
                    G_GNUC_UNUSED gpointer data)
{
	PurplePlugin *plugin;
	PurplePluginInfo *info;
	FinchPluginInfoPrivate *priv = NULL;
	FinchPluginUiData *ui_data;

	g_return_if_fail(plugins.tree != NULL);

	plugin = gnt_tree_get_selection_data(GNT_TREE(plugins.tree));
	if (!purple_plugin_is_loaded(plugin))
	{
		purple_notify_error(plugin, _("Error"),
			_("Plugin need to be loaded before you can configure it."), NULL, NULL);
		return;
	}

	info = purple_plugin_get_info(plugin);

	if (g_object_get_data(G_OBJECT(info), "finch-ui-data")) {
		return;
	}
	ui_data = g_new0(FinchPluginUiData, 1);
	g_object_set_data_full(G_OBJECT(info), "finch-ui-data", ui_data, g_free);

	if (FINCH_IS_PLUGIN_INFO(info))
		priv = finch_plugin_info_get_instance_private(
				FINCH_PLUGIN_INFO(info));

	if (priv && priv->pref_frame_cb != NULL)
	{
		GntWidget *window = gnt_vbox_new(FALSE);
		GntWidget *box, *button;

		gnt_box_set_toplevel(GNT_BOX(window), TRUE);
		gnt_box_set_title(GNT_BOX(window),
		                  gplugin_plugin_info_get_name(
		                          GPLUGIN_PLUGIN_INFO(info)));
		gnt_box_set_alignment(GNT_BOX(window), GNT_ALIGN_MID);

		box = priv->pref_frame_cb();
		gnt_box_add_widget(GNT_BOX(window), box);

		box = gnt_hbox_new(FALSE);
		gnt_box_add_widget(GNT_BOX(window), box);

		button = gnt_button_new(_("Close"));
		gnt_box_add_widget(GNT_BOX(box), button);
		g_signal_connect_swapped(G_OBJECT(button), "activate",
				G_CALLBACK(gnt_widget_destroy), window);
		g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(remove_confwin), plugin);

		gnt_widget_show(window);

		ui_data->type = FINCH_PLUGIN_UI_DATA_TYPE_WINDOW;
		ui_data->u.window = window;
	}
	else
	{
		purple_notify_info(plugin, _("Error"), _("No configuration "
			"options for this plugin."), NULL, NULL);
		g_object_set_data(G_OBJECT(info), "finch-ui-data", NULL);
	}
}

void finch_plugins_show_all(void)
{
	GntWidget *window, *tree, *box, *aboot, *button;
	GList *plugin_list, *iter;
	GList *seen;

	if (plugins.window) {
		gnt_window_present(plugins.window);
		return;
	}

	purple_plugins_refresh();

	plugins.window = window = gnt_vbox_new(FALSE);
	gnt_box_set_toplevel(GNT_BOX(window), TRUE);
	gnt_box_set_title(GNT_BOX(window), _("Plugins"));
	gnt_box_set_pad(GNT_BOX(window), 0);
	gnt_box_set_alignment(GNT_BOX(window), GNT_ALIGN_MID);

	gnt_box_add_widget(GNT_BOX(window),
			gnt_label_new(_("You can (un)load plugins from the following list.")));
	gnt_box_add_widget(GNT_BOX(window), gnt_hline_new());

	box = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(window), box);
	gnt_box_add_widget(GNT_BOX(window), gnt_hline_new());

	gnt_box_set_pad(GNT_BOX(box), 0);
	plugins.tree = tree = gnt_tree_new();
	gnt_tree_set_compare_func(GNT_TREE(tree), (GCompareFunc)plugin_compare);
	gnt_widget_set_has_border(tree, FALSE);
	gnt_box_add_widget(GNT_BOX(box), tree);
	gnt_box_add_widget(GNT_BOX(box), gnt_vline_new());

	plugins.aboot = aboot = gnt_text_view_new();
	gnt_text_view_set_flag(GNT_TEXT_VIEW(aboot), GNT_TEXT_VIEW_TOP_ALIGN);
	gnt_widget_set_size(aboot, 40, 20);
	gnt_box_add_widget(GNT_BOX(box), aboot);

	seen = purple_prefs_get_path_list("/finch/plugins/seen");

	plugin_list = purple_plugins_find_all();
	for (iter = plugin_list; iter; iter = iter->next)
	{
		PurplePlugin *plug = PURPLE_PLUGIN(iter->data);

		if (purple_plugin_is_internal(plug))
			continue;

		gnt_tree_add_choice(
		        GNT_TREE(tree), plug,
		        gnt_tree_create_row(
		                GNT_TREE(tree),
		                gplugin_plugin_info_get_name(
		                        GPLUGIN_PLUGIN_INFO(
		                                purple_plugin_get_info(plug)))),
		        NULL, NULL);
		gnt_tree_set_choice(GNT_TREE(tree), plug, purple_plugin_is_loaded(plug));
		if (!g_list_find_custom(seen, gplugin_plugin_get_filename(plug),
		                        (GCompareFunc)strcmp)) {
			gnt_tree_set_row_flags(GNT_TREE(tree), plug, GNT_TEXT_FLAG_BOLD);
		}
	}
	g_list_free(plugin_list);

	gnt_tree_set_col_width(GNT_TREE(tree), 0, 30);
	g_signal_connect(G_OBJECT(tree), "toggled", G_CALLBACK(plugin_toggled_cb), NULL);
	g_signal_connect(G_OBJECT(tree), "selection_changed", G_CALLBACK(selection_changed), NULL);
	g_object_set_data(G_OBJECT(tree), "seen-list", seen);

	box = gnt_hbox_new(FALSE);
	gnt_box_add_widget(GNT_BOX(window), box);

	button = gnt_button_new(_("Close"));
	gnt_box_add_widget(GNT_BOX(box), button);
	g_signal_connect_swapped(G_OBJECT(button), "activate",
			G_CALLBACK(gnt_widget_destroy), window);

	plugins.conf = button = gnt_button_new(_("Configure Plugin"));
	gnt_box_add_widget(GNT_BOX(box), button);
	g_signal_connect(G_OBJECT(button), "activate", G_CALLBACK(configure_plugin_cb), NULL);

	g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(reset_plugin_window), NULL);

	gnt_widget_show(window);

	decide_conf_button(gnt_tree_get_selection_data(GNT_TREE(tree)));
}

#if 0
static void
free_stringlist(GList *list) {
	g_list_free_full(list, g_free);
}

static GntWidget*
process_pref_frame(PurplePluginPrefFrame *frame)
{
	PurpleRequestField *field;
	PurpleRequestPage *page;
	PurpleRequestGroup *group = NULL;
	GList *prefs;
	GList *stringlist = NULL;
	GntWidget *ret = NULL;

	page = purple_request_page_new();

	for (prefs = purple_plugin_pref_frame_get_prefs(frame); prefs; prefs = prefs->next) {
		PurplePluginPref *pref = prefs->data;
		PurplePrefType type;
		const char *name = purple_plugin_pref_get_name(pref);
		const char *label = purple_plugin_pref_get_label(pref);
		if(name == NULL) {
			if(label == NULL)
				continue;

			if(purple_plugin_pref_get_pref_type(pref) == PURPLE_PLUGIN_PREF_INFO) {
				field = purple_request_field_label_new("*", purple_plugin_pref_get_label(pref));
				purple_request_group_add_field(group, field);
			} else {
				group = purple_request_group_new(label);
				purple_request_page_add_group(page, group);
			}
			continue;
		}

		field = NULL;
		type = purple_prefs_get_pref_type(name);
		if(purple_plugin_pref_get_pref_type(pref) == PURPLE_PLUGIN_PREF_CHOICE) {
			gpointer current_value = NULL;

			switch(type) {
				case PURPLE_PREF_BOOLEAN:
					current_value = g_strdup_printf("%d", (int)purple_prefs_get_bool(name));
					break;
				case PURPLE_PREF_INT:
					current_value = g_strdup_printf("%d", (int)purple_prefs_get_int(name));
					break;
				case PURPLE_PREF_STRING:
					current_value = g_strdup(purple_prefs_get_string(name));
					break;
				default:
					continue;
			}

			field = purple_request_field_list_new(name, label);
			purple_request_field_list_set_multi_select(PURPLE_REQUEST_FIELD_LIST(field),
			                                           FALSE);
			for (GList *list = purple_plugin_pref_get_choices(pref); list != NULL; list = list->next) {
				const PurpleKeyValuePair *choice = list->data;
				char *value = NULL;
				switch(type) {
					case PURPLE_PREF_BOOLEAN:
						value = g_strdup_printf("%d", GPOINTER_TO_INT(choice->value));
						break;
					case PURPLE_PREF_INT:
						value = g_strdup_printf("%d", GPOINTER_TO_INT(choice->value));
						break;
					case PURPLE_PREF_STRING:
						value = g_strdup(choice->value);
						break;
					default:
						break;
				}
				stringlist = g_list_prepend(stringlist, value);
				purple_request_field_list_add_icon(PURPLE_REQUEST_FIELD_LIST(field),
				                                   choice->key, NULL, value);
				if(purple_strequal(value, current_value)) {
					purple_request_field_list_add_selected(PURPLE_REQUEST_FIELD_LIST(field),
					                                       choice->key);
				}
			}
			g_free(current_value);
		} else {
			switch(type) {
				case PURPLE_PREF_BOOLEAN:
					field = purple_request_field_bool_new(name, label, purple_prefs_get_bool(name));
					break;
				case PURPLE_PREF_INT:
					field = purple_request_field_int_new(name, label, purple_prefs_get_int(name), INT_MIN, INT_MAX);
					break;
				case PURPLE_PREF_STRING:
					field = purple_request_field_string_new(name, label, purple_prefs_get_string(name),
							purple_plugin_pref_get_format_type(pref) & PURPLE_STRING_FORMAT_TYPE_MULTILINE);
					break;
				default:
					break;
			}
		}

		if (field) {
			if (group == NULL) {
				group = purple_request_group_new(_("Preferences"));
				purple_request_page_add_group(page, group);
			}
			purple_request_group_add_field(group, field);
		}
	}

	ret = purple_request_fields(NULL, _("Preferences"), NULL, NULL, page,
			_("Save"), G_CALLBACK(finch_request_save_in_prefs), _("Cancel"), NULL,
			NULL, NULL);
	g_signal_connect_swapped(G_OBJECT(ret), "destroy", G_CALLBACK(free_stringlist), stringlist);
	return ret;
}
#endif
