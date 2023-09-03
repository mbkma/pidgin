/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301, USA.
 */

#include <glib.h>
#include <stdlib.h>

#include <purple.h>

#include "buddy.h"
#include "bonjour.h"
#include "glibcompat.h"

/**
 * Creates a new buddy.
 */
BonjourBuddy *
bonjour_buddy_new(const gchar *name, PurpleAccount* account)
{
	BonjourBuddy *buddy = g_new0(BonjourBuddy, 1);

	buddy->account = account;
	buddy->name = g_strdup(name);

	_mdns_init_buddy(buddy);

	return buddy;
}

void
clear_bonjour_buddy_values(BonjourBuddy *buddy)
{
	g_clear_pointer(&buddy->first, g_free);
	g_clear_pointer(&buddy->email, g_free);
	g_clear_pointer(&buddy->ext, g_free);
	g_clear_pointer(&buddy->jid, g_free);
	g_clear_pointer(&buddy->last, g_free);
	g_clear_pointer(&buddy->msg, g_free);
	g_clear_pointer(&buddy->nick, g_free);
	g_clear_pointer(&buddy->node, g_free);
	g_clear_pointer(&buddy->phsh, g_free);
	g_clear_pointer(&buddy->status, g_free);
	g_clear_pointer(&buddy->vc, g_free);
	g_clear_pointer(&buddy->ver, g_free);
	g_clear_pointer(&buddy->AIM, g_free);
}

void
set_bonjour_buddy_value(BonjourBuddy* buddy, const char *record_key, const char *value, guint32 len){
	gchar **fld = NULL;

	g_return_if_fail(record_key != NULL);

	if (purple_strequal(record_key, "1st"))
		fld = &buddy->first;
	else if(purple_strequal(record_key, "email"))
		fld = &buddy->email;
	else if(purple_strequal(record_key, "ext"))
		fld = &buddy->ext;
	else if(purple_strequal(record_key, "jid"))
		fld = &buddy->jid;
	else if(purple_strequal(record_key, "last"))
		fld = &buddy->last;
	else if(purple_strequal(record_key, "msg"))
		fld = &buddy->msg;
	else if(purple_strequal(record_key, "nick"))
		fld = &buddy->nick;
	else if(purple_strequal(record_key, "node"))
		fld = &buddy->node;
	else if(purple_strequal(record_key, "phsh"))
		fld = &buddy->phsh;
	else if(purple_strequal(record_key, "status"))
		fld = &buddy->status;
	else if(purple_strequal(record_key, "vc"))
		fld = &buddy->vc;
	else if(purple_strequal(record_key, "ver"))
		fld = &buddy->ver;
	else if(purple_strequal(record_key, "AIM"))
		fld = &buddy->AIM;

	if(fld == NULL)
		return;

	g_free(*fld);
	*fld = NULL;
	*fld = g_strndup(value, len);
}

/**
 * Check if all the compulsory buddy data is present.
 */
gboolean
bonjour_buddy_check(BonjourBuddy *buddy)
{
	if (buddy->account == NULL)
		return FALSE;

	if (buddy->name == NULL)
		return FALSE;

	return TRUE;
}

/**
 * If the buddy does not yet exist, then create it and add it to
 * our buddy list.  In either case we set the correct status for
 * the buddy.
 */
