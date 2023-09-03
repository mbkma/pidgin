/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#if !defined(PIDGIN_GLOBAL_HEADER_INSIDE) && !defined(PIDGIN_COMPILATION)
# error "only <pidgin.h> may be included directly"
#endif

#ifndef PIDGIN_ACCOUNT_EDITOR_H
#define PIDGIN_ACCOUNT_EDITOR_H

#include <gtk/gtk.h>

#include <adwaita.h>

#include <purple.h>

/**
 * PidginAccountEditor:
 *
 * #PidginAccountEditor is a dialog that allows you to edit an account.
 *
 * Since: 3.0.0
 */

G_BEGIN_DECLS

#define PIDGIN_TYPE_ACCOUNT_EDITOR pidgin_account_editor_get_type()
G_DECLARE_FINAL_TYPE(PidginAccountEditor, pidgin_account_editor, PIDGIN,
                     ACCOUNT_EDITOR, AdwPreferencesPage)

/**
 * pidgin_account_editor_new:
 * @account: (nullable): The [class@Purple.Account] to edit.
 *
 * Creates a new #PidginAccountEditor for @account. If @account is %NULL, the
 * editor will create a new account.
 *
 * Returns: (transfer full): The new instance.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_account_editor_new(PurpleAccount *account);

/**
 * pidgin_account_editor_get_account:
 * @editor: The instance.
 *
 * Gets the [class@Purple.Account] that @editor is modifying.
 *
 * Returns: (transfer none): The [class@Purple.Account] or %NULL.
 *
 * Since: 3.0.0
 */
PurpleAccount *pidgin_account_editor_get_account(PidginAccountEditor *editor);

/**
 * pidgin_account_editor_set_account:
 * @editor: The instance.
 * @account: (nullable): The new account to edit.
 *
 * Sets the account that @editor is editing to @account. You can pass %NULL
 * to remove the current account or edit a new account.
 *
 * Since: 3.0.0
 */
void pidgin_account_editor_set_account(PidginAccountEditor *editor, PurpleAccount *account);

/**
 * pidgin_account_editor_get_is_valid:
 * @editor: The instance.
 *
 * Gets whether or not the settings for the account is valid and can be saved.
 *
 * Returns: %TRUE if the account being edited has valid values, otherwise
 *          %FALSE.
 *
 * Since: 3.0.0
 */
gboolean pidgin_account_editor_get_is_valid(PidginAccountEditor *editor);

/**
 * pidgin_account_editor_save:
 * @editor: The instance.
 *
 * Save the account to disk. If this is a new account, it will be set to match
 * the current global status which is "online" in most cases.
 *
 * Since: 3.0.0
 */
void pidgin_account_editor_save(PidginAccountEditor *editor);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_EDITOR_H */
