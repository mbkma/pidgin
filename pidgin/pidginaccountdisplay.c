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

#include "pidginaccountdisplay.h"

struct _PidginAccountDisplay {
	GtkBox parent;

	GtkImage *image;
	GtkLabel *label;

	PurpleAccount *account;
};

enum {
	PROP_0,
	PROP_ACCOUNT,
	PROP_LAST
};

static GParamSpec *properties[PROP_LAST] = {NULL};

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
pidgin_account_display_refresh(PidginAccountDisplay *display) {
	const char *icon_name = NULL;
	char *markup = NULL;

	if(PURPLE_IS_ACCOUNT(display->account)) {
		PurpleAccount *account = display->account;
		PurpleContactInfo *info = NULL;
		PurpleProtocol *protocol = NULL;
		const char *alias = NULL;
		const char *protocol_name = NULL;
		const char *username = NULL;

		protocol = purple_account_get_protocol(account);
		icon_name = purple_protocol_get_icon_name(protocol);

		info = PURPLE_CONTACT_INFO(account);
		alias = purple_contact_info_get_alias(info);
		protocol_name = purple_account_get_protocol_name(account);
		username = purple_contact_info_get_username(info);

		if(alias != NULL) {
			markup = g_strdup_printf(_("%s (%s) (%s)"), username, alias,
			                         protocol_name);
		} else {
			markup = g_strdup_printf(_("%s (%s)"), username, protocol_name);
		}
	}

	gtk_image_set_from_icon_name(display->image, icon_name);
	gtk_label_set_text(display->label, markup);

	g_free(markup);
}

/******************************************************************************
 * GObject implementation
 *****************************************************************************/
G_DEFINE_TYPE(PidginAccountDisplay, pidgin_account_display, GTK_TYPE_BOX)

static void
pidgin_account_display_get_property(GObject *object, guint prop_id,
                                    GValue *value, GParamSpec *pspec)
{
	PidginAccountDisplay *display = PIDGIN_ACCOUNT_DISPLAY(object);

	switch (prop_id) {
		case PROP_ACCOUNT:
			g_value_set_object(value,
			                   pidgin_account_display_get_account(display));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_account_display_set_property(GObject *object, guint prop_id,
                                    const GValue *value, GParamSpec *pspec)
{
	PidginAccountDisplay *display = PIDGIN_ACCOUNT_DISPLAY(object);

	switch (prop_id) {
		case PROP_ACCOUNT:
			pidgin_account_display_set_account(display,
			                                   g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
			break;
	}
}

static void
pidgin_account_display_finalize(GObject *obj) {
	PidginAccountDisplay *display = PIDGIN_ACCOUNT_DISPLAY(obj);

	g_clear_object(&display->account);

	G_OBJECT_CLASS(pidgin_account_display_parent_class)->finalize(obj);
}


static void
pidgin_account_display_class_init(PidginAccountDisplayClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	/* Properties */
	obj_class->get_property = pidgin_account_display_get_property;
	obj_class->set_property = pidgin_account_display_set_property;
	obj_class->finalize = pidgin_account_display_finalize;

	/**
	 * PurpleAccountDisplay:account:
	 *
	 * The account that is currently displayed.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ACCOUNT] = g_param_spec_object(
	        "account", "Account", "The account that is currently displayed.",
	        PURPLE_TYPE_ACCOUNT,
	        G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, PROP_LAST, properties);

	/* Widget template */
	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/Accounts/display.ui");

	gtk_widget_class_bind_template_child(widget_class, PidginAccountDisplay,
	                                     image);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountDisplay,
	                                     label);
}

static void
pidgin_account_display_init(PidginAccountDisplay *display) {
	gtk_widget_init_template(GTK_WIDGET(display));
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_account_display_new(PurpleAccount *account) {
	return g_object_new(PIDGIN_TYPE_ACCOUNT_DISPLAY,
	                    "account", account,
	                    NULL);
}

PurpleAccount *
pidgin_account_display_get_account(PidginAccountDisplay *display) {
	g_return_val_if_fail(PIDGIN_IS_ACCOUNT_DISPLAY(display), NULL);

	return display->account;
}

void
pidgin_account_display_set_account(PidginAccountDisplay *display,
                                   PurpleAccount *account)
{
	g_return_if_fail(PIDGIN_IS_ACCOUNT_DISPLAY(display));

	if(g_set_object(&display->account, account)) {
		pidgin_account_display_refresh(display);

		g_object_notify_by_pspec(G_OBJECT(display), properties[PROP_ACCOUNT]);
	}
}
