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
#include "purplerequestfieldstring.h"

struct _PurpleRequestFieldString {
	PurpleRequestField parent;

	gboolean multiline;
	gboolean masked;
	char *default_value;
	char *value;
};

enum {
	PROP_0,
	PROP_MULTILINE,
	PROP_MASKED,
	PROP_DEFAULT_VALUE,
	PROP_VALUE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_request_field_string_set_multiline(PurpleRequestFieldString *field,
                                          gboolean multiline)
{
	field->multiline = multiline;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_MULTILINE]);
}

/******************************************************************************
 * PurpleRequestField Implementation
 *****************************************************************************/
static gboolean
purple_request_field_string_is_filled(PurpleRequestField *field) {
	PurpleRequestFieldString *strfield = PURPLE_REQUEST_FIELD_STRING(field);

	return !purple_strempty(strfield->value);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldString, purple_request_field_string,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_string_get_property(GObject *obj, guint param_id,
                                         GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldString *field = PURPLE_REQUEST_FIELD_STRING(obj);

	switch(param_id) {
		case PROP_MULTILINE:
			g_value_set_boolean(value,
			                    purple_request_field_string_is_multiline(field));
			break;
		case PROP_MASKED:
			g_value_set_boolean(value,
			                    purple_request_field_string_is_masked(field));
			break;
		case PROP_DEFAULT_VALUE:
			g_value_set_string(value,
			                   purple_request_field_string_get_default_value(field));
			break;
		case PROP_VALUE:
			g_value_set_string(value,
			                   purple_request_field_string_get_value(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_string_set_property(GObject *obj, guint param_id,
                                         const GValue *value,
                                         GParamSpec *pspec)
{
	PurpleRequestFieldString *field = PURPLE_REQUEST_FIELD_STRING(obj);

	switch(param_id) {
		case PROP_MULTILINE:
			purple_request_field_string_set_multiline(field,
			                                          g_value_get_boolean(value));
			break;
		case PROP_MASKED:
			purple_request_field_string_set_masked(field,
			                                       g_value_get_boolean(value));
			break;
		case PROP_DEFAULT_VALUE:
			purple_request_field_string_set_default_value(field,
			                                              g_value_get_string(value));
			break;
		case PROP_VALUE:
			purple_request_field_string_set_value(field,
			                                      g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_string_finalize(GObject *obj) {
	PurpleRequestFieldString *field = PURPLE_REQUEST_FIELD_STRING(obj);

	g_free(field->default_value);
	g_free(field->value);

	G_OBJECT_CLASS(purple_request_field_string_parent_class)->finalize(obj);
}

static void
purple_request_field_string_init(G_GNUC_UNUSED PurpleRequestFieldString *field)
{
}

static void
purple_request_field_string_class_init(PurpleRequestFieldStringClass *klass) {
	PurpleRequestFieldClass *field_class = PURPLE_REQUEST_FIELD_CLASS(klass);
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	field_class->is_filled = purple_request_field_string_is_filled;

	obj_class->finalize = purple_request_field_string_finalize;
	obj_class->get_property = purple_request_field_string_get_property;
	obj_class->set_property = purple_request_field_string_set_property;

	/**
	 * PurpleRequestFieldString:multiline:
	 *
	 * Whether the field should allow multiline input.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MULTILINE] = g_param_spec_boolean(
		"multiline", "multiline",
		"Whether the field should allow multiline input.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldString:masked:
	 *
	 * Whether the field should be masked.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MASKED] = g_param_spec_boolean(
		"masked", "masked",
		"Whether the field should be masked.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldString:default-value:
	 *
	 * The default value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DEFAULT_VALUE] = g_param_spec_string(
		"default-value", "default-value",
		"The default value of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldString:value:
	 *
	 * The value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALUE] = g_param_spec_string(
		"value", "value",
		"The value of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_string_new(const char *id, const char *text,
                                const char *default_value, gboolean multiline)
{
	g_return_val_if_fail(id != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_STRING,
	                    "id", id,
	                    "label", text,
	                    "multiline", multiline,
	                    "default-value", default_value,
	                    "value", default_value,
	                    NULL);
}

void
purple_request_field_string_set_default_value(PurpleRequestFieldString *field,
                                              const char *default_value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field));

	if(!purple_strequal(field->default_value, default_value)) {
		g_free(field->default_value);
		field->default_value = g_strdup(default_value);

		g_object_notify_by_pspec(G_OBJECT(field),
		                         properties[PROP_DEFAULT_VALUE]);
	}
}

void
purple_request_field_string_set_value(PurpleRequestFieldString *field,
                                      const char *value)
{
	gboolean before, after;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field));

	if(purple_strequal(field->value, value)) {
		return;
	}

	before = purple_request_field_string_is_filled(PURPLE_REQUEST_FIELD(field));
	g_free(field->value);
	field->value = g_strdup(value);
	after = purple_request_field_string_is_filled(PURPLE_REQUEST_FIELD(field));

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALUE]);
	g_object_notify(G_OBJECT(field), "valid");
	if(before != after) {
		g_object_notify(G_OBJECT(field), "filled");
	}
	g_object_thaw_notify(G_OBJECT(field));
}

void
purple_request_field_string_set_masked(PurpleRequestFieldString *field,
                                       gboolean masked)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field));

	if(field->masked == masked) {
		return;
	}

	field->masked = masked;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_MASKED]);
}

const char *
purple_request_field_string_get_default_value(PurpleRequestFieldString *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), NULL);

	return field->default_value;
}

const char *
purple_request_field_string_get_value(PurpleRequestFieldString *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), NULL);

	return field->value;
}

gboolean
purple_request_field_string_is_multiline(PurpleRequestFieldString *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), FALSE);

	return field->multiline;
}

gboolean
purple_request_field_string_is_masked(PurpleRequestFieldString *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), FALSE);

	return field->masked;
}

/******************************************************************************
 * Validators
 *****************************************************************************/

gboolean
purple_request_field_email_validator(PurpleRequestField *field, gchar **errmsg,
                                     G_GNUC_UNUSED gpointer user_data)
{
	const char *value;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), FALSE);

	value = purple_request_field_string_get_value(PURPLE_REQUEST_FIELD_STRING(field));

	if (value != NULL && purple_email_is_valid(value))
		return TRUE;

	if (errmsg)
		*errmsg = g_strdup(_("Invalid email address"));
	return FALSE;
}

gboolean
purple_request_field_alphanumeric_validator(PurpleRequestField *field,
	gchar **errmsg, void *allowed_characters)
{
	const char *value;
	gchar invalid_char = '\0';

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_STRING(field), FALSE);

	value = purple_request_field_string_get_value(PURPLE_REQUEST_FIELD_STRING(field));

	g_return_val_if_fail(value != NULL, FALSE);

	if (allowed_characters)
	{
		gchar *value_r = g_strdup(value);
		g_strcanon(value_r, allowed_characters, '\0');
		invalid_char = value[strlen(value_r)];
		g_free(value_r);
	}
	else
	{
		while (value)
		{
			if (!g_ascii_isalnum(*value))
			{
				invalid_char = *value;
				break;
			}
			value++;
		}
	}
	if (!invalid_char)
		return TRUE;

	if (errmsg)
		*errmsg = g_strdup_printf(_("Invalid character '%c'"),
			invalid_char);
	return FALSE;
}
