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

#include <glib/gi18n-lib.h>

#include <gtk/gtk.h>

#include <purple.h>

#include "pidginstatusbox.h"
#include "pidginiconname.h"
#include "pidginstatusdisplay.h"

#define SAVEDSTATUS_FORMAT "savedstatus_%lu"

struct _PidginStatusBox {
	GtkBox parent;

	GtkWidget *button;
	PidginStatusDisplay *display;
	GtkPopover *popover;

	GMenu *primitives;
	GMenu *saved_statuses;
	GList *custom_widgets;
};

G_DEFINE_TYPE(PidginStatusBox, pidgin_status_box, GTK_TYPE_BOX)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static GtkWidget *
pidgin_status_box_make_primitive_widget(const char *action, const char *id) {
	GtkWidget *button = NULL;
	GtkWidget *display = NULL;
	PurpleStatusPrimitive primitive = PURPLE_STATUS_UNSET;

	primitive = purple_primitive_get_type_from_id(id);
	display = pidgin_status_display_new_for_primitive(primitive);

	button = gtk_button_new();
	gtk_widget_add_css_class(button, "flat");
	gtk_button_set_child(GTK_BUTTON(button), display);

	gtk_actionable_set_action_name(GTK_ACTIONABLE(button), action);
	gtk_actionable_set_action_target(GTK_ACTIONABLE(button),
	                                 (const char *)G_VARIANT_TYPE_INT32,
	                                 primitive);

	return button;
}

static void
pidgin_status_box_populate_primitives(PidginStatusBox *status_box) {
	GtkPopoverMenu *popover = GTK_POPOVER_MENU(status_box->popover);
	GMenuModel *menu = G_MENU_MODEL(status_box->primitives);
	gint n_items = 0;

	n_items = g_menu_model_get_n_items(menu);
	for(gint index = 0; index < n_items; index++) {
		GtkWidget *button = NULL;
		char *action = NULL;
		char *target = NULL;
		char *custom_id = NULL;

		g_menu_model_get_item_attribute(menu, index, G_MENU_ATTRIBUTE_ACTION,
		                                "s", &action);
		g_menu_model_get_item_attribute(menu, index, G_MENU_ATTRIBUTE_TARGET,
		                                "s", &target);
		g_menu_model_get_item_attribute(menu, index, "custom", "s", &custom_id);

		button = pidgin_status_box_make_primitive_widget(action, target);
		gtk_popover_menu_add_child(popover, button, custom_id);

		g_free(action);
		g_free(target);
		g_free(custom_id);
	}
}

static char *
pidgin_status_box_make_savedstatus_widget(PurpleSavedStatus *saved_status,
                                          GtkWidget **widget)
{
	GtkWidget *button = NULL;
	GtkWidget *display = NULL;
	time_t creation_time = 0;

	display = pidgin_status_display_new_for_saved_status(saved_status);

	if(!purple_savedstatus_is_transient(saved_status)) {
		GtkWidget *image = gtk_image_new_from_icon_name("document-save");
		gtk_widget_set_halign(image, GTK_ALIGN_END);
		gtk_widget_set_hexpand(image, TRUE);
		gtk_box_append(GTK_BOX(display), image);
	}

	button = gtk_button_new();
	gtk_widget_add_css_class(button, "flat");
	gtk_button_set_child(GTK_BUTTON(button), display);

	creation_time = purple_savedstatus_get_creation_time(saved_status);
	gtk_actionable_set_action_name(GTK_ACTIONABLE(button), "status.set-saved");
	gtk_actionable_set_action_target(GTK_ACTIONABLE(button),
	                                 (const char *)G_VARIANT_TYPE_INT64,
	                                 creation_time);
	*widget = button;

	return g_strdup_printf(SAVEDSTATUS_FORMAT, creation_time);
}

