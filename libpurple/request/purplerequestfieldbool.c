/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Purple is the legal property of its developers, whose names are too numerous
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

#include "purplerequestfield.h"
#include "purplerequestfieldbool.h"

struct _PurpleRequestFieldBool {
	PurpleRequestField parent;

	gboolean default_value;
	gboolean value;
};

enum {
	PROP_0,
	PROP_DEFAULT_VALUE,
	PROP_VALUE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldBool, purple_request_field_bool,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_bool_get_property(GObject *obj, guint param_id,
                                       GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldBool *field = PURPLE_REQUEST_FIELD_BOOL(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			g_value_set_boolean(value,
			                    purple_request_field_bool_get_default_value(field));
			break;
		case PROP_VALUE:
			g_value_set_boolean(value,
			                    purple_request_field_bool_get_value(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_bool_set_property(GObject *obj, guint param_id,
                                       const GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldBool *field = PURPLE_REQUEST_FIELD_BOOL(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			purple_request_field_bool_set_default_value(field,
			                                            g_value_get_boolean(value));
			break;
		case PROP_VALUE:
			purple_request_field_bool_set_value(field,
			                                    g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_bool_init(G_GNUC_UNUSED PurpleRequestFieldBool *field) {
}

static void
purple_request_field_bool_class_init(PurpleRequestFieldBoolClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->get_property = purple_request_field_bool_get_property;
	obj_class->set_property = purple_request_field_bool_set_property;

	/**
	 * PurpleRequestFieldBool:default-value:
	 *
	 * The default value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DEFAULT_VALUE] = g_param_spec_boolean(
		"default-value", "default-value",
		"The default value of the field.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldBool:value:
	 *
	 * The value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALUE] = g_param_spec_boolean(
		"value", "value",
		"The value of the field.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_bool_new(const char *id, const char *text,
                              gboolean default_value)
{
	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_BOOL,
	                    "id", id,
	                    "label", text,
	                    "default-value", default_value,
	                    "value", default_value,
	                    NULL);
}

void
purple_request_field_bool_set_default_value(PurpleRequestFieldBool *field,
                                            gboolean default_value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_BOOL(field));

	if(field->default_value == default_value) {
		return;
	}

	field->default_value = default_value;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_DEFAULT_VALUE]);
}

void
purple_request_field_bool_set_value(PurpleRequestFieldBool *field,
                                    gboolean value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_BOOL(field));

	if(field->value == value) {
		return;
	}

	field->value = value;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALUE]);
	g_object_notify(G_OBJECT(field), "valid");
	g_object_thaw_notify(G_OBJECT(field));
}

gboolean
purple_request_field_bool_get_default_value(PurpleRequestFieldBool *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_BOOL(field), FALSE);

	return field->default_value;
}

gboolean
purple_request_field_bool_get_value(PurpleRequestFieldBool *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_BOOL(field), FALSE);

	return field->value;
}
