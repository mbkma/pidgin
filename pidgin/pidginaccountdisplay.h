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

#ifndef PIDGIN_ACCOUNT_DISPLAY_H
#define PIDGIN_ACCOUNT_DISPLAY_H

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_ACCOUNT_DISPLAY (pidgin_account_display_get_type())
G_DECLARE_FINAL_TYPE(PidginAccountDisplay, pidgin_account_display, PIDGIN,
                     ACCOUNT_DISPLAY, GtkBox)

/**
 * pidgin_account_display_new:
 * @account: The account to display.
 *
 * Creates a display for an account.
 *
 * Returns: (transfer full): The account display.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_account_display_new(PurpleAccount *account);

/**
 * pidgin_account_display_get_account:
 * @display: The display.
 *
 * Gets the currently displayed account.
 *
 * Returns: (transfer none): Returns the [type@Purple.Account] that is
 *          currently displayed.
 *
 * Since: 3.0.0
 */
PurpleAccount *pidgin_account_display_get_account(PidginAccountDisplay *display);

/**
 * pidgin_account_display_set_account:
 * @display: The display.
 * @account: The account to display.
 *
 * Sets the currently displayed account.
 *
 * Since: 3.0.0
 */
void pidgin_account_display_set_account(PidginAccountDisplay *display, PurpleAccount *account);

G_END_DECLS

#endif /* PIDGIN_ACCOUNT_DISPLAY_H */
