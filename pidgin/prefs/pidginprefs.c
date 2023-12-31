/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <errno.h>
#include <math.h>

#include <glib/gi18n-lib.h>
#include <glib/gstdio.h>

#include <purple.h>

#include "pidginprefs.h"
#include "pidginprefsinternal.h"
#include "gtkutils.h"
#include "pidgincore.h"
#include "pidginvvprefs.h"

struct _PidginPrefsWindow {
	GtkDialog parent;
};

G_DEFINE_TYPE(PidginPrefsWindow, pidgin_prefs_window, GTK_TYPE_DIALOG);

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
update_spin_value(G_GNUC_UNUSED GtkWidget *w, GtkWidget *spin)
{
	const char *key = g_object_get_data(G_OBJECT(spin), "val");
	int value;

	value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));

	purple_prefs_set_int(key, value);
}

void
pidgin_prefs_bind_spin_button(const char *key, GtkWidget *spin)
{
	GtkAdjustment *adjust;
	int val;

	val = purple_prefs_get_int(key);

	adjust = gtk_spin_button_get_adjustment(GTK_SPIN_BUTTON(spin));
	gtk_adjustment_set_value(adjust, val);
	g_object_set_data(G_OBJECT(spin), "val", (char *)key);
	g_signal_connect(G_OBJECT(adjust), "value-changed",
			G_CALLBACK(update_spin_value), GTK_WIDGET(spin));
}

static void
entry_set(GtkEntry *entry, gpointer data)
{
	const char *key = (const char*)data;

	purple_prefs_set_string(key, gtk_editable_get_text(GTK_EDITABLE(entry)));
}

void
pidgin_prefs_bind_entry(const char *key, GtkWidget *entry)
{
	const gchar *value;

	value = purple_prefs_get_string(key);

	gtk_editable_set_text(GTK_EDITABLE(entry), value);
	g_signal_connect(G_OBJECT(entry), "changed", G_CALLBACK(entry_set),
			(char*)key);
}

static void
bind_combo_row_set(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                   gpointer data)
{
	const gchar *key = data;
	GtkStringObject *item = NULL;
	const gchar *value = NULL;

	item = adw_combo_row_get_selected_item(ADW_COMBO_ROW(obj));
	value = gtk_string_object_get_string(item);
	purple_prefs_set_string(key, value);
}

void
pidgin_prefs_bind_combo_row(const gchar *key, GtkWidget *widget) {
	GtkStringList *model = NULL;
	const char *pref_value = NULL;
	guint selected = GTK_INVALID_LIST_POSITION;

	pref_value = purple_prefs_get_string(key);
	model = GTK_STRING_LIST(adw_combo_row_get_model(ADW_COMBO_ROW(widget)));

	for(guint i = 0; i < g_list_model_get_n_items(G_LIST_MODEL(model)); i++) {
		const char *value = gtk_string_list_get_string(model, i);

		if (purple_strequal(pref_value, value)) {
			selected = i;
			break;
		}
	}

	adw_combo_row_set_selected(ADW_COMBO_ROW(widget), selected);

	g_signal_connect(widget, "notify::selected",
	                 G_CALLBACK(bind_combo_row_set), (gpointer)key);
}

static void
set_bool_pref(GtkWidget *w, const char *key)
{
	purple_prefs_set_bool(key,
			gtk_check_button_get_active(GTK_CHECK_BUTTON(w)));
}

void
pidgin_prefs_bind_checkbox(const char *key, GtkWidget *button)
{
	gtk_check_button_set_active(GTK_CHECK_BUTTON(button),
			purple_prefs_get_bool(key));
	g_signal_connect(G_OBJECT(button), "toggled",
			G_CALLBACK(set_bool_pref), (char *)key);
}

static void
set_bool_switch_pref(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                     gpointer data)
{
	const gchar *key = data;

	purple_prefs_set_bool(key, gtk_switch_get_active(GTK_SWITCH(obj)));
}

