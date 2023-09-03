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

#ifndef PURPLE_PROTOCOL_ROSTER_H
#define PURPLE_PROTOCOL_ROSTER_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/account.h>
#include <libpurple/purplecontact.h>
#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_ROSTER (purple_protocol_roster_get_type())
G_DECLARE_INTERFACE(PurpleProtocolRoster, purple_protocol_roster, PURPLE,
                    PROTOCOL_ROSTER, PurpleProtocol)

/**
 * PurpleProtocolRoster:
 *
 * The #PurpleProtocolRoster interface defines the behavior to tell a
 * protocol when the users wants to manage contacts on the server side roster.
 *
 * Since: 3.0.0
 */

/**
 * PurpleProtocolRosterInterface:
 * @add_async: Called when the user is trying to add a contact to the server
 *             side roster.
 * @add_finish: Called when adding the contact has completed.
 * @update_async: Called when the user is trying to update a contact on the
 *                server side roster.
 * @update_finish: Called when updating the contact has completed.
 * @remove_async: Called when the user is trying to remove a contact from the
 *                server side roster.
 * @remove_finish: Called when removing the contact has completed.
 *
 * The interface for managing the server side roster.
 *
 * This interface provides a gateway between purple and the protocol for
 * managing the server side roster. All of the functions are asynchronous to
 * make sure nothing blocks the rest of the program.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolRosterInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	void (*add_async)(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*add_finish)(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

	void (*update_async)(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*update_finish)(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

	void (*remove_async)(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	gboolean (*remove_finish)(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_protocol_roster_add_async:
 * @roster: The instance.
 * @account: The [class@Account] to use.
 * @contact: The [class@Contact] to add.
 * @cancellable: A [class@Gio.Cancellable].
 * @callback: A [callback@Gio.AsyncReadyCallback] to call when the request has
 *            completed.
 * @data: User data to pass to @callback.
 *
 * Requests that @roster adds @contact to the server side roster. When
 * @callback is called, [method@ProtocolRoster.add_finish] should be called to
 * get the result.
 *
 * Since: 3.0.0
 */
void purple_protocol_roster_add_async(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_roster_add_finish:
 * @roster: The instance.
 * @result: A [iface@Gio.AsyncResult].
 * @error: (out): A return address for an error.
 *
 * Gets the result of a previous call to [method@ProtocolRoster.add_async].
 *
 * If an error was encountered, @error will be set and %FALSE will be returned.
 * Otherwise the result will be returned.
 *
 * Returns: %TRUE if the operation was successful, otherwise %FALSE with error
 *          optionally set.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_roster_add_finish(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

/**
 * purple_protocol_roster_update_async:
 * @roster: The instance.
 * @account: The [class@Account] to use.
 * @contact: The [class@Contact] to update.
 * @cancellable: A [class@Gio.Cancellable].
 * @callback: A [callback@Gio.AsyncReadyCallback] to call when the request has
 *            completed.
 * @data: User data to pass to @callback.
 *
 * Requests that @roster updates @contact on the server side roster. When
 * @callback is called, [method@ProtocolRoster.update_finish] should be called
 * to get the result.
 *
 * This would include things that the libpurple user can change about a remote
 * contact. Including but not limited to [property@ContactInfo:alias],
 * [property@ContactInfo:permission], [property@ContactInfo:person], and
 * [property@ContactInfo:tags].
 *
 * Since: 3.0.0
 */
void purple_protocol_roster_update_async(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_roster_update_finish:
 * @roster: The instance.
 * @result: A [iface@Gio.AsyncResult].
 * @error: (out): A return address for an error.
 *
 * Gets the result of a previous call to [method@ProtocolRoster.update_async].
 *
 * If an error was encountered, @error will be set and %FALSE will be returned.
 * Otherwise the result will be returned.
 *
 * Returns: %TRUE if the operation was successful, otherwise %FALSE with error
 *          optionally set.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_roster_update_finish(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

/**
 * purple_protocol_roster_remove_async:
 * @roster: The instance.
 * @account: The [class@Account] to use.
 * @contact: The [class@Contact] to remove.
 * @cancellable: A [class@Gio.Cancellable].
 * @callback: A [callback@Gio.AsyncReadyCallback] to call when the request has
 *            completed.
 * @data: User data to pass to @callback.
 *
 * Requests that @roster removes @contact from the server side roster. When
 * @callback is called, [method@ProtocolRoster.remove_finish] should be called
 * to get the result.
 *
 * Since: 3.0.0
 */
void purple_protocol_roster_remove_async(PurpleProtocolRoster *roster, PurpleAccount *account, PurpleContact *contact, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_roster_remove_finish:
 * @roster: The instance.
 * @result: A [iface@Gio.AsyncResult].
 * @error: (out): A return address for an error.
 *
 * Gets the result of a previous call to [method@ProtocolRoster.remove_async].
 *
 * If an error was encountered, @error will be set and %FALSE will be returned.
 * Otherwise the result will be returned.
 *
 * Returns: %TRUE if the operation was successful, otherwise %FALSE with error
 *          optionally set.
 *
 * Since: 3.0.0
 */
gboolean purple_protocol_roster_remove_finish(PurpleProtocolRoster *roster, GAsyncResult *result, GError **error);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_ROSTER_H */
