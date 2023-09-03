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

#include "purpleprotocolroster.h"

/******************************************************************************
 * GInterface Implementation
 *****************************************************************************/
G_DEFINE_INTERFACE(PurpleProtocolRoster, purple_protocol_roster,
                   PURPLE_TYPE_PROTOCOL)

static void
purple_protocol_roster_default_init(G_GNUC_UNUSED PurpleProtocolRosterInterface *iface)
{
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_protocol_roster_add_async(PurpleProtocolRoster *roster,
                                 PurpleAccount *account,
                                 PurpleContact *contact,
                                 GCancellable *cancellable,
                                 GAsyncReadyCallback callback,
                                 gpointer data)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster));
	g_return_if_fail(PURPLE_IS_CONTACT(contact));

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->add_async != NULL) {
		iface->add_async(roster, account, contact, cancellable, callback,
		                 data);
	} else {
		g_warning("%s does not implement the add_async method",
		          G_OBJECT_TYPE_NAME(roster));
	}
}

gboolean
purple_protocol_roster_add_finish(PurpleProtocolRoster *roster,
                                  GAsyncResult *result, GError **error)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster), FALSE);

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->add_finish != NULL) {
		return iface->add_finish(roster, result, error);
	}

	return FALSE;
}

void
purple_protocol_roster_update_async(PurpleProtocolRoster *roster,
                                    PurpleAccount *account,
                                    PurpleContact *contact,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback,
                                    gpointer data)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster));
	g_return_if_fail(PURPLE_IS_CONTACT(contact));

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->update_async != NULL) {
		iface->update_async(roster, account, contact, cancellable, callback,
		                    data);
	} else {
		g_warning("%s does not implement the update_async method",
		          G_OBJECT_TYPE_NAME(roster));
	}
}

gboolean
purple_protocol_roster_update_finish(PurpleProtocolRoster *roster,
                                     GAsyncResult *result, GError **error)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster), FALSE);

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->update_finish != NULL) {
		return iface->update_finish(roster, result, error);
	}

	return FALSE;
}

void
purple_protocol_roster_remove_async(PurpleProtocolRoster *roster,
                                    PurpleAccount *account,
                                    PurpleContact *contact,
                                    GCancellable *cancellable,
                                    GAsyncReadyCallback callback,
                                    gpointer data)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster));
	g_return_if_fail(PURPLE_IS_CONTACT(contact));

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->remove_async != NULL) {
		iface->remove_async(roster, account, contact, cancellable, callback,
		                    data);
	} else {
		g_warning("%s does not implement the remove_async method",
		          G_OBJECT_TYPE_NAME(roster));
	}
}

gboolean
purple_protocol_roster_remove_finish(PurpleProtocolRoster *roster,
                                     GAsyncResult *result, GError **error)
{
	PurpleProtocolRosterInterface *iface = NULL;

	g_return_val_if_fail(PURPLE_IS_PROTOCOL_ROSTER(roster), FALSE);

	iface = PURPLE_PROTOCOL_ROSTER_GET_IFACE(roster);
	if(iface != NULL && iface->remove_finish != NULL) {
		return iface->remove_finish(roster, result, error);
	}

	return FALSE;
}
