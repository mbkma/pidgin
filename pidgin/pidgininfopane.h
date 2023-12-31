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

#ifndef PIDGIN_INFO_PANE_H
#define PIDGIN_INFO_PANE_H

#include <glib.h>

#include <gtk/gtk.h>

#include <purple.h>

G_BEGIN_DECLS

/**
 * PidginInfoPane:
 *
 * #PidginInfoPane is a widget that displays information above
 * #PidginConversations.
 *
 * Since: 3.0.0
 */

#define PIDGIN_TYPE_INFO_PANE (pidgin_info_pane_get_type())
G_DECLARE_FINAL_TYPE(PidginInfoPane, pidgin_info_pane, PIDGIN, INFO_PANE,
                     GtkBox)

/**
 * pidgin_info_pane_new:
 * @conversation: The #PurpleConversation instance.
 *
 * Creates a new #PidginInfoPane instance that will display information about
 * @conversation.
 *
 * Returns: (transfer full): The new #PidginInfoPane instance.
 */
GtkWidget *pidgin_info_pane_new(PurpleConversation *conversation);

/**
 * pidgin_info_pane_get_conversation:
 * @pane: The #PidginInfoPane instance.
 *
 * Gets the #PurpleConversation that @pane is displaying information for.
 *
 * Returns: (transfer none): The #PurpleConversation displayed by @pane.
 */
PurpleConversation *pidgin_info_pane_get_conversation(PidginInfoPane *pane);

/**
 * pidgin_info_pane_set_conversation:
 * @pane: The instance.
 * @conversation: (nullable): The [class@Purple.Conversation] to use.
 *
 * Sets the conversation that @pane uses to get its values.
 *
 * Typically this is only necessary for the conversation instance itself.
 *
 * Since: 3.0.0
 */
void pidgin_info_pane_set_conversation(PidginInfoPane *pane, PurpleConversation *conversation);

G_END_DECLS

#endif /* PIDGIN_INFO_PANE_H */
