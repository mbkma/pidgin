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

#ifndef PURPLE_REQUEST_PAGE_H
#define PURPLE_REQUEST_PAGE_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleRequestPage:
 *
 * Multiple fields request data.
 *
 * Since: 3.0.0
 */
typedef struct _PurpleRequestPage PurpleRequestPage;

#include "account.h"
#include "purplerequestgroup.h"
#include "purplerequestfield.h"

G_BEGIN_DECLS

#define PURPLE_TYPE_REQUEST_PAGE (purple_request_page_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestPage, purple_request_page,
                     PURPLE, REQUEST_PAGE, GObject)

/**
 * purple_request_page_new:
 *
 * Creates a page of fields to pass to [func@Purple.request_fields].
 *
 * Returns: (transfer full): The new request page.
 *
 * Since: 3.0.0
 */
PurpleRequestPage *purple_request_page_new(void);

/**
 * purple_request_page_add_group:
 * @page: The fields page.
 * @group: (transfer full): The group to add.
 *
 * Adds a group of fields to the list.
 *
 * Since: 3.0.0
 */
void purple_request_page_add_group(PurpleRequestPage *page, PurpleRequestGroup *group);

/**
 * purple_request_page_exists:
 * @page: The fields page.
 * @id: The ID of the field.
 *
 * Returns whether or not the field with the specified ID exists.
 *
 * Returns: TRUE if the field exists, or FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_request_page_exists(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_is_field_required:
 * @page: The fields page.
 * @id: The field ID.
 *
 * Returns whether or not a field with the specified ID is required.
 *
 * Returns: TRUE if the specified field is required, or FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_request_page_is_field_required(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_is_valid:
 * @page: The fields page.
 *
 * Returns whether or not all fields are valid.
 *
 * Returns: %TRUE if all fields in the page are valid, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_request_page_is_valid(PurpleRequestPage *page);

/**
 * purple_request_page_get_field:
 * @page: The fields page.
 * @id: The ID of the field.
 *
 * Return the field with the specified ID.
 *
 * Returns: (transfer none): The field, if found.
 *
 * Since: 3.0.0
 */
PurpleRequestField *purple_request_page_get_field(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_get_string:
 * @page: The fields page.
 * @id: The ID of the field.
 *
 * Returns the string value of a field with the specified ID.
 *
 * Returns: The string value, if found, or %NULL otherwise.
 *
 * Since: 3.0.0
 */
const char *purple_request_page_get_string(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_get_integer:
 * @page: The fields page.
 * @id: The ID of the field.
 *
 * Returns the integer value of a field with the specified ID.
 *
 * Returns: The integer value, if found, or 0 otherwise.
 *
 * Since: 3.0.0
 */
int purple_request_page_get_integer(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_get_bool:
 * @page: The fields page.
 * @id: The ID of the field.
 *
 * Returns the boolean value of a field with the specified ID.
 *
 * Returns: The boolean value, if found, or %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_request_page_get_bool(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_get_choice:
 * @page: The fields page.
 * @id:     The ID of the field.
 *
 * Returns the choice index of a field with the specified ID.
 *
 * Returns: The choice value, if found, or NULL otherwise.
 *
 * Since: 3.0.0
 */
gpointer purple_request_page_get_choice(PurpleRequestPage *page, const char *id);

/**
 * purple_request_page_get_account:
 * @page: The fields page.
 * @id:     The ID of the field.
 *
 * Returns the account of a field with the specified ID.
 *
 * Returns: (transfer none): The account value, if found, or %NULL otherwise.
 *
 * Since: 3.0.0
 */
PurpleAccount *purple_request_page_get_account(PurpleRequestPage *page, const char *id);

G_END_DECLS

#endif /* PURPLE_REQUEST_PAGE_H */
