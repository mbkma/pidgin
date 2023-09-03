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

#ifndef PIDGIN_STATUS_DISPLAY_H
#define PIDGIN_STATUS_DISPLAY_H

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

#define PIDGIN_TYPE_STATUS_DISPLAY (pidgin_status_display_get_type())
G_DECLARE_FINAL_TYPE(PidginStatusDisplay, pidgin_status_display,
                     PIDGIN, STATUS_DISPLAY, GtkBox)

/**
 * pidgin_status_display_new:
 *
 * Creates a display for a status.
 *
 * Returns: (transfer full): The status display.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_status_display_new(void);

/**
 * pidgin_status_display_new_for_primitive:
 * @primitive: The status primitive to display.
 *
 * Creates a display for a status primitive.
 *
 * Returns: (transfer full): The status display.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_status_display_new_for_primitive(PurpleStatusPrimitive primitive);

/**
 * pidgin_status_display_new_for_saved_status:
 * @status: The status to display.
 *
 * Creates a display for a saved status.
 *
 * Returns: (transfer full): The status display.
 *
 * Since: 3.0.0
 */
GtkWidget *pidgin_status_display_new_for_saved_status(PurpleSavedStatus *status);

/**
 * pidgin_status_display_get_primitive:
 * @display: The display.
 *
 * Gets the currently displayed status primitive.
 *
 * Returns: (transfer none): Returns the [type@Purple.StatusPrimitive] that is
 *          currently displayed.
 *
 * Since: 3.0.0
 */
PurpleStatusPrimitive pidgin_status_display_get_primitive(PidginStatusDisplay *display);

/**
 * pidgin_status_display_set_primitive:
 * @display: The display.
 * @primitive: The status primitive to display.
 *
 * Sets the currently displayed status primitive.
 *
 * If a saved status was previously set, it will be unset.
 *
 * Since: 3.0.0
 */
void pidgin_status_display_set_primitive(PidginStatusDisplay *display, PurpleStatusPrimitive primitive);

/**
 * pidgin_status_display_get_saved_status:
 * @display: The display.
 *
 * Gets the currently displayed saved status.
 *
 * Returns: (transfer none): Returns the [type@Purple.SavedStatus] that is
 *          currently displayed.
 *
 * Since: 3.0.0
 */
PurpleSavedStatus *pidgin_status_display_get_saved_status(PidginStatusDisplay *display);

/**
 * pidgin_status_display_set_saved_status:
 * @display: The display.
 * @status: The saved status to display.
 *
 * Sets the currently displayed saved status.
 *
 * If a status primitive was previously set, it will be unset.
 *
 * Since: 3.0.0
 */
void pidgin_status_display_set_saved_status(PidginStatusDisplay *display, PurpleSavedStatus *status);

G_END_DECLS

#endif /* PIDGIN_STATUS_DISPLAY_H */
