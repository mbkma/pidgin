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

#ifndef PURPLE_REQUEST_FIELD_BOOL_H
#define PURPLE_REQUEST_FIELD_BOOL_H

#include <glib.h>
#include <glib-object.h>

#include "purplerequestfield.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldBool:
 *
 * A boolean request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_BOOL (purple_request_field_bool_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldBool, purple_request_field_bool,
                     PURPLE, REQUEST_FIELD_BOOL, PurpleRequestField)

/**
 * purple_request_field_bool_new:
 * @id:            The field ID.
 * @text:          The text label of the field.
 * @default_value: The default value.
 *
 * Creates a boolean field.
 *
 * This is often represented as a checkbox.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_bool_new(const char *id, const char *text, gboolean default_value);

/**
 * purple_request_field_bool_set_default_value:
 * @field:         The field.
 * @default_value: The default value.
 *
 * Sets the default value in an boolean field.
 */
void purple_request_field_bool_set_default_value(PurpleRequestFieldBool *field, gboolean default_value);

/**
 * purple_request_field_bool_set_value:
 * @field: The field.
 * @value: The value.
 *
 * Sets the value in an boolean field.
 */
void purple_request_field_bool_set_value(PurpleRequestFieldBool *field, gboolean value);

/**
 * purple_request_field_bool_get_default_value:
 * @field: The field.
 *
 * Returns the default value in an boolean field.
 *
 * Returns: The default value.
 */
gboolean purple_request_field_bool_get_default_value(PurpleRequestFieldBool *field);

/**
 * purple_request_field_bool_get_value:
 * @field: The field.
 *
 * Returns the user-entered value in an boolean field.
 *
 * Returns: The value.
 */
gboolean purple_request_field_bool_get_value(PurpleRequestFieldBool *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_BOOL_H */
