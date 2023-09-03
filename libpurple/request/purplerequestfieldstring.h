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

#ifndef PURPLE_REQUEST_FIELD_STRING_H
#define PURPLE_REQUEST_FIELD_STRING_H

#include <glib.h>
#include <glib-object.h>

#include "purplerequestfield.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldString:
 *
 * A string request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_STRING (purple_request_field_string_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldString, purple_request_field_string,
                     PURPLE, REQUEST_FIELD_STRING, PurpleRequestField)

/**
 * purple_request_field_string_new:
 * @id:            The field ID.
 * @text:          The text label of the field.
 * @default_value: The optional default value.
 * @multiline:     Whether or not this should be a multiline string.
 *
 * Creates a string request field.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_string_new(const char *id, const char *text, const char *default_value, gboolean multiline);

/**
 * purple_request_field_string_set_default_value:
 * @field:         The field.
 * @default_value: The default value.
 *
 * Sets the default value in a string field.
 */
void purple_request_field_string_set_default_value(PurpleRequestFieldString *field, const char *default_value);

/**
 * purple_request_field_string_set_value:
 * @field: The field.
 * @value: The value.
 *
 * Sets the value in a string field.
 */
void purple_request_field_string_set_value(PurpleRequestFieldString *field, const char *value);

/**
 * purple_request_field_string_set_masked:
 * @field:  The field.
 * @masked: The masked value.
 *
 * Sets whether or not a string field is masked
 * (commonly used for password fields).
 */
void purple_request_field_string_set_masked(PurpleRequestFieldString *field, gboolean masked);

/**
 * purple_request_field_string_get_default_value:
 * @field: The field.
 *
 * Returns the default value in a string field.
 *
 * Returns: The default value.
 */
const char *purple_request_field_string_get_default_value(PurpleRequestFieldString *field);

/**
 * purple_request_field_string_get_value:
 * @field: The field.
 *
 * Returns the user-entered value in a string field.
 *
 * Returns: The value.
 */
const char *purple_request_field_string_get_value(PurpleRequestFieldString *field);

/**
 * purple_request_field_string_is_multiline:
 * @field: The field.
 *
 * Returns whether or not a string field is multi-line.
 *
 * Returns: %TRUE if the field is mulit-line, or %FALSE otherwise.
 */
gboolean purple_request_field_string_is_multiline(PurpleRequestFieldString *field);

/**
 * purple_request_field_string_is_masked:
 * @field: The field.
 *
 * Returns whether or not a string field is masked.
 *
 * Returns: %TRUE if the field is masked, or %FALSE otherwise.
 */
gboolean purple_request_field_string_is_masked(PurpleRequestFieldString *field);

/**
 * purple_request_field_email_validator:
 * @field: The field.
 * @errmsg: (out) (optional): destination for error message.
 * @user_data: Ignored.
 *
 * Validates a field which should contain an email address.
 *
 * See [method@Purple.RequestField.set_validator].
 *
 * Returns: TRUE, if field contains valid email address.
 */
gboolean purple_request_field_email_validator(PurpleRequestField *field, char **errmsg, gpointer user_data);

/**
 * purple_request_field_alphanumeric_validator:
 * @field: The field.
 * @errmsg: (allow-none): destination for error message.
 * @allowed_characters: (allow-none): allowed character list
 *                      (NULL-terminated string).
 *
 * Validates a field which should contain alphanumeric content.
 *
 * See [method@Purple.RequestField.set_validator].
 *
 * Returns: TRUE, if field contains only alphanumeric characters.
 */
gboolean purple_request_field_alphanumeric_validator(PurpleRequestField *field, char **errmsg, gpointer allowed_characters);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_STRING_H */
