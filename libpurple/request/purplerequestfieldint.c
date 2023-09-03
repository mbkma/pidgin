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
#include "purplerequestfieldint.h"

struct _PurpleRequestFieldInt {
	PurpleRequestField parent;

	int default_value;
	int value;
	int lower_bound;
	int upper_bound;
};

enum {
	PROP_0,
	PROP_DEFAULT_VALUE,
	PROP_VALUE,
	PROP_LOWER_BOUND,
	PROP_UPPER_BOUND,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * PurpleRequestField Implementation
 *****************************************************************************/
static gboolean
purple_request_field_int_is_valid(PurpleRequestField *field, char **errmsg) {
	PurpleRequestFieldInt *intfield = PURPLE_REQUEST_FIELD_INT(field);

	if(intfield->value < intfield->lower_bound) {
		if(errmsg != NULL) {
			*errmsg = g_strdup_printf(_("Int value %d exceeds lower bound %d"),
			                          intfield->value, intfield->lower_bound);
		}
		return FALSE;
	}

	if(intfield->value > intfield->upper_bound) {
		if(errmsg != NULL) {
			*errmsg = g_strdup_printf(_("Int value %d exceeds upper bound %d"),
			                          intfield->value, intfield->upper_bound);
		}
		return FALSE;
	}

	return TRUE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldInt, purple_request_field_int,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_int_get_property(GObject *obj, guint param_id,
                                      GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldInt *field = PURPLE_REQUEST_FIELD_INT(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			g_value_set_int(value,
			                purple_request_field_int_get_default_value(field));
			break;
		case PROP_VALUE:
			g_value_set_int(value, purple_request_field_int_get_value(field));
			break;
		case PROP_LOWER_BOUND:
			g_value_set_int(value,
			                purple_request_field_int_get_lower_bound(field));
			break;
		case PROP_UPPER_BOUND:
			g_value_set_int(value,
			                purple_request_field_int_get_upper_bound(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_int_set_property(GObject *obj, guint param_id,
                                      const GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldInt *field = PURPLE_REQUEST_FIELD_INT(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			purple_request_field_int_set_default_value(field,
			                                           g_value_get_int(value));
			break;
		case PROP_VALUE:
			purple_request_field_int_set_value(field, g_value_get_int(value));
			break;
		case PROP_LOWER_BOUND:
			purple_request_field_int_set_lower_bound(field,
			                                         g_value_get_int(value));
			break;
		case PROP_UPPER_BOUND:
			purple_request_field_int_set_upper_bound(field,
			                                         g_value_get_int(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_int_init(G_GNUC_UNUSED PurpleRequestFieldInt *field) {
}

static void
purple_request_field_int_class_init(PurpleRequestFieldIntClass *klass) {
	PurpleRequestFieldClass *request_class = PURPLE_REQUEST_FIELD_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	request_class->is_valid = purple_request_field_int_is_valid;

	obj_class->get_property = purple_request_field_int_get_property;
	obj_class->set_property = purple_request_field_int_set_property;

	/**
	 * PurpleRequestFieldInt:default-value:
	 *
	 * The default value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DEFAULT_VALUE] = g_param_spec_int(
		"default-value", "default-value",
		"The default value of the field.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldInt:value:
	 *
	 * The value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALUE] = g_param_spec_int(
		"value", "value",
		"The value of the field.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldInt:lower-bound:
	 *
	 * The lower bound of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_LOWER_BOUND] = g_param_spec_int(
		"lower-bound", "lower-bound",
		"The lower bound of the field.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldInt:upper-bound:
	 *
	 * The upper bound of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_UPPER_BOUND] = g_param_spec_int(
		"upper-bound", "upper-bound",
		"The upper bound of the field.",
		G_MININT, G_MAXINT, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_int_new(const char *id, const char *text,
                             int default_value, int lower_bound,
                             int upper_bound)
{
	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_INT,
	                    "id", id,
	                    "label", text,
	                    "lower-bound", lower_bound,
	                    "upper-bound", upper_bound,
	                    "default-value", default_value,
	                    "value", default_value,
	                    NULL);
}

void
purple_request_field_int_set_default_value(PurpleRequestFieldInt *field,
                                           int default_value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field));

	if(field->default_value == default_value) {
		return;
	}

	field->default_value = default_value;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_DEFAULT_VALUE]);
}

void
purple_request_field_int_set_lower_bound(PurpleRequestFieldInt *field,
                                         int lower_bound)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field));

	if(field->lower_bound == lower_bound) {
		return;
	}

	field->lower_bound = lower_bound;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_LOWER_BOUND]);
	g_object_notify(G_OBJECT(field), "valid");
	g_object_thaw_notify(G_OBJECT(field));
}

void
purple_request_field_int_set_upper_bound(PurpleRequestFieldInt *field,
                                         int upper_bound)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field));

	if(field->upper_bound == upper_bound) {
		return;
	}

	field->upper_bound = upper_bound;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_UPPER_BOUND]);
	g_object_notify(G_OBJECT(field), "valid");
	g_object_thaw_notify(G_OBJECT(field));
}

void
purple_request_field_int_set_value(PurpleRequestFieldInt *field, int value) {
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field));

	if(field->value == value) {
		return;
	}

	field->value = value;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALUE]);
	g_object_notify(G_OBJECT(field), "valid");
	g_object_thaw_notify(G_OBJECT(field));
}

int
purple_request_field_int_get_default_value(PurpleRequestFieldInt *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field), 0);

	return field->default_value;
}

int
purple_request_field_int_get_lower_bound(PurpleRequestFieldInt *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field), 0);

	return field->lower_bound;
}

int
purple_request_field_int_get_upper_bound(PurpleRequestFieldInt *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field), 0);

	return field->upper_bound;
}

int
purple_request_field_int_get_value(PurpleRequestFieldInt *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field), 0);

	return field->value;
}