static void
pidgin_status_box_populate_saved_statuses(PidginStatusBox *status_box)
{
	GtkPopoverMenu *popover_menu = NULL;
	GMenu *menu = NULL;
	GList *list, *cur;

	list = purple_savedstatuses_get_popular(6);
	if (list == NULL) {
		/* Odd... oh well, nothing we can do about it. */
		return;
	}

	popover_menu = GTK_POPOVER_MENU(status_box->popover);
	menu = status_box->saved_statuses;
	for(cur = list; cur != NULL; cur = cur->next) {
		PurpleSavedStatus *saved = cur->data;
		GtkWidget *widget = NULL;
		char *id = NULL;
		GMenuItem *item = NULL;

		id = pidgin_status_box_make_savedstatus_widget(saved, &widget);
		item = g_menu_item_new(NULL, NULL);
		g_menu_item_set_attribute(item, "custom", "s", id);
		g_menu_append_item(menu, item);
		gtk_popover_menu_add_child(popover_menu, widget, id);
		status_box->custom_widgets = g_list_prepend(status_box->custom_widgets,
		                                            widget);

		g_free(id);
	}

	g_list_free(list);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_status_box_set_primitive(G_GNUC_UNUSED GSimpleAction *action,
                                GVariant *parameter, gpointer data)
{
	PidginStatusBox *status_box = data;
	PurpleSavedStatus *saved_status = NULL;
	PurpleStatusPrimitive primitive;

	gtk_menu_button_popdown(GTK_MENU_BUTTON(status_box->button));

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT32)) {
		g_critical("status.set-primitive action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	primitive = g_variant_get_int32(parameter);

	saved_status = purple_savedstatus_find_transient_by_type_and_message(primitive, NULL);
	if(saved_status == NULL) {
		saved_status = purple_savedstatus_new(NULL, primitive);
	}

	if(saved_status != NULL) {
		if(saved_status != purple_savedstatus_get_current()) {
			purple_savedstatus_activate(saved_status);
		}
	}
}

static void
pidgin_status_box_set_saved_status(G_GNUC_UNUSED GSimpleAction *action,
                                   GVariant *parameter, gpointer data)
{
	PidginStatusBox *status_box = data;
	PurpleSavedStatus *saved_status = NULL;
	time_t creation_time = 0;

	gtk_menu_button_popdown(GTK_MENU_BUTTON(status_box->button));

	if(!g_variant_is_of_type(parameter, G_VARIANT_TYPE_INT64)) {
		g_critical("status.set-saved action parameter is of incorrect type %s",
		           g_variant_get_type_string(parameter));
		return;
	}

	creation_time = (time_t)g_variant_get_int64(parameter);
	saved_status = purple_savedstatus_find_by_creation_time(creation_time);

	if(saved_status != NULL) {
		if(saved_status != purple_savedstatus_get_current()) {
			purple_savedstatus_activate(saved_status);
		}
	}
}

static void
pidgin_status_box_savedstatus_changed_cb(PurpleSavedStatus *now,
                                         G_GNUC_UNUSED PurpleSavedStatus *old,
                                         gpointer data)
{
	PidginStatusBox *status_box = data;

	/* If we don't have a status, we have to bail. */
	if(now == NULL) {
		return;
	}

	pidgin_status_display_set_saved_status(status_box->display, now);
}

static void
pidgin_status_box_remove_custom_widget(GtkWidget *widget, gpointer user_data) {
	GtkPopoverMenu *popover_menu = user_data;

	gtk_popover_menu_remove_child(popover_menu, widget);
}

static void
pidgin_status_box_savedstatus_updated_cb(G_GNUC_UNUSED PurpleSavedStatus *status,
                                         gpointer data)
{
	PidginStatusBox *status_box = data;

	g_list_foreach(status_box->custom_widgets,
	               (GFunc)pidgin_status_box_remove_custom_widget,
	               status_box->popover);
	g_clear_list(&status_box->custom_widgets, NULL);
	g_menu_remove_all(status_box->saved_statuses);

	pidgin_status_box_populate_saved_statuses(status_box);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_status_box_finalize(GObject *obj) {
	PidginStatusBox *status_box = PIDGIN_STATUS_BOX(obj);

	purple_signals_disconnect_by_handle(status_box);

	g_clear_list(&status_box->custom_widgets, NULL);

	G_OBJECT_CLASS(pidgin_status_box_parent_class)->finalize(obj);
}

static void
pidgin_status_box_init(PidginStatusBox *status_box) {
	gpointer handle;
	GSimpleActionGroup *action_group = NULL;
	GActionEntry actions[] = {
		{
			.name = "set-primitive",
			.activate = pidgin_status_box_set_primitive,
			.parameter_type = (const char *)G_VARIANT_TYPE_INT32,
		}, {
			.name = "set-saved",
			.activate = pidgin_status_box_set_saved_status,
			.parameter_type = (const char *)G_VARIANT_TYPE_INT64,
		},
	};

	gtk_widget_init_template(GTK_WIDGET(status_box));

	action_group = g_simple_action_group_new();
	g_action_map_add_action_entries(G_ACTION_MAP(action_group),
	                                actions, G_N_ELEMENTS(actions),
	                                status_box);
	gtk_widget_insert_action_group(GTK_WIDGET(status_box), "status",
	                               G_ACTION_GROUP(action_group));

	status_box->popover = gtk_menu_button_get_popover(GTK_MENU_BUTTON(status_box->button));
	gtk_popover_set_has_arrow(status_box->popover, FALSE);

	pidgin_status_box_populate_primitives(status_box);
	pidgin_status_box_populate_saved_statuses(status_box);

	handle = purple_savedstatuses_get_handle();
	purple_signal_connect(handle, "savedstatus-changed", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_changed_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-added", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-deleted", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
	purple_signal_connect(handle, "savedstatus-modified", status_box,
	                      G_CALLBACK(pidgin_status_box_savedstatus_updated_cb),
	                      status_box);
}

static void
pidgin_status_box_constructed(GObject *obj) {
	PidginStatusBox *status_box = PIDGIN_STATUS_BOX(obj);
	PurpleSavedStatus *status = NULL;

	G_OBJECT_CLASS(pidgin_status_box_parent_class)->constructed(obj);

	status = purple_savedstatus_get_current();
	pidgin_status_display_set_saved_status(status_box->display, status);
}

static void
pidgin_status_box_class_init(PidginStatusBoxClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	obj_class->finalize = pidgin_status_box_finalize;
	obj_class->constructed = pidgin_status_box_constructed;

	gtk_widget_class_set_template_from_resource(
	    widget_class,
	    "/im/pidgin/Pidgin3/Status/box.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     button);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     display);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     primitives);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusBox,
	                                     saved_statuses);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_box_new(void) {
	return g_object_new(PIDGIN_TYPE_STATUS_BOX, NULL);
}
