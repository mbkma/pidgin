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

#ifndef PURPLE_REQUEST_FIELD_INT_H
#define PURPLE_REQUEST_FIELD_INT_H

#include <glib.h>
#include <glib-object.h>

#include "purplerequestfield.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldInt:
 *
 * An integer request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_INT (purple_request_field_int_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldInt, purple_request_field_int,
                     PURPLE, REQUEST_FIELD_INT, PurpleRequestField)

/**
 * purple_request_field_int_new:
 * @id:            The field ID.
 * @text:          The text label of the field.
 * @default_value: The default value.
 * @lower_bound:   The lower bound.
 * @upper_bound:   The upper bound.
 *
 * Creates an integer field.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_int_new(const char *id, const char *text, int default_value, int lower_bound, int upper_bound);

/**
 * purple_request_field_int_set_default_value:
 * @field:         The field.
 * @default_value: The default value.
 *
 * Sets the default value in an integer field.
 */
void purple_request_field_int_set_default_value(PurpleRequestFieldInt *field, int default_value);

/**
 * purple_request_field_int_set_lower_bound:
 * @field:       The field.
 * @lower_bound: The lower bound.
 *
 * Sets the lower bound in an integer field.
 */
void purple_request_field_int_set_lower_bound(PurpleRequestFieldInt *field, int lower_bound);

/**
 * purple_request_field_int_set_upper_bound:
 * @field:       The field.
 * @upper_bound: The upper bound.
 *
 * Sets the upper bound in an integer field.
 */
void purple_request_field_int_set_upper_bound(PurpleRequestFieldInt *field, int upper_bound);

/**
 * purple_request_field_int_set_value:
 * @field: The field.
 * @value: The value.
 *
 * Sets the value in an integer field.
 */
void purple_request_field_int_set_value(PurpleRequestFieldInt *field, int value);

/**
 * purple_request_field_int_get_default_value:
 * @field: The field.
 *
 * Returns the default value in an integer field.
 *
 * Returns: The default value.
 */
int purple_request_field_int_get_default_value(PurpleRequestFieldInt *field);

/**
 * purple_request_field_int_get_lower_bound:
 * @field: The field.
 *
 * Returns the lower bound in an integer field.
 *
 * Returns: The lower bound.
 */
int purple_request_field_int_get_lower_bound(PurpleRequestFieldInt *field);

/**
 * purple_request_field_int_get_upper_bound:
 * @field: The field.
 *
 * Returns the upper bound in an integer field.
 *
 * Returns: The upper bound.
 */
int purple_request_field_int_get_upper_bound(PurpleRequestFieldInt *field);

/**
 * purple_request_field_int_get_value:
 * @field: The field.
 *
 * Returns the user-entered value in an integer field.
 *
 * Returns: The value.
 */
int purple_request_field_int_get_value(PurpleRequestFieldInt *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_INT_H */
