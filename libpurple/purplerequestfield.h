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

#ifndef PURPLE_REQUEST_FIELD_H
#define PURPLE_REQUEST_FIELD_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleRequestField:
 *
 * A request field.
 *
 * Since: 3.0.0
 */
typedef struct _PurpleRequestField PurpleRequestField;

#include "purplerequestgroup.h"

/**
 * PurpleRequestFieldClass:
 *
 * #PurpleRequestFieldClass defines the interface for a request field.
 *
 * Since: 3.0.0
 */
struct _PurpleRequestFieldClass {
	/*< private >*/
	GObjectClass parent_class;

	/*< public >*/
	gboolean (*is_filled)(PurpleRequestField *field);
	gboolean (*is_valid)(PurpleRequestField *field, char **errmsg);

	/*< private >*/
	gpointer reserved[4];
};

/**
 * PurpleRequestFieldValidator:
 * @field: The field.
 * @errmsg: (nullable) (optional) (out): A location to store an error message
 *          if the field is invalid.
 * @user_data: (closure): The data passed to
 *             [method@Purple.RequestField.set_validator].
 *
 * A callback to check whether a field is valid.
 */
typedef gboolean (*PurpleRequestFieldValidator)(PurpleRequestField *field, char **errmsg, gpointer user_data);

G_BEGIN_DECLS

#define PURPLE_TYPE_REQUEST_FIELD (purple_request_field_get_type())
G_DECLARE_DERIVABLE_TYPE(PurpleRequestField, purple_request_field,
                         PURPLE, REQUEST_FIELD, GObject)

/**
 * purple_request_field_set_label:
 * @field: The field.
 * @label: The text label.
 *
 * Sets the label text of a field.
 */
void purple_request_field_set_label(PurpleRequestField *field, const char *label);

/**
 * purple_request_field_set_visible:
 * @field:   The field.
 * @visible: TRUE if visible, or FALSE if not.
 *
 * Sets whether or not a field is visible.
 */
void purple_request_field_set_visible(PurpleRequestField *field, gboolean visible);

/**
 * purple_request_field_set_type_hint:
 * @field:     The field.
 * @type_hint: The type hint.
 *
 * Sets the type hint for the field.
 *
 * This is optionally used by the UIs to provide such features as
 * auto-completion for type hints like "account" and "screenname".
 */
void purple_request_field_set_type_hint(PurpleRequestField *field,
									  const char *type_hint);

/**
 * purple_request_field_set_tooltip:
 * @field:     The field.
 * @tooltip:   The tooltip text.
 *
 * Sets the tooltip for the field.
 *
 * This is optionally used by the UIs to provide a tooltip for
 * the field.
 */
void purple_request_field_set_tooltip(PurpleRequestField *field, const char *tooltip);

/**
 * purple_request_field_set_required:
 * @field:    The field.
 * @required: TRUE if required, or FALSE.
 *
 * Sets whether or not a field is required.
 */
void purple_request_field_set_required(PurpleRequestField *field, gboolean required);

/**
 * purple_request_field_get_group:
 * @field: The field.
 *
 * Returns the group for the field.
 *
 * Returns: (transfer none): The UI data.
 */
PurpleRequestGroup *purple_request_field_get_group(PurpleRequestField *field);

/**
 * purple_request_field_get_id:
 * @field: The field.
 *
 * Returns the ID of a field.
 *
 * Returns: The ID
 */
const char *purple_request_field_get_id(PurpleRequestField *field);

/**
 * purple_request_field_get_label:
 * @field: The field.
 *
 * Returns the label text of a field.
 *
 * Returns: The label text.
 */
const char *purple_request_field_get_label(PurpleRequestField *field);

/**
 * purple_request_field_is_visible:
 * @field: The field.
 *
 * Returns whether or not a field is visible.
 *
 * Returns: TRUE if the field is visible. FALSE otherwise.
 */
gboolean purple_request_field_is_visible(PurpleRequestField *field);

/**
 * purple_request_field_get_type_hint:
 * @field: The field.
 *
 * Returns the field's type hint.
 *
 * Returns: The field's type hint.
 */
const char *purple_request_field_get_type_hint(PurpleRequestField *field);

/**
 * purple_request_field_get_tooltip:
 * @field: The field.
 *
 * Returns the field's tooltip.
 *
 * Returns: The field's tooltip.
 */
const char *purple_request_field_get_tooltip(PurpleRequestField *field);

/**
 * purple_request_field_is_required:
 * @field: The field.
 *
 * Returns whether or not a field is required.
 *
 * Returns: TRUE if the field is required, or FALSE.
 */
gboolean purple_request_field_is_required(PurpleRequestField *field);

/**
 * purple_request_field_is_filled:
 * @field: The field.
 *
 * Returns whether the field is currently filled.
 *
 * Note: For subclassers, if this is not overridden, then the field is assumed
 * to always be filled. If the filled status changes, then subclasses should
 * notify on [property@RequestField:filled].
 *
 * Returns: TRUE if the field has value, or FALSE.
 */
gboolean purple_request_field_is_filled(PurpleRequestField *field);

/**
 * purple_request_field_set_validator:
 * @field:     The field.
 * @validator: (scope notified) (closure user_data): The validator callback, or
 *             %NULL to disable additional validation.
 * @user_data: The data to pass to the validator callback.
 * @destroy_data: A cleanup function for @user_data.
 *
 * Set an additional validator for a field.
 */
void purple_request_field_set_validator(PurpleRequestField *field, PurpleRequestFieldValidator validator, gpointer user_data, GDestroyNotify destroy_data);

/**
 * purple_request_field_is_valid:
 * @field:  The field.
 * @errmsg: (nullable) (optional) (out): If non-%NULL, the memory area, where
 *          the validation failure message will be returned.
 *
 * Checks, if specified field is valid.
 *
 * If detailed message about failure reason is needed, there is an option to
 * return (via errmsg argument) pointer to newly allocated error message.
 * It must be freed with g_free after use.
 *
 * Note: Required, but unfilled, fields are invalid.
 *
 * Returns: TRUE, if the field is valid, FALSE otherwise.
 */
gboolean purple_request_field_is_valid(PurpleRequestField *field, gchar **errmsg);

/**
 * purple_request_field_set_sensitive:
 * @field:     The field.
 * @sensitive: TRUE if the field should be sensitive for user input.
 *
 * Sets field editable.
 */
void purple_request_field_set_sensitive(PurpleRequestField *field, gboolean sensitive);

/**
 * purple_request_field_is_sensitive:
 * @field: The field.
 *
 * Checks, if field is editable.
 *
 * Returns: TRUE, if the field is sensitive for user input.
 */
gboolean purple_request_field_is_sensitive(PurpleRequestField *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_H */
