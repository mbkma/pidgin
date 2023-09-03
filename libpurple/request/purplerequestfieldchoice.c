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
#include "purplerequestfieldchoice.h"
#include "purplekeyvaluepair.h"

struct _PurpleRequestFieldChoice {
	PurpleRequestField parent;

	gpointer default_value;
	gpointer value;

	GList *elements;
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
G_DEFINE_TYPE(PurpleRequestFieldChoice, purple_request_field_choice,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_choice_get_property(GObject *obj, guint param_id,
                                         GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldChoice *field = PURPLE_REQUEST_FIELD_CHOICE(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			g_value_set_pointer(value,
			                    purple_request_field_choice_get_default_value(field));
			break;
		case PROP_VALUE:
			g_value_set_pointer(value,
			                    purple_request_field_choice_get_value(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_choice_set_property(GObject *obj, guint param_id,
                                         const GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldChoice *field = PURPLE_REQUEST_FIELD_CHOICE(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			purple_request_field_choice_set_default_value(field,
			                                              g_value_get_pointer(value));
			break;
		case PROP_VALUE:
			purple_request_field_choice_set_value(field,
			                                      g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_choice_finalize(GObject *obj) {
	PurpleRequestFieldChoice *field = PURPLE_REQUEST_FIELD_CHOICE(obj);

	g_list_free_full(field->elements,
	                 (GDestroyNotify)purple_key_value_pair_free);

	G_OBJECT_CLASS(purple_request_field_choice_parent_class)->finalize(obj);
}

static void
purple_request_field_choice_init(G_GNUC_UNUSED PurpleRequestFieldChoice *field) {
}

static void
purple_request_field_choice_class_init(PurpleRequestFieldChoiceClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_choice_finalize;
	obj_class->get_property = purple_request_field_choice_get_property;
	obj_class->set_property = purple_request_field_choice_set_property;

	/**
	 * PurpleRequestFieldChoice:default-value:
	 *
	 * The default value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DEFAULT_VALUE] = g_param_spec_pointer(
		"default-value", "default-value",
		"The default value of the field.",
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldChoice:value:
	 *
	 * The value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALUE] = g_param_spec_pointer(
		"value", "value",
		"The value of the field.",
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_choice_new(const char *id, const char *text,
                                gpointer default_value)
{
	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_CHOICE,
	                    "id", id,
	                    "label", text,
	                    "default-value", default_value,
	                    "value", default_value,
	                    NULL);
}

void
purple_request_field_choice_add(PurpleRequestFieldChoice *field,
                                const char *label, gpointer value)
{
	purple_request_field_choice_add_full(field, label, value, NULL);
}

void
purple_request_field_choice_add_full(PurpleRequestFieldChoice *field,
                                     const char *label, gpointer value,
                                     GDestroyNotify destroy)
{
	PurpleKeyValuePair *choice;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field));
	g_return_if_fail(label != NULL);

	choice = purple_key_value_pair_new_full(label, value, destroy);

	field->elements = g_list_append(field->elements, choice);
}

void
purple_request_field_choice_set_default_value(PurpleRequestFieldChoice *field,
                                              gpointer default_value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field));

	if(field->default_value == default_value) {
		return;
	}

	field->default_value = default_value;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_DEFAULT_VALUE]);
}

void
purple_request_field_choice_set_value(PurpleRequestFieldChoice *field,
                                      gpointer value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field));

	if(field->value == value) {
		return;
	}

	field->value = value;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALUE]);
	g_object_notify(G_OBJECT(field), "valid");
	g_object_thaw_notify(G_OBJECT(field));
}

gpointer
purple_request_field_choice_get_default_value(PurpleRequestFieldChoice *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field), NULL);

	return field->default_value;
}

gpointer
purple_request_field_choice_get_value(PurpleRequestFieldChoice *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field), NULL);

	return field->value;
}

GList *
purple_request_field_choice_get_elements(PurpleRequestFieldChoice *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_CHOICE(field), NULL);

	return field->elements;
}
