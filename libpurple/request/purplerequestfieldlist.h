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

#ifndef PURPLE_REQUEST_FIELD_LIST_H
#define PURPLE_REQUEST_FIELD_LIST_H

#include <glib.h>
#include <glib-object.h>

#include "purplerequestfield.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldList:
 *
 * A list request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_LIST (purple_request_field_list_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldList, purple_request_field_list,
                     PURPLE, REQUEST_FIELD_LIST, PurpleRequestField)

/**
 * purple_request_field_list_new:
 * @id:   The field ID.
 * @text: The optional label of the field.
 *
 * Creates a multiple list item field.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_list_new(const char *id, const char *text);

/**
 * purple_request_field_list_set_multi_select:
 * @field:        The list field.
 * @multi_select: TRUE if multiple selection is enabled,
 *                     or FALSE otherwise.
 *
 * Sets whether or not a list field allows multiple selection.
 */
void purple_request_field_list_set_multi_select(PurpleRequestFieldList *field, gboolean multi_select);

/**
 * purple_request_field_list_get_multi_select:
 * @field: The list field.
 *
 * Returns whether or not a list field allows multiple selection.
 *
 * Returns: TRUE if multiple selection is enabled, or FALSE otherwise.
 */
gboolean purple_request_field_list_get_multi_select(PurpleRequestFieldList *field);

/**
 * purple_request_field_list_get_data:
 * @field: The list field.
 * @text:  The item text.
 *
 * Returns the data for a particular item.
 *
 * Returns: The data associated with the item.
 */
gpointer purple_request_field_list_get_data(PurpleRequestFieldList *field, const char *text);

/**
 * purple_request_field_list_add_icon:
 * @field:     The list field.
 * @item:      The list item.
 * @icon_path: The path to icon file, or %NULL for no icon.
 * @data:      The associated data.
 *
 * Adds an item to a list field.
 */
void purple_request_field_list_add_icon(PurpleRequestFieldList *field, const char *item, const char *icon_path, gpointer data);

/**
 * purple_request_field_list_add_selected:
 * @field: The field.
 * @item:  The item to add.
 *
 * Adds a selected item to the list field.
 */
void purple_request_field_list_add_selected(PurpleRequestFieldList *field, const char *item);

/**
 * purple_request_field_list_clear_selected:
 * @field: The field.
 *
 * Clears the list of selected items in a list field.
 */
void purple_request_field_list_clear_selected(PurpleRequestFieldList *field);

/**
 * purple_request_field_list_set_selected:
 * @field: The field.
 * @items: (element-type utf8) (transfer none): The list of selected items.
 *
 * Sets a list of selected items in a list field.
 */
void purple_request_field_list_set_selected(PurpleRequestFieldList *field, GList *items);

/**
 * purple_request_field_list_is_selected:
 * @field: The field.
 * @item:  The item.
 *
 * Returns whether or not a particular item is selected in a list field.
 *
 * Returns: TRUE if the item is selected. FALSE otherwise.
 */
gboolean purple_request_field_list_is_selected(PurpleRequestFieldList *field, const char *item);

/**
 * purple_request_field_list_get_selected:
 * @field: The field.
 *
 * Returns a list of selected items in a list field.
 *
 * To retrieve the data for each item, use
 * purple_request_field_list_get_data().
 *
 * Returns: (element-type utf8) (transfer none): The list of selected items.
 */
GList *purple_request_field_list_get_selected(PurpleRequestFieldList *field);

/**
 * purple_request_field_list_get_items:
 * @field: The field.
 *
 * Returns a list of items in a list field.
 *
 * Returns: (element-type PurpleKeyValuePair) (transfer none): The list of items.
 */
GList *purple_request_field_list_get_items(PurpleRequestFieldList *field);

/**
 * purple_request_field_list_has_icons:
 * @field: The field.
 *
 * Indicates if list field has icons.
 *
 * Returns: TRUE if list field has icons, FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_request_field_list_has_icons(PurpleRequestFieldList *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_LIST_H */
