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

#ifndef PURPLE_PROTOCOL_CONTACT_SEARCH_H
#define PURPLE_PROTOCOL_CONTACT_SEARCH_H

#include <glib.h>
#include <glib-object.h>

#include <libpurple/purpleprotocol.h>

#define PURPLE_TYPE_PROTOCOL_CONTACT_SEARCH (purple_protocol_contact_search_get_type())
G_DECLARE_INTERFACE(PurpleProtocolContactSearch,
                    purple_protocol_contact_search, PURPLE,
                    PROTOCOL_CONTACT_SEARCH, PurpleProtocol)

#define PURPLE_PROTOCOL_CONTACT_SEARCH_DOMAIN (g_quark_from_static_string("purple-protocol-contact-search"))

/**
 * PurpleProtocolContactSearch:
 *
 * The #PurpleProtocolContactSearch interface defines the behavior to search
 * for new contacts to add to your contact list. A user interface will use
 * these methods to help users find new contacts.
 *
 * Since: 3.0.0
 */

/**
 * PurpleProtocolContactSearchInterface:
 *
 * This interface defines the behavior to implement searching for new contacts.
 *
 * Since: 3.0.0
 */
struct _PurpleProtocolContactSearchInterface {
	/*< private >*/
	GTypeInterface parent;

	/*< public >*/
	void (*search_async)(PurpleProtocolContactSearch *search, PurpleAccount *account, const char *text, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
	GListModel *(*search_finish)(PurpleProtocolContactSearch *search, GAsyncResult *result, GError **error);

	/*< private >*/
	gpointer reserved[4];
};

G_BEGIN_DECLS

/**
 * purple_protocol_contact_search_search_async:
 * @search: The instance.
 * @account: The [class@Account] to search under.
 * @text: The text to search for which must not be an empty string.
 * @cancellable: (nullable): optional GCancellable object, %NULL to ignore.
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 *            satisfied.
 * @data: User data to pass to @callback.
 *
 * Starts the process of searching for contacts using @account that match
 * @text.
 *
 * Call [method@ProtocolContactSearch.search_finish] to get the results.
 *
 * Since: 3.0.0
 */
void purple_protocol_contact_search_search_async(PurpleProtocolContactSearch *search, PurpleAccount *account, const char *text, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);

/**
 * purple_protocol_contact_search_search_finish:
 * @search: The instance.
 * @result: The [iface@Gio.AsyncResult] from the previous
 *          [method@ProtocolContactSearch.search_async] call.
 * @error: Return address for a #GError, or %NULL.
 *
 * Finishes a previous call to [method@ProtocolContactSearch.search_async] and
 * gets the result.
 *
 * Returns: (transfer full): A [iface@Gio.ListModel] of the matched contacts or
 *          %NULL with @error set on error.
 *
 * Since: 3.0.0
 */
GListModel *purple_protocol_contact_search_search_finish(PurpleProtocolContactSearch *search, GAsyncResult *result, GError **error);

G_END_DECLS

#endif /* PURPLE_PROTOCOL_CONTACT_SEARCH_H */
