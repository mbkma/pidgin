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

#include "pidginstatusdisplay.h"
#include "pidginiconname.h"

struct _PidginStatusDisplay {
	GtkBox parent;

	GtkImage *image;
	GtkLabel *label;

	PurpleStatusPrimitive primitive;
	PurpleSavedStatus *status;
};

enum {
	PROP_0,
	PROP_PRIMITIVE,
	PROP_SAVED_STATUS,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = {NULL};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_status_display_refresh(PidginStatusDisplay *display) {
	const char *icon_name = NULL;
	char *label = NULL;

	if(display->status != NULL) {
		PurpleSavedStatus *status = display->status;
		PurpleStatusPrimitive primitive;
		GString *text = NULL;

		primitive = purple_savedstatus_get_primitive_type(status);
		icon_name = pidgin_icon_name_from_status_primitive(primitive, NULL);

		text = g_string_new(purple_savedstatus_get_title(status));

		/* Transient statuses do not have a title, so the savedstatus API
		 * returns the message when purple_savedstatus_get_title() is called,
		 * so we don't need to get the message a second time.
		 */
		if(!purple_savedstatus_is_transient(status)) {
			const char *message = NULL;

			message = purple_savedstatus_get_message(status);
			if(message != NULL) {
				char *stripped = purple_markup_strip_html(message);

				purple_util_chrreplace(stripped, '\n', ' ');
				g_string_append_printf(text, " - %s", stripped);
				g_free(stripped);
			}
		}

		label = g_string_free(text, FALSE);

	} else if(display->primitive != PURPLE_STATUS_UNSET) {
		icon_name = pidgin_icon_name_from_status_primitive(display->primitive,
		                                                   NULL);
		label = g_strdup(purple_primitive_get_name_from_type(display->primitive));
	}

	gtk_image_set_from_icon_name(display->image, icon_name);
	gtk_label_set_text(display->label, label);

	g_free(label);
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginStatusDisplay, pidgin_status_display, GTK_TYPE_BOX)

static void
pidgin_status_display_get_property(GObject *object, guint prop_id,
                                   GValue *value, GParamSpec *pspec)
{
	PidginStatusDisplay *display = PIDGIN_STATUS_DISPLAY(object);

	switch (prop_id) {
		case PROP_PRIMITIVE:
			g_value_set_enum(value,
			                 pidgin_status_display_get_primitive(display));
			break;
		case PROP_SAVED_STATUS:
			g_value_set_pointer(value,
			                    pidgin_status_display_get_saved_status(display));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_status_display_set_property(GObject *object, guint prop_id,
                                   const GValue *value, GParamSpec *pspec)
{
	PidginStatusDisplay *display = PIDGIN_STATUS_DISPLAY(object);

	switch (prop_id) {
		case PROP_PRIMITIVE:
			pidgin_status_display_set_primitive(display,
			                                    g_value_get_enum(value));
			break;
		case PROP_SAVED_STATUS:
			pidgin_status_display_set_saved_status(display,
			                                       g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_status_display_class_init(PidginStatusDisplayClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	/* Properties */
	obj_class->get_property = pidgin_status_display_get_property;
	obj_class->set_property = pidgin_status_display_set_property;

	/**
	 * PidginStatusDisplay:primitive:
	 *
	 * The status primitive that is currently displayed.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PRIMITIVE] = g_param_spec_enum(
	        "primitive", "primitive",
	        "The status primitive that is currently displayed.",
	        PURPLE_TYPE_STATUS_PRIMITIVE, PURPLE_STATUS_UNSET,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PidginStatusDisplay:saved-status:
	 *
	 * The saved status that is currently displayed.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SAVED_STATUS] = g_param_spec_pointer(
	        "saved-status", "saved-status",
	        "The saved status that is currently displayed.",
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);

	/* Widget template */
	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Status/display.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginStatusDisplay,
	                                     image);
	gtk_widget_class_bind_template_child(widget_class, PidginStatusDisplay,
	                                     label);
}

static void
pidgin_status_display_init(PidginStatusDisplay *display) {
	gtk_widget_init_template(GTK_WIDGET(display));
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_display_new(void) {
	return g_object_new(PIDGIN_TYPE_STATUS_DISPLAY, NULL);
}

GtkWidget *
pidgin_status_display_new_for_primitive(PurpleStatusPrimitive primitive) {
	return g_object_new(PIDGIN_TYPE_STATUS_DISPLAY,
	                    "primitive", primitive,
	                    NULL);
}

GtkWidget *
pidgin_status_display_new_for_saved_status(PurpleSavedStatus *status) {
	return g_object_new(PIDGIN_TYPE_STATUS_DISPLAY,
	                    "saved-status", status,
	                    NULL);
}

PurpleStatusPrimitive
pidgin_status_display_get_primitive(PidginStatusDisplay *display) {
	g_return_val_if_fail(PIDGIN_IS_STATUS_DISPLAY(display),
	                     PURPLE_STATUS_UNSET);

	return display->primitive;
}

void
pidgin_status_display_set_primitive(PidginStatusDisplay *display,
                                    PurpleStatusPrimitive primitive)
{
	g_return_if_fail(PIDGIN_IS_STATUS_DISPLAY(display));

	g_object_freeze_notify(G_OBJECT(display));

	display->status = NULL;
	g_object_notify_by_pspec(G_OBJECT(display), properties[PROP_SAVED_STATUS]);

	if(display->primitive != primitive) {
		display->primitive = primitive;
		g_object_notify_by_pspec(G_OBJECT(display),
		                         properties[PROP_PRIMITIVE]);
	}

	pidgin_status_display_refresh(display);

	g_object_thaw_notify(G_OBJECT(display));
}

PurpleSavedStatus *
pidgin_status_display_get_saved_status(PidginStatusDisplay *display) {
	g_return_val_if_fail(PIDGIN_IS_STATUS_DISPLAY(display), NULL);

	return display->status;
}

void
pidgin_status_display_set_saved_status(PidginStatusDisplay *display,
                                       PurpleSavedStatus *status)
{
	g_return_if_fail(PIDGIN_IS_STATUS_DISPLAY(display));

	g_object_freeze_notify(G_OBJECT(display));

	display->primitive = PURPLE_STATUS_UNSET;
	g_object_notify_by_pspec(G_OBJECT(display), properties[PROP_PRIMITIVE]);

	if(display->status != status) {
		display->status = status;
		g_object_notify_by_pspec(G_OBJECT(display),
		                         properties[PROP_SAVED_STATUS]);
	}

	pidgin_status_display_refresh(display);

	g_object_thaw_notify(G_OBJECT(display));
}