void
pidgin_prefs_bind_switch(const gchar *key, GtkWidget *widget)
{
	gtk_switch_set_active(GTK_SWITCH(widget), purple_prefs_get_bool(key));
	g_signal_connect(widget, "notify::active",
	                 G_CALLBACK(set_bool_switch_pref), (gchar *)key);
}

static void
set_expander_row_pref(GObject *obj, G_GNUC_UNUSED GParamSpec *pspec,
                     gpointer data)
{
	const gchar *key = data;
	gboolean enabled;

	enabled = adw_expander_row_get_enable_expansion(ADW_EXPANDER_ROW(obj));
	purple_prefs_set_bool(key, enabled);
}

void
pidgin_prefs_bind_expander_row(const gchar *key, GtkWidget *widget)
{
	adw_expander_row_set_enable_expansion(ADW_EXPANDER_ROW(widget),
	                                      purple_prefs_get_bool(key));
	g_signal_connect(widget, "notify::enable-expansion",
	                 G_CALLBACK(set_expander_row_pref), (gchar *)key);
}

static void
vv_test_switch_page_cb(GtkStack *stack, G_GNUC_UNUSED GParamSpec *pspec,
                       gpointer data)
{
	PidginVVPrefs *vv_prefs = data;

	if (!g_str_equal(gtk_stack_get_visible_child_name(stack), "vv")) {
		/* Disable any running test pipelines. */
		pidgin_vv_prefs_disable_test_pipelines(vv_prefs);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_prefs_window_finalize(GObject *obj) {
	purple_prefs_disconnect_by_handle(obj);

	G_OBJECT_CLASS(pidgin_prefs_window_parent_class)->finalize(obj);
}

static void
pidgin_prefs_window_class_init(PidginPrefsWindowClass *klass)
{
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_prefs_window_finalize;

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Prefs/prefs.ui"
	);

	/* Main window */
	gtk_widget_class_bind_template_callback(widget_class,
	                                        vv_test_switch_page_cb);
}

static void
pidgin_prefs_window_init(PidginPrefsWindow *win)
{
	gtk_widget_init_template(GTK_WIDGET(win));
}

/******************************************************************************
 * API
 *****************************************************************************/
void
pidgin_prefs_init(void)
{
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "");
	purple_prefs_add_none("/plugins/gtk");

	/* Plugins */
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/plugins");
	purple_prefs_add_path_list(PIDGIN_PREFS_ROOT "/plugins/loaded", NULL);

	/* File locations */
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/filelocations");
	purple_prefs_add_path(PIDGIN_PREFS_ROOT "/filelocations/last_save_folder", "");
	purple_prefs_add_path(PIDGIN_PREFS_ROOT "/filelocations/last_open_folder", "");
	purple_prefs_add_path(PIDGIN_PREFS_ROOT "/filelocations/last_icon_folder", "");

	/* Voice/Video */
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/audio");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/audio/src");
	purple_prefs_add_string(PIDGIN_PREFS_ROOT "/vvconfig/audio/src/device", "");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/audio/sink");
	purple_prefs_add_string(PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/device", "");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/video");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/video/src");
	purple_prefs_add_string(PIDGIN_PREFS_ROOT "/vvconfig/video/src/device", "");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/video");
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/vvconfig/video/sink");
	purple_prefs_add_string(PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device", "");

	pidgin_prefs_update_old();
}

