/*
 * Pidgin - Transparency plugin
 *
 * Copyright (C) 1998-2002, Rob Flynn <rob@marko.net>
 * Copyright (C) 2002-2003, Herman Bloggs <hermanator12002@yahoo.com>
 * Copyright (C) 2005,      Daniel Atallah <daniel_atallah@yahoo.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02111-1301, USA.
 *
 */

#include <glib.h>
#include <glib/gi18n-lib.h>

#include <purple.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <pidgin.h>

/*
 *  MACROS & DEFINES
 */
/* The plugin name is left unchanged from its WinAPI days in order to keep it
 * loading for users who were using it. */
#define WINTRANS_PLUGIN_ID	"gtk-win-trans"

/* Key used to save GtkEventControllerFocus for this plugin. */
#define WINTRANS_CONTROLLER_KEY (WINTRANS_PLUGIN_ID "-focus-controller")

/* Key used to save GtkSlider for this plugin. */
#define WINTRANS_SLIDER_KEY (WINTRANS_PLUGIN_ID "-slider")

/*
 *  LOCALS
 */
#define OPT_SCHEMA "im.pidgin.Pidgin.plugin.Transparency"
#define OPT_WINTRANS_ALPHA "alpha"
#define OPT_WINTRANS_SLIDER "slider"
#define OPT_WINTRANS_ONFOCUS "solid-on-focus"

/*
 *  CODE
 */

/* Set window transparency level */
static void
set_wintrans(GtkWidget *window, int alpha, gboolean enabled)
{
	if (enabled) {
		gtk_widget_set_opacity(window, alpha / 255.0);
	} else {
		gtk_widget_set_opacity(window, 1);
	}

	/* Changing from opaque to partially transparent seems to need some kind of
	 * structural refresh. Unfortunately, a simple `gtk_widget_queue_draw` is
	 * not sufficient, so we need to do this instead. */
	gtk_widget_queue_resize(window);
}

/* When a conv window is focused, if we're only transparent when unfocused,
 * deal with transparency */
static void
focus_conv_win_cb(GtkEventControllerFocus *self, gpointer data) {
	GtkWidget *window = NULL;
	gboolean enter = GPOINTER_TO_INT(data);
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(OPT_SCHEMA,
	                                       purple_core_get_settings_backend());

	if(!g_settings_get_boolean(settings, OPT_WINTRANS_ONFOCUS)) {
		g_object_unref(settings);
		return;
	}

	window = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(self));
	set_wintrans(window, g_settings_get_int(settings, OPT_WINTRANS_ALPHA),
	             !enter);

	g_object_unref(settings);
}

static void
add_focus_controller_to_conv_win(GtkWidget *window) {
	GtkEventController *focus = NULL;

	focus = gtk_event_controller_focus_new();
	g_signal_connect(focus, "enter", G_CALLBACK(focus_conv_win_cb),
	                 GINT_TO_POINTER(TRUE));
	g_signal_connect(focus, "leave", G_CALLBACK(focus_conv_win_cb),
	                 GINT_TO_POINTER(FALSE));

	gtk_widget_add_controller(window, focus);
	g_object_set_data(G_OBJECT(window), WINTRANS_CONTROLLER_KEY, focus);
}

static void
remove_focus_controller_from_conv_win(GtkWidget *window) {
	GtkEventController *focus = NULL;

	focus = g_object_get_data(G_OBJECT(window), WINTRANS_CONTROLLER_KEY);
	if(GTK_IS_EVENT_CONTROLLER_FOCUS(focus)) {
		gtk_widget_remove_controller(window, focus);
	}
	g_object_set_data(G_OBJECT(window), WINTRANS_CONTROLLER_KEY, NULL);
}

static void change_alpha(GtkWidget *w, gpointer data) {
	int alpha = gtk_range_get_value(GTK_RANGE(w));
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(OPT_SCHEMA,
	                                       purple_core_get_settings_backend());

	g_settings_set_int(settings, OPT_WINTRANS_ALPHA, alpha);

	/* If we're in no-transparency on focus mode,
	 * don't take effect immediately */
	if(!g_settings_get_boolean(settings, OPT_WINTRANS_ONFOCUS)) {
		set_wintrans(GTK_WIDGET(data), alpha, TRUE);
	}

	g_object_unref(settings);
}

/* Clean up transparency stuff for the conv window */
static void
conversation_delete_cb(G_GNUC_UNUSED GtkApplication *application,
                       GtkWindow *window, G_GNUC_UNUSED gpointer data)
{
	if(!PIDGIN_IS_DISPLAY_WINDOW(window)) {
		return;
	}

	purple_debug_info(WINTRANS_PLUGIN_ID,
	                  "Conv window destroyed... removing from list");

	g_object_set_data(G_OBJECT(window), WINTRANS_SLIDER_KEY, NULL);

	/* Remove the focus cbs */
	remove_focus_controller_from_conv_win(GTK_WIDGET(window));
}

static void
remove_slider(GtkWidget *slider_frame) {
	gtk_widget_unparent(slider_frame);
}

static void
update_slider(GSettings *settings, gchar *key, gpointer data)
{
	GtkWidget *slider = data;
	gint alpha = 255;

	alpha = g_settings_get_int(settings, key);
	gtk_range_set_value(GTK_RANGE(slider), alpha);
}

