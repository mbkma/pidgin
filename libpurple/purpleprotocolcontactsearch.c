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

#include "purpleprotocolcontactsearch.h"

/******************************************************************************
 * GInterface Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolContactSearch, purple_protocol_contact_search,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_contact_search_default_init(G_GNUC_UNUSED PurpleProtocolContactSearchInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_protocol_contact_search_search_async(PurpleProtocolContactSearch *search,
                                            PurpleAccount *account,
                                            const char *text,
                                            GCancellable *cancellable,
                                            GAsyncReadyCallback callback,
                                            gpointer data)
{
	PurpleProtocolContactSearchInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CONTACT_SEARCH(search));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));
	g_return_if_fail(!purple_strempty(text));

	iface = PURPLE_PROTOCOL_CONTACT_SEARCH_GET_IFACE(search);
	if(iface != NULL && iface->search_async != NULL) {
		iface->search_async(search, account, text, cancellable, callback,
		                    data);
	} else {
		g_warning("%s does not implement search_async",
		          G_OBJECT_TYPE_NAME(search));
	}
}

GListModel *
purple_protocol_contact_search_search_finish(PurpleProtocolContactSearch *search,
                                             GAsyncResult *result,
                                             GError **error)
{
	PurpleProtocolContactSearchInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CONTACT_SEARCH(search), NULL);

	iface = PURPLE_PROTOCOL_CONTACT_SEARCH_GET_IFACE(search);
	if(iface != NULL && iface->search_finish != NULL) {
		GListModel *ret = iface->search_finish(search, result, error);

		if(G_IS_LIST_MODEL(ret)) {
			GType type = G_TYPE_INVALID;

			type = g_list_model_get_item_type(G_LIST_MODEL(ret));
			if(g_type_is_a(type, PURPLE_TYPE_CONTACT_INFO)) {
				return ret;
			}

			/* The GListModel we got back doesn't have an item type that is
			 * PurpleContactInfo or a subclass of it.
			 */
			g_clear_object(&ret);

			g_set_error(error, PURPLE_PROTOCOL_CONTACT_SEARCH_DOMAIN, 0,
			            "%s returned a list of type %s which is not "
			            "PurpleContactInfo or a a subclass of",
			            G_OBJECT_TYPE_NAME(search), g_type_name(type));
		}
	} else {
		g_warning("%s does not implement search_finish",
		          G_OBJECT_TYPE_NAME(search));
	}

	return NULL;
}
