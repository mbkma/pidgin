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

#ifndef PURPLE_PROTOCOL_CONTACTS_H
#define PURPLE_PROTOCOL_CONTACTS_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_CONTACTS (purple_protocol_contacts_get_type())
G_DECLARE_INTERFACE(PurpleProtocolContacts,
                    purple_protocol_contacts, PURPLE,
                    PROTOCOL_CONTACTS, PurpleProtocol)

/**
 * PurpleProtocolContacts:
 *
 * #PurpleProtocolContacts provides methods for interacting with remote
 * contacts.
 *
 * Since: 3.0.0
 */

/**
 * PurpleProtocolContactsInterface:
 *
 * This interface defines the behavior for interacting with contacts at the
 * protocol layer. These methods will primarily be called by the user
 * interface.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolContactsInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	void (*get_profile_async)(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	char *(*get_profile_finish)(PurpleProtocolContacts *protocol_contacts, GAsyncResult *result, GError **error);

	GActionGroup *(*get_actions)(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info);
	GMenuModel *(*get_menu)(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_protocol_contacts_get_profile_async:
 * @protocol_contacts: The instance.
 * @info: The [class@ContactInfo] whose profile to get.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Starts the process requesting the user profile for @info.
 *
 * Call [method@ProtocolContacts.get_profile_finish] to get the results.
 *
 * Since: 3.0.0
 */
void purple_protocol_contacts_get_profile_async(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_contacts_get_profile_finish:
 * @search: The instance.
 * @result: The [iface@Gio.AsyncResult] from the previous
 *          [method@ProtocolContacts.get_profile_async] call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to [method@ProtocolContacts.get_profile_async] and
 * gets the result.
 *
 * Returns: (transfer full): A plain text or markdown formatted string of the
 *          contact info's profile, or %NULL with @error set on error.
 *
 * Since: 3.0.0
 */
char *purple_protocol_contacts_get_profile_finish(PurpleProtocolContacts *protocol_contacts, GAsyncResult *result, GError **error);

/**
 * purple_protocol_contacts_get_actions:
 * @protocol_contacts: The instance.
 * @info: The [class@ContactInfo] to get the actions for.
 *
 * Gets a [iface@Gio.ActionGroup] for @info. When this action group is used,
 * it should use the prefix of `contact`.
 *
 * Returns: (transfer full): The action group or %NULL.
 *
 * Since: 3.0.0
 */
GActionGroup *purple_protocol_contacts_get_actions(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info);

/*
 * purple_protocol_contacts_get_menu:
 * @protocol_contacts: The instance.
 * @info: The [class@ContactInfo] to get the menu for.
 *
 * Gets a [class@Gio.MenuModel] for @info.
 *
 * This menu will have at least the action groups from [iface@ProtocolActions]
 * and [iface@ProtocolContacts] available to it.
 *
 * Returns: (transfer full): The menu model or %NULL.
 *
 * Since: 3.0.0
 */
GMenuModel *purple_protocol_contacts_get_menu(PurpleProtocolContacts *protocol_contacts, PurpleContactInfo *info);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_CONTACTS_H */
