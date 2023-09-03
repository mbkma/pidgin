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

#include "purpleprotocolcontacts.h"

/******************************************************************************
 * GInterface Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolContacts, purple_protocol_contacts,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_contacts_default_init(G_GNUC_UNUSED PurpleProtocolContactsInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_protocol_contacts_get_profile_async(PurpleProtocolContacts *protocol_contacts,
                                           PurpleContactInfo *info,
                                           GCancellable *cancellable,
                                           GAsyncReadyCallback callback,
                                           gpointer data)
{
	PurpleProtocolContactsInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_CONTACTS(protocol_contacts));
	g_return_if_fail(PURPLE_IS_CONTACT_INFO(info));

	iface = PURPLE_PROTOCOL_CONTACTS_GET_IFACE(protocol_contacts);
	if(iface != NULL && iface->get_profile_async != NULL) {
		iface->get_profile_async(protocol_contacts, info, cancellable,
		                         callback, data);
	} else {
		g_warning("%s does not implement get_profile_async",
		          G_OBJECT_TYPE_NAME(protocol_contacts));
	}
}

char *
purple_protocol_contacts_get_profile_finish(PurpleProtocolContacts *protocol_contacts,
                                            GAsyncResult *result,
                                            GError **error)
{
	PurpleProtocolContactsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CONTACTS(protocol_contacts), NULL);

	iface = PURPLE_PROTOCOL_CONTACTS_GET_IFACE(protocol_contacts);
	if(iface != NULL && iface->get_profile_finish != NULL) {
		return iface->get_profile_finish(protocol_contacts, result, error);
	} else {
		g_warning("%s does not implement get_profile_finish",
		          G_OBJECT_TYPE_NAME(protocol_contacts));
	}

	return NULL;
}

GActionGroup *
purple_protocol_contacts_get_actions(PurpleProtocolContacts *protocol_contacts,
                                     PurpleContactInfo *info)
{
	PurpleProtocolContactsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CONTACTS(protocol_contacts), NULL);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	iface = PURPLE_PROTOCOL_CONTACTS_GET_IFACE(protocol_contacts);
	if(iface != NULL && iface->get_actions != NULL) {
		return iface->get_actions(protocol_contacts, info);
	}

	return NULL;
}

GMenuModel *
purple_protocol_contacts_get_menu(PurpleProtocolContacts *protocol_contacts,
                                  PurpleContactInfo *info)
{
	PurpleProtocolContactsInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_CONTACTS(protocol_contacts), NULL);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	iface = PURPLE_PROTOCOL_CONTACTS_GET_IFACE(protocol_contacts);
	if(iface != NULL && iface->get_menu != NULL) {
		return iface->get_menu(protocol_contacts, info);
	}

	return NULL;
}