static void add_slider(GtkWidget *win) {
	GtkWidget *vbox = NULL;
	GtkWidget *slider_frame;
	GtkWidget *hbox;
	GtkWidget *label, *slider;
	gint imalpha = 255;
	GSettings *settings = NULL;

	/* Look up this window to see if it already has a slider */
	if (g_object_get_data(G_OBJECT(win), WINTRANS_SLIDER_KEY) != NULL) {
		return;
	}

	vbox = gtk_widget_get_first_child(win);
	while(vbox != NULL && !GTK_IS_BOX(vbox)) {
		vbox = gtk_widget_get_next_sibling(vbox);
	}

	if(vbox == NULL) {
		purple_debug_error(WINTRANS_PLUGIN_ID, "no vbox found");
		return;
	}

	settings = g_settings_new_with_backend(OPT_SCHEMA,
	                                       purple_core_get_settings_backend());

	slider_frame = gtk_frame_new(NULL);
	gtk_widget_set_margin_start(slider_frame, 6);
	gtk_widget_set_margin_end(slider_frame, 6);
	gtk_box_prepend(GTK_BOX(vbox), slider_frame);

	g_settings_bind(settings, OPT_WINTRANS_SLIDER, slider_frame, "visible",
	                G_SETTINGS_BIND_DEFAULT);

	hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
	gtk_frame_set_child(GTK_FRAME(slider_frame), hbox);

	label = gtk_label_new(_("Opacity:"));
	gtk_box_append(GTK_BOX(hbox), label);

	slider = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 50, 255, 1);
	gtk_widget_set_hexpand(slider, TRUE);
	gtk_box_append(GTK_BOX(hbox), slider);

	imalpha = g_settings_get_int(settings, OPT_WINTRANS_ALPHA);
	gtk_range_set_value(GTK_RANGE(slider), imalpha);
	/* On slider val change, update window's transparency level */
	g_signal_connect(G_OBJECT(slider), "value-changed",
	                 G_CALLBACK(change_alpha), win);

	/* We cannot use g_settings_bind because GtkScale has no value property. */
	g_signal_connect_object(settings, "changed::" OPT_WINTRANS_ALPHA,
	                        G_CALLBACK(update_slider), slider, 0);

	/* Set the initial transparency level */
	set_wintrans(win, g_settings_get_int(settings, OPT_WINTRANS_ALPHA),
	             TRUE);

	/* Set window data, to track that it has a slider */
	g_object_set_data_full(G_OBJECT(win), WINTRANS_SLIDER_KEY, slider_frame,
	                       (GDestroyNotify)remove_slider);

	g_object_unref(settings);
}

/* Remove all transparency related aspects from conversation windows */
static void remove_convs_wintrans(gboolean remove_signal) {
	GApplication *application = NULL;
	GList *wins;

	application = g_application_get_default();
	wins = gtk_application_get_windows(GTK_APPLICATION(application));

	for(; wins; wins = wins->next) {
		GtkWidget *window = wins->data;

		if(!PIDGIN_IS_DISPLAY_WINDOW(window)) {
			continue;
		}

		set_wintrans(window, 0, FALSE);

		/* Remove the focus cbs */
		if (remove_signal) {
			remove_focus_controller_from_conv_win(window);
		}

		g_object_set_data(G_OBJECT(window), WINTRANS_SLIDER_KEY, NULL);
	}
}

static void
set_conv_window_trans(GtkWidget *window) {
	GSettings *settings = NULL;

	settings = g_settings_new_with_backend(OPT_SCHEMA,
	                                       purple_core_get_settings_backend());

	set_wintrans(window, g_settings_get_int(settings, OPT_WINTRANS_ALPHA),
	             TRUE);

	add_slider(window);

	g_object_unref(settings);
}

static void
new_conversation_cb(G_GNUC_UNUSED GtkApplication *application,
                    GtkWindow *window, G_GNUC_UNUSED gpointer data)
{
	if(!PIDGIN_IS_DISPLAY_WINDOW(window)) {
		return;
	}

	set_conv_window_trans(GTK_WIDGET(window));
	add_focus_controller_to_conv_win(GTK_WIDGET(window));
}

static void
update_existing_convs(void) {
	GApplication *application = NULL;
	GList *wins;

	application = g_application_get_default();
	wins = gtk_application_get_windows(GTK_APPLICATION(application));

	for(; wins; wins = wins->next) {
		GtkWidget *window = wins->data;

		if(!PIDGIN_IS_DISPLAY_WINDOW(window)) {
			continue;
		}

		set_conv_window_trans(window);

		add_focus_controller_to_conv_win(window);
	}
}

static GPluginPluginInfo *
transparency_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Pidgin Developers <devel@pidgin.im>",
		NULL
	};

	return pidgin_plugin_info_new(
		"id", WINTRANS_PLUGIN_ID,
		"name", N_("Transparency"),
		"version", DISPLAY_VERSION,
		"category", N_("User interface"),
		"summary", N_("Variable Transparency for the buddy list and conversations."),
		"description", N_("This plugin enables variable alpha transparency on conversation windows and the buddy list."),
		"authors", authors,
		"website", PURPLE_WEBSITE,
		"abi-version", PURPLE_ABI_VERSION,
		"settings-schema", OPT_SCHEMA,
		NULL
	);
}

static gboolean
transparency_load(G_GNUC_UNUSED GPluginPlugin *plugin,
                  G_GNUC_UNUSED GError **error)
{
	GApplication *application = NULL;

	application = g_application_get_default();
	g_signal_connect(application, "window-added",
	                 G_CALLBACK(new_conversation_cb), NULL);
	g_signal_connect(application, "window-removed",
	                 G_CALLBACK(conversation_delete_cb), NULL);

	update_existing_convs();

	return TRUE;
}

static gboolean
transparency_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                    G_GNUC_UNUSED gboolean shutdown,
                    G_GNUC_UNUSED GError **error)
{
	purple_debug_info(WINTRANS_PLUGIN_ID, "Unloading transparency plugin\n");

	remove_convs_wintrans(TRUE);

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(transparency)