void
bonjour_buddy_add_to_purple(BonjourBuddy *bonjour_buddy)
{
	PurpleAccount *account = bonjour_buddy->account;
	PurpleContact *contact = NULL;
	PurpleContactManager *manager = NULL;
	PurplePerson *person = NULL;
	PurplePresence *presence = NULL;
	PurplePresencePrimitive primitive = PURPLE_PRESENCE_PRIMITIVE_AVAILABLE;

	/* Translate between the Bonjour status and the Purple status */
	if(bonjour_buddy->status != NULL &&
	   g_ascii_strcasecmp("dnd", bonjour_buddy->status) == 0)
	{
		primitive = PURPLE_PRESENCE_PRIMITIVE_AWAY;
	}

	/*
	 * TODO: Figure out the idle time by getting the "away"
	 * field from the DNS SD.
	 */

	/* Determine if we already know about this contact. */
	manager = purple_contact_manager_get_default();
	contact = purple_contact_manager_find_with_username(manager, account,
	                                                    bonjour_buddy->name);

	/* If not, create the contact and add them to the manager. */
	if(!PURPLE_IS_CONTACT(contact)) {
		PurpleTags *tags = NULL;

		contact = purple_contact_new(account, NULL);
		purple_contact_manager_add(manager, contact);

		purple_contact_info_set_username(PURPLE_CONTACT_INFO(contact),
		                                 bonjour_buddy->name);

		tags = purple_contact_info_get_tags(PURPLE_CONTACT_INFO(contact));
		purple_tags_add_with_value(tags, "group", BONJOUR_GROUP_NAME);
	}

	/* Make sure we have a person for this contact as we want them to appear in
	 * the contact list.
	 */
	person = purple_contact_info_get_person(PURPLE_CONTACT_INFO(contact));
	if(!PURPLE_IS_PERSON(person)) {
		person = g_object_new(PURPLE_TYPE_PERSON, "id", bonjour_buddy->name,
		                      NULL);
		purple_contact_info_set_person(PURPLE_CONTACT_INFO(contact), person);
		purple_person_add_contact_info(person, PURPLE_CONTACT_INFO(contact));

		/* We remove our reference as the pointer is valid and we're treating
		 * it just like if we had called _get_person above but with the new
		 * instance.
		 */
		g_object_unref(person);
	}

	/* Set the alias. */
	if(!purple_strempty(bonjour_buddy->nick)) {
		purple_contact_info_set_alias(PURPLE_CONTACT_INFO(contact),
		                              bonjour_buddy->nick);
	} else {
		GStrvBuilder *builder = NULL;
		GStrv parts = NULL;
		gchar *alias = NULL;

		builder = g_strv_builder_new();

		if(!purple_strempty(bonjour_buddy->first)) {
			g_strv_builder_add(builder, bonjour_buddy->first);
		}

		if(!purple_strempty(bonjour_buddy->last)) {
			g_strv_builder_add(builder, bonjour_buddy->last);
		}

		parts = g_strv_builder_end(builder);

		alias = g_strjoinv(" ", parts);
		g_strfreev(parts);

		if(!purple_strempty(alias)) {
			purple_contact_info_set_alias(PURPLE_CONTACT_INFO(contact), alias);
		} else {
			purple_contact_info_set_alias(PURPLE_CONTACT_INFO(contact), NULL);
		}
		g_free(alias);
	}

	g_object_set_data(G_OBJECT(contact), "bonjour-buddy", bonjour_buddy);

	/* Set the user's status */
	presence = purple_contact_info_get_presence(PURPLE_CONTACT_INFO(contact));
	purple_presence_set_primitive(presence, primitive);
	purple_presence_set_message(presence, bonjour_buddy->msg);
	purple_presence_set_idle(presence, FALSE, NULL);

	/* TODO: Because we don't save Bonjour buddies in blist.xml,
	 * we will always have to look up the buddy icon at login time.
	 * I think we should figure out a way to do something about this. */

#if 0
	/* Deal with the buddy icon */
	old_hash = purple_buddy_icons_get_checksum_for_user(buddy);
	new_hash = (bonjour_buddy->phsh && *(bonjour_buddy->phsh)) ? bonjour_buddy->phsh : NULL;
	if (new_hash && !purple_strequal(old_hash, new_hash)) {
		/* Look up the new icon data */
		/* TODO: Make sure the hash assigned to the retrieved buddy icon is the same
		 * as what we looked up. */
		bonjour_dns_sd_retrieve_buddy_icon(bonjour_buddy);
	} else if (!new_hash)
		purple_buddy_icons_set_for_user(account, name, NULL, 0, NULL);
#endif

	g_clear_object(&contact);
}

/**
 * The buddy has signed off Bonjour.
 * If the buddy is being saved, mark as offline, otherwise delete
 */
void bonjour_buddy_signed_off(PurpleContact *contact) {
	BonjourBuddy *bb = NULL;
	PurplePresence *presence = NULL;

	presence = purple_contact_info_get_presence(PURPLE_CONTACT_INFO(contact));
	purple_presence_set_primitive(presence, PURPLE_PRESENCE_PRIMITIVE_OFFLINE);

	bb = g_object_get_data(G_OBJECT(contact), "bonjour-buddy");
	if(bb != NULL) {
		bonjour_buddy_delete(bb);
	}
	g_object_set_data(G_OBJECT(contact), "bonjour-buddy", NULL);
}

/**
 * We got the buddy icon data; deal with it
 */
void bonjour_buddy_got_buddy_icon(BonjourBuddy *buddy, gconstpointer data, gsize len) {
	/* Recalculate the hash instead of using the current phsh to make sure it is accurate for the icon. */
	gchar *hash;

	if (data == NULL || len == 0)
		return;

	hash = g_compute_checksum_for_data(G_CHECKSUM_SHA1, data, len);

	purple_debug_info("bonjour", "Got buddy icon for %s icon hash='%s' phsh='%s'.\n", buddy->name,
			  hash, buddy->phsh ? buddy->phsh : "(null)");

	purple_buddy_icons_set_for_user(buddy->account, buddy->name,
		g_memdup2(data, len), len, hash);

	g_free(hash);
}

/**
 * Deletes a buddy from memory.
 */
void
bonjour_buddy_delete(BonjourBuddy *buddy)
{
	g_free(buddy->name);
	g_slist_free_full(buddy->ips, g_free);
	g_free(buddy->first);
	g_free(buddy->phsh);
	g_free(buddy->status);
	g_free(buddy->email);
	g_free(buddy->last);
	g_free(buddy->jid);
	g_free(buddy->AIM);
	g_free(buddy->vc);
	g_free(buddy->msg);
	g_free(buddy->ext);
	g_free(buddy->nick);
	g_free(buddy->node);
	g_free(buddy->ver);

	bonjour_xmpp_close_conversation(buddy->conversation);
	buddy->conversation = NULL;

	/* Clean up any mdns implementation data */
	_mdns_delete_buddy(buddy);

	g_free(buddy);
}
