/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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

#include <glib/gi18n-lib.h>

#include "purpleircv3messagehandlers.h"

#include "purpleircv3connection.h"
#include "purpleircv3core.h"

/******************************************************************************
 * Fallback
 *****************************************************************************/
gboolean
purple_ircv3_message_handler_fallback(G_GNUC_UNUSED GHashTable *tags,
                                      const char *source,
                                      const char *command,
                                      guint n_params,
                                      GStrv params,
                                      G_GNUC_UNUSED GError **error,
                                      gpointer data)
{
	PurpleIRCv3Connection *connection = data;
	char *new_command = NULL;

	new_command = g_strdup_printf(_("unknown command '%s'"), command);
	purple_ircv3_connection_add_status_message(connection, source, new_command,
	                                           n_params, params);

	g_clear_pointer(&new_command, g_free);

	return TRUE;
}

/******************************************************************************
 * Status Messages
 *****************************************************************************/
gboolean
purple_ircv3_message_handler_status(G_GNUC_UNUSED GHashTable *tags,
                                    const char *source,
                                    const char *command,
                                    guint n_params,
                                    GStrv params,
                                    G_GNUC_UNUSED GError **error,
                                    gpointer data)
{
	purple_ircv3_connection_add_status_message(data, source, command, n_params,
	                                           params);

	return TRUE;
}

gboolean
purple_ircv3_message_handler_status_ignore_param0(G_GNUC_UNUSED GHashTable *tags,
                                                  const char *source,
                                                  const char *command,
                                                  guint n_params,
                                                  GStrv params,
                                                  GError **error,
                                                  gpointer data)
{
	if(n_params <= 1) {
		g_set_error(error, PURPLE_IRCV3_DOMAIN, 0,
		            "expected n_params > 1, got %u", n_params);

		return FALSE;
	}

	purple_ircv3_connection_add_status_message(data, source, command,
	                                           n_params - 1, params + 1);

	return TRUE;
}

/******************************************************************************
 * General Commands
 *****************************************************************************/
gboolean
purple_ircv3_message_handler_ping(G_GNUC_UNUSED GHashTable *tags,
                                  G_GNUC_UNUSED const char *source,
                                  G_GNUC_UNUSED const char *command,
                                  guint n_params,
                                  GStrv params,
                                  G_GNUC_UNUSED GError **error,
                                  gpointer data)
{
	PurpleIRCv3Connection *connection = data;

	if(n_params == 1) {
		purple_ircv3_connection_writef(connection, "PONG %s", params[0]);
	} else {
		purple_ircv3_connection_writef(connection, "PONG");
	}

	return TRUE;
}

gboolean
purple_ircv3_message_handler_privmsg(GHashTable *tags,
                                     const char *source,
                                     const char *command,
                                     guint n_params,
                                     GStrv params,
                                     G_GNUC_UNUSED GError **error,
                                     gpointer data)
{
	PurpleIRCv3Connection *connection = data;
	PurpleAccount *account = NULL;
	PurpleContact *contact = NULL;
	PurpleContactManager *contact_manager = NULL;
	PurpleConversation *conversation = NULL;
	PurpleConversationManager *conversation_manager = NULL;
	PurpleMessage *message = NULL;
	PurpleMessageFlags flags = PURPLE_MESSAGE_RECV;
	GDateTime *dt = NULL;
	gpointer raw_id = NULL;
	gpointer raw_timestamp = NULL;
	const char *id = NULL;
	const char *target = NULL;

	if(n_params != 2) {
		char *body = g_strjoinv(" ", params);
		g_warning("unknown privmsg message format: '%s'", body);
		g_free(body);

		return FALSE;
	}

	account = purple_connection_get_account(PURPLE_CONNECTION(connection));

	contact_manager = purple_contact_manager_get_default();
	contact = purple_contact_manager_find_with_username(contact_manager,
	                                                    account,
	                                                    source);
	if(!PURPLE_IS_CONTACT(contact)) {
		contact = purple_contact_new(account, NULL);
		purple_contact_info_set_username(PURPLE_CONTACT_INFO(contact), source);
		purple_contact_manager_add(contact_manager, contact);
	}
	g_clear_object(&contact);

	target = params[0];
	conversation_manager = purple_conversation_manager_get_default();
	conversation = purple_conversation_manager_find(conversation_manager,
	                                                account, target);
	if(!PURPLE_IS_CONVERSATION(conversation)) {
		if(target[0] == '#') {
			conversation = purple_chat_conversation_new(account, target);
		} else {
			conversation = purple_im_conversation_new(account, source);
		}

		purple_conversation_manager_register(conversation_manager,
		                                     conversation);

		/* The manager creates its own reference on our new conversation, so we
		 * borrow it like we do above if it already exists.
		 */
		g_object_unref(conversation);
	}

	/* Grab the msgid if one was provided. */
	if(g_hash_table_lookup_extended(tags, "msgid", NULL, &raw_id)) {
		if(!purple_strempty(raw_id)) {
			id = raw_id;
		}
	}

	if(purple_strequal(command, "NOTICE")) {
		flags |= PURPLE_MESSAGE_NOTIFY;
	}

	/* Determine the timestamp of the message. */
	if(g_hash_table_lookup_extended(tags, "time", NULL, &raw_timestamp)) {
		const char *timestamp = raw_timestamp;

		if(!purple_strempty(timestamp)) {
			GTimeZone *tz = g_time_zone_new_utc();

			dt = g_date_time_new_from_iso8601(timestamp, tz);

			g_time_zone_unref(tz);
		}
	}

	/* If the server didn't provide a time, use the current local time. */
	if(dt == NULL) {
		dt = g_date_time_new_now_local();
	}

	message = g_object_new(
		PURPLE_TYPE_MESSAGE,
		"author", source,
		"contents", params[1],
		"flags", flags,
		"id", id,
		"timestamp", dt,
		NULL);

	g_date_time_unref(dt);

	purple_conversation_write_message(conversation, message);

	g_clear_object(&message);

	return TRUE;
}
