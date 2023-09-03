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

#ifndef PURPLE_SAVED_PRESENCE_H
#define PURPLE_SAVED_PRESENCE_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purplepresence.h>

G_BEGIN_DECLS

#define PURPLE_TYPE_SAVED_PRESENCE purple_saved_presence_get_type()
G_DECLARE_FINAL_TYPE(PurpleSavedPresence, purple_saved_presence, PURPLE,
                     SAVED_PRESENCE, GObject)

/**
 * purple_saved_presence_get_last_used:
 * @presence: The instance.
 *
 * Gets the [struct@GLib.DateTime] that @presence was last used.
 *
 * Returns: (transfer none) (nullable): The time @presence was last used or
 *          %NULL.
 *
 * Since: 3.0.0
 */
GDateTime *purple_saved_presence_get_last_used(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_last_used:
 * @presence: The instance.
 * @last_used: (nullable): The time this was last used.
 *
 * Sets the last time @presence was used to @last_used. If @last_used is %NULL,
 * the time will be cleared.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_last_used(PurpleSavedPresence *presence, GDateTime *last_used);

/**
 * purple_saved_presence_get_use_count:
 * @presence: The instance.
 *
 * Gets the number of times @presence has been used.
 *
 * Returns: The number of times @presence has been used.
 *
 * Since: 3.0.0
 */
guint64 purple_saved_presence_get_use_count(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_use_count:
 * @presence: The instance.
 * @use_count: The new use count.
 *
 * Sets the number of times @presence has been used to @use_count.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_use_count(PurpleSavedPresence *presence, guint64 use_count);

/**
 * purple_saved_presence_get_id:
 * @presence: The instance.
 *
 * Gets the identifier for @presence. If an identifier was not specified when
 * @presence was created a random one will have been generated.
 *
 * Returns: The identifier for @presence.
 *
 * Since: 3.0.0
 */
const char *purple_saved_presence_get_id(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_get_name:
 * @presence: The instance.
 *
 * Gets the name of @presence.
 *
 * Returns: (nullable): The name of @presence.
 *
 * Since: 3.0.0
 */
const char *purple_saved_presence_get_name(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_name:
 * @presence: The instance.
 * @name: (nullable): The new name.
 *
 * Sets the name of @presence to @name. If @name is %NULL the name will be
 * cleared.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_name(PurpleSavedPresence *presence, const char *name);

/**
 * purple_saved_presence_get_escaped_name:
 * @presence: The instance.
 *
 * Gets the escaped version of [property@SavedPresence:name] of @presence. The
 * escaped name is suitable for serialization.
 *
 * Returns: The escaped name.
 *
 * Since: 3.0.0
 */
const char *purple_saved_presence_get_escaped_name(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_get_primitive:
 * @presence: The instance.
 *
 * Gets the [enum@PresencePrimitive] for @presence.
 *
 * Returns: The [enum@PresencePrimitive] for @presence.
 *
 * Since: 3.0.0
 */
PurplePresencePrimitive purple_saved_presence_get_primitive(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_primitive:
 * @presence: The instance.
 * @primitive: The new primitive.
 *
 * Sets the [enum@PresencePrimitive] of @presence to @primitive.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_primitive(PurpleSavedPresence *presence, PurplePresencePrimitive primitive);

/**
 * purple_saved_presence_get_message:
 * @presence: The instance.
 *
 * Gets the message of @presence.
 *
 * Returns: (nullable): The message from @presence.
 *
 * Since: 3.0.0
 */
const char *purple_saved_presence_get_message(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_message:
 * @presence: The instance.
 * @message: (nullable): The new message.
 *
 * Sets the message of @presence to @message. If @message is %NULL the message
 * is cleared.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_message(PurpleSavedPresence *presence, const char *message);

/**
 * purple_saved_presence_get_emoji:
 * @presence: The instance.
 *
 * Gets the emoji for @presence.
 *
 * Returns: (nullable): The emoji for @presence.
 *
 * Since: 3.0.0
 */
const char *purple_saved_presence_get_emoji(PurpleSavedPresence *presence);

/**
 * purple_saved_presence_set_emoji:
 * @presence: The instance.
 * @emoji: (nullable): The new emoji.
 *
 * Sets the emoji of @presence to @emoji. If @emoji is %NULL, the emoji will be
 * cleared.
 *
 * Since: 3.0.0
 */
void purple_saved_presence_set_emoji(PurpleSavedPresence *presence, const char *emoji);

/**
 * purple_saved_presence_equal:
 * @a: (nullable): The first instance.
 * @b: (nullable): The second instance.
 *
 * Checks if @a is equal to @b. This is done by checking each property for
 * equality.
 *
 * Returns: %TRUE if @a is equal to @b, otherwise %FALSE.
 *
 * Since: 3.0.0
 */
gboolean purple_saved_presence_equal(PurpleSavedPresence *a, PurpleSavedPresence *b);

G_END_DECLS

#endif /* PURPLE_SAVED_PRESENCE_H */
