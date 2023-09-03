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

#ifndef PURPLE_REQUEST_FIELD_ACCOUNT_H
#define PURPLE_REQUEST_FIELD_ACCOUNT_H

#include <glib.h>
#include <glib-object.h>

#include "account.h"
#include "purplerequestfield.h"

/**
 * PurpleRequestFieldAccountFilterFunc:
 * @account: The account.
 * @user_data: The data passed to [method@Purple.RequestFieldAccount.set_filter].
 *
 * A function that is called to determine if the account should be matched.
 *
 * If the filter matches the account, this function must return %TRUE. If the
 * account should be filtered out, %FALSE must be returned.
 *
 * Returns: %TRUE to keep the account.
 *
 * Since: 3.0.0
 */
typedef gboolean (*PurpleRequestFieldAccountFilterFunc)(PurpleAccount *account, gpointer user_data);

G_BEGIN_DECLS

/**
 * PurpleRequestFieldAccount:
 *
 * An account request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_ACCOUNT (purple_request_field_account_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldAccount, purple_request_field_account,
                     PURPLE, REQUEST_FIELD_ACCOUNT, PurpleRequestField)

/**
 * purple_request_field_account_new:
 * @id:      The field ID.
 * @text:    The text label of the field.
 * @account: (transfer none): The optional default account.
 *
 * Creates an account field.
 *
 * By default, this field will not show offline accounts.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_account_new(const char *id, const char *text, PurpleAccount *account);

/**
 * purple_request_field_account_set_default_value:
 * @field:         The account field.
 * @default_value: (transfer none): The default account.
 *
 * Sets the default account on an account field.
 */
void purple_request_field_account_set_default_value(PurpleRequestFieldAccount *field, PurpleAccount *default_value);

/**
 * purple_request_field_account_set_value:
 * @field: The account field.
 * @value: (transfer none): The account.
 *
 * Sets the account in an account field.
 */
void purple_request_field_account_set_value(PurpleRequestFieldAccount *field, PurpleAccount *value);

/**
 * purple_request_field_account_set_show_all:
 * @field:    The account field.
 * @show_all: Whether or not to show all accounts.
 *
 * Sets whether or not to show all accounts in an account field.
 *
 * If TRUE, all accounts, online or offline, will be shown. If FALSE,
 * only online accounts will be shown.
 */
void purple_request_field_account_set_show_all(PurpleRequestFieldAccount *field, gboolean show_all);

/**
 * purple_request_field_account_set_filter:
 * @field:       The account field.
 * @filter_func: (scope notified) (closure user_data): The account filter
 *               function, or %NULL to disable additional filtering.
 * @user_data: The data to pass to the filter callback.
 * @destroy_data: A cleanup function for @user_data.
 *
 * Sets the account filter function in an account field.
 *
 * This function will determine which accounts get displayed and which
 * don't.
 */
void purple_request_field_account_set_filter(PurpleRequestFieldAccount *field, PurpleRequestFieldAccountFilterFunc filter_func, gpointer user_data, GDestroyNotify destroy_data);

/**
 * purple_request_field_account_get_default_value:
 * @field: The field.
 *
 * Returns the default account in an account field.
 *
 * Returns: (transfer none): The default account.
 */
PurpleAccount *purple_request_field_account_get_default_value(PurpleRequestFieldAccount *field);

/**
 * purple_request_field_account_get_value:
 * @field: The field.
 *
 * Returns the user-entered account in an account field.
 *
 * Returns: (transfer none): The user-entered account.
 */
PurpleAccount *purple_request_field_account_get_value(PurpleRequestFieldAccount *field);

/**
 * purple_request_field_account_get_show_all:
 * @field: The account field.
 *
 * Returns whether or not to show all accounts in an account field.
 *
 * If TRUE, all accounts, online or offline, will be shown. If FALSE,
 * only online accounts will be shown.
 *
 * Returns: Whether or not to show all accounts.
 */
gboolean purple_request_field_account_get_show_all(PurpleRequestFieldAccount *field);

/**
 * purple_request_field_account_match:
 * @field: The account field.
 * @account: The account to check.
 *
 * Returns whether the specified account is matched by the filter or not.
 *
 * Returns: Whether or not to show the account.
 *
 * Since: 3.0.0
 */
gboolean purple_request_field_account_match(PurpleRequestFieldAccount *field, PurpleAccount *account);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_ACCOUNT_H */
