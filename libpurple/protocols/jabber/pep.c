/*
 * purple - Jabber Protocol Plugin
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <glib/gi18n-lib.h>

#include "pep.h"
#include "iq.h"
#include <string.h>
#include "useravatar.h"
#include "usernick.h"

static GHashTable *pep_handlers = NULL;

void jabber_pep_init(void) {
	if(!pep_handlers) {
		pep_handlers = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);

		/* register PEP handlers */
		jabber_avatar_init();
		jabber_nick_init();
	}
}

void jabber_pep_uninit(void) {
	/* any PEP handlers that need to clean things up go here. The standard
	 * cleanup of removing the handler and feature are handled here and by
	 * jabber_features_destroy() in jabber.c
	 */
	g_clear_pointer(&pep_handlers, g_hash_table_destroy);
}

void
jabber_pep_add_action_entries(GSimpleActionGroup *group) {
	jabber_nick_add_action_entries(group);
}

void
jabber_pep_append_menu(GMenu *menu) {
	/* register the PEP-specific actions */
	jabber_nick_append_menu(menu);
}

void jabber_pep_register_handler(const char *xmlns, JabberPEPHandler handlerfunc) {
	gchar *notifyns = g_strdup_printf("%s+notify", xmlns);
	jabber_add_feature(notifyns, NULL); /* receiving PEPs is always supported */
	g_free(notifyns);
	g_hash_table_replace(pep_handlers, g_strdup(xmlns), handlerfunc);
}

static void
do_pep_iq_request_item_callback(JabberStream *js, const char *from,
                                JabberIqType type,
                                G_GNUC_UNUSED const char *id,
                                PurpleXmlNode *packet, gpointer data)
{
	PurpleXmlNode *pubsub;
	PurpleXmlNode *items = NULL;
	JabberPEPHandler *cb = data;

	if (type == JABBER_IQ_RESULT) {
		pubsub = purple_xmlnode_get_child_with_namespace(packet, "pubsub", "http://jabber.org/protocol/pubsub");
		if(pubsub)
			items = purple_xmlnode_get_child(pubsub, "items");
	}

	cb(js, from, items);
}

void jabber_pep_request_item(JabberStream *js, const char *to, const char *node, const char *id, JabberPEPHandler cb) {
	JabberIq *iq = jabber_iq_new(js, JABBER_IQ_GET);
	PurpleXmlNode *pubsub, *items;

	if (to)
		purple_xmlnode_set_attrib(iq->node, "to", to);

	pubsub = purple_xmlnode_new_child(iq->node,"pubsub");
	purple_xmlnode_set_namespace(pubsub, "http://jabber.org/protocol/pubsub");

	items = purple_xmlnode_new_child(pubsub, "items");
	purple_xmlnode_set_attrib(items,"node",node);

	if (id) {
		PurpleXmlNode *item = purple_xmlnode_new_child(items, "item");
		purple_xmlnode_set_attrib(item, "id", id);
	} else
		/* Most recent item */
		purple_xmlnode_set_attrib(items, "max_items", "1");

	jabber_iq_set_callback(iq,do_pep_iq_request_item_callback,(gpointer)cb);

	jabber_iq_send(iq);
}

gboolean
jabber_pep_namespace_only_when_pep_enabled_cb(JabberStream *js,
                                              G_GNUC_UNUSED const char *namespace)
{
	return js->pep;
}

void jabber_handle_event(JabberMessage *jm) {
	/* this may be called even when the own server doesn't support pep! */
	JabberPEPHandler *jph;
	GList *itemslist;
	char *jid;

	if (jm->type != JABBER_MESSAGE_EVENT)
		return;

	jid = jabber_get_bare_jid(jm->from);

	for(itemslist = jm->eventitems; itemslist; itemslist = itemslist->next) {
		PurpleXmlNode *items = (PurpleXmlNode*)itemslist->data;
		const char *nodename = purple_xmlnode_get_attrib(items,"node");

		if(nodename && (jph = g_hash_table_lookup(pep_handlers, nodename)))
			jph(jm->js, jid, items);
	}

	/* discard items we don't have a handler for */
	g_free(jid);
}

void jabber_pep_publish(JabberStream *js, PurpleXmlNode *publish) {
	JabberIq *iq;
	PurpleXmlNode *pubsub;

	if(js->pep != TRUE) {
		/* ignore when there's no PEP support on the server */
		purple_xmlnode_free(publish);
		return;
	}

	iq = jabber_iq_new(js, JABBER_IQ_SET);

	pubsub = purple_xmlnode_new("pubsub");
	purple_xmlnode_set_namespace(pubsub, "http://jabber.org/protocol/pubsub");

	purple_xmlnode_insert_child(pubsub, publish);

	purple_xmlnode_insert_child(iq->node, pubsub);

	jabber_iq_send(iq);
}
