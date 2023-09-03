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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_REQUEST_FIELD_CHOICE_H
#define PURPLE_REQUEST_FIELD_CHOICE_H

#include <glib.h>
#include <glib-object.h>

#include "purplerequestfield.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldChoice:
 *
 * A choice request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_CHOICE (purple_request_field_choice_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldChoice, purple_request_field_choice,
                     PURPLE, REQUEST_FIELD_CHOICE, PurpleRequestField)

/**
 * purple_request_field_choice_new:
 * @id:            The field ID.
 * @text:          The optional label of the field.
 * @default_value: The default choice.
 *
 * Creates a multiple choice field.
 *
 * This is often represented as a group of radio buttons.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_choice_new(const char *id, const char *text, gpointer default_value);

/**
 * purple_request_field_choice_add:
 * @field: The choice field.
 * @label: The choice label.
 * @data:  The choice value.
 *
 * Adds a choice to a multiple choice field.
 */
void purple_request_field_choice_add(PurpleRequestFieldChoice *field, const char *label, gpointer data);

/**
 * purple_request_field_choice_add_full:
 * @field: The choice field.
 * @label: The choice label.
 * @data:  The choice value.
 * @destroy: The value destroy function.
 *
 * Adds a choice to a multiple choice field with destructor for value.
 *
 * Since: 3.0.0
 */
void purple_request_field_choice_add_full(PurpleRequestFieldChoice *field, const char *label, gpointer data, GDestroyNotify destroy);

/**
 * purple_request_field_choice_set_default_value:
 * @field:         The field.
 * @default_value: The default value.
 *
 * Sets the default value in a choice field.
 */
void purple_request_field_choice_set_default_value(PurpleRequestFieldChoice *field, gpointer default_value);

/**
 * purple_request_field_choice_set_value:
 * @field: The field.
 * @value: The value.
 *
 * Sets the value in a choice field.
 */
void purple_request_field_choice_set_value(PurpleRequestFieldChoice *field, gpointer value);

/**
 * purple_request_field_choice_get_default_value:
 * @field: The field.
 *
 * Returns the default value in a choice field.
 *
 * Returns: The default value.
 */
gpointer purple_request_field_choice_get_default_value(PurpleRequestFieldChoice *field);

/**
 * purple_request_field_choice_get_value:
 * @field: The field.
 *
 * Returns the user-entered value in a choice field.
 *
 * Returns: The value.
 */
gpointer purple_request_field_choice_get_value(PurpleRequestFieldChoice *field);

/**
 * purple_request_field_choice_get_elements:
 * @field: The field.
 *
 * Returns a list of elements in a choice field.
 *
 * Returns: (element-type PurpleKeyValuePair) (transfer none): The list of pairs of {label, value}.
 */
GList *purple_request_field_choice_get_elements(PurpleRequestFieldChoice *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_CHOICE_H */