void
pidgin_prefs_update_old(void)
{
	const gchar *video_sink = NULL;

	/* Rename some old prefs */
	purple_prefs_rename(PIDGIN_PREFS_ROOT "/conversations/im/raise_on_events", "/plugins/gtk/X11/notify/method_raise");

	purple_prefs_rename_boolean_toggle(PIDGIN_PREFS_ROOT "/conversations/ignore_colors",
									 PIDGIN_PREFS_ROOT "/conversations/show_incoming_formatting");

	/* Remove some no-longer-used prefs */
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/auto_expand_contacts");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/button_style");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/grey_idle_buddies");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/raise_on_events");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/show_group_count");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/show_warning_level");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/tooltip_delay");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/x");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/blist/y");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/browsers");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/browsers/browser");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/browsers/command");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/browsers/place");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/browsers/manual_command");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/button_type");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/ctrl_enter_sends");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/enter_sends");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/escape_closes");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/html_shortcuts");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/icons_on_tabs");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/send_formatting");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/show_urls_as_links");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/use_custom_bgcolor");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/use_custom_fgcolor");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/use_custom_font");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/use_custom_size");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/old_tab_complete");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/tab_completion");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/im/hide_on_send");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/color_nicks");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/raise_on_events");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/ignore_fonts");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/ignore_font_sizes");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/passthrough_unknown_commands");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/debug/timestamps");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/idle");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/signon");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/silent_signon");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/command");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/conv_focus");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/chat_msg_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/first_im_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/got_attention");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/im_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/join_chat");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/left_chat");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/login");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/logout");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/nick_said");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/pounce_default");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/send_chat_msg");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/send_im");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled/sent_attention");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/enabled");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/chat_msg_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/first_im_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/got_attention");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/im_recv");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/join_chat");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/left_chat");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/login");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/logout");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/nick_said");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/pounce_default");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/send_chat_msg");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/send_im");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file/sent_attention");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/file");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/method");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/mute");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound/theme");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/sound");

	purple_prefs_remove(PIDGIN_PREFS_ROOT "/away/queue_messages");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/away");
	purple_prefs_remove("/plugins/gtk/docklet/queue_messages");

	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/default_width");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat/default_height");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/im/default_width");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/im/default_height");
	purple_prefs_rename(PIDGIN_PREFS_ROOT "/conversations/x",
			PIDGIN_PREFS_ROOT "/conversations/im/x");
	purple_prefs_rename(PIDGIN_PREFS_ROOT "/conversations/y",
			PIDGIN_PREFS_ROOT "/conversations/im/y");

	/* Fixup vvconfig plugin prefs */
	if (purple_prefs_exists("/plugins/core/vvconfig/audio/src/device")) {
		purple_prefs_set_string(PIDGIN_PREFS_ROOT "/vvconfig/audio/src/device",
				purple_prefs_get_string("/plugins/core/vvconfig/audio/src/device"));
	}
	if (purple_prefs_exists("/plugins/core/vvconfig/audio/sink/device")) {
		purple_prefs_set_string(PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/device",
				purple_prefs_get_string("/plugins/core/vvconfig/audio/sink/device"));
	}
	if (purple_prefs_exists("/plugins/core/vvconfig/video/src/device")) {
		purple_prefs_set_string(PIDGIN_PREFS_ROOT "/vvconfig/video/src/device",
				purple_prefs_get_string("/plugins/core/vvconfig/video/src/device"));
	}
	if (purple_prefs_exists("/plugins/gtk/vvconfig/video/sink/device")) {
		purple_prefs_set_string(PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device",
				purple_prefs_get_string("/plugins/gtk/vvconfig/video/sink/device"));
	}

	video_sink = purple_prefs_get_string(PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device");
	if (purple_strequal(video_sink, "glimagesink") || purple_strequal(video_sink, "directdrawsink")) {
		/* Accelerated sinks move to GTK GL. */
		/* video_sink = "gtkglsink"; */
		/* FIXME: I haven't been able to get gtkglsink to work yet: */
		video_sink = "gtksink";
	} else {
		/* Everything else, including default will be moved to GTK sink. */
		video_sink = "gtksink";
	}
	purple_prefs_set_string(PIDGIN_PREFS_ROOT "/vvconfig/video/sink/device", video_sink);

	purple_prefs_remove("/plugins/core/vvconfig");
	purple_prefs_remove("/plugins/gtk/vvconfig");

	purple_prefs_remove(PIDGIN_PREFS_ROOT "/vvconfig/audio/src/plugin");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/vvconfig/audio/sink/plugin");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/vvconfig/video/src/plugin");
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/vvconfig/video/sink/plugin");
}

