/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib/gi18n-lib.h>

#include "purpleconversation.h"

#include "conversations.h"
#include "debug.h"
#include "purpleconversationmanager.h"
#include "purpleconversationmember.h"
#include "purpleenums.h"
#include "purplehistorymanager.h"
#include "purplemarkup.h"
#include "purpleprivate.h"
#include "purpletags.h"
#include "server.h"

typedef struct {
	char *id;
	PurpleAccount *account;

	char *name;
	char *title;

	PurpleConversationUiOps *ui_ops;

	PurpleConnectionFlags features;

	gboolean age_restricted;
	char *description;
	char *topic;
	PurpleContactInfo *topic_author;
	GDateTime *topic_updated;
	char *user_nickname;
	gboolean favorite;
	GDateTime *created_on;
	PurpleContactInfo *creator;
	PurpleTags *tags;
	GListStore *members;

	GListStore *messages;
} PurpleConversationPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_ACCOUNT,
	PROP_NAME,
	PROP_TITLE,
	PROP_FEATURES,
	PROP_AGE_RESTRICTED,
	PROP_DESCRIPTION,
	PROP_TOPIC,
	PROP_TOPIC_AUTHOR,
	PROP_TOPIC_UPDATED,
	PROP_USER_NICKNAME,
	PROP_FAVORITE,
	PROP_CREATED_ON,
	PROP_CREATOR,
	PROP_TAGS,
	PROP_MEMBERS,
	PROP_MESSAGES,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES] = { NULL, };

enum {
	SIG_MEMBER_ADDED,
	SIG_MEMBER_REMOVED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS] = {0, };

G_DEFINE_TYPE_WITH_PRIVATE(PurpleConversation, purple_conversation,
                           G_TYPE_OBJECT);

/**************************************************************************
 * Helpers
 **************************************************************************/
static void
purple_conversation_set_id(PurpleConversation *conversation, const char *id) {
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(!purple_strequal(id, priv->id)) {
		g_free(priv->id);
		priv->id = g_strdup(id);

		g_object_notify_by_pspec(G_OBJECT(conversation), properties[PROP_ID]);
	}
}

static gboolean
purple_conversation_check_member_equal(gconstpointer a, gconstpointer b) {
	PurpleConversationMember *member_a = (PurpleConversationMember *)a;
	PurpleConversationMember *member_b = (PurpleConversationMember *)b;
	PurpleContactInfo *info_a = NULL;
	PurpleContactInfo *info_b = NULL;

	info_a = purple_conversation_member_get_contact_info(member_a);
	info_b = purple_conversation_member_get_contact_info(member_b);

	return (purple_contact_info_compare(info_a, info_b) == 0);
}

static void
common_send(PurpleConversation *conv, const gchar *message,
            PurpleMessageFlags msgflags)
{
	PurpleAccount *account;
	PurpleConnection *gc;
	PurpleConversationPrivate *priv = NULL;
	gchar *displayed = NULL;
	const gchar *sent, *me;
	gint err = 0;
	gpointer handle = NULL;

	if(*message == '\0') {
		return;
	}

	priv = purple_conversation_get_instance_private(conv);

	account = purple_conversation_get_account(conv);
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	gc = purple_account_get_connection(account);
	g_return_if_fail(PURPLE_IS_CONNECTION(gc));

	me = purple_contact_info_get_name_for_display(PURPLE_CONTACT_INFO(account));

	/* Always linkify the text for display, unless we're explicitly asked to do
	 * otherwise. */
	if(!(msgflags & PURPLE_MESSAGE_INVISIBLE)) {
		if(msgflags & PURPLE_MESSAGE_NO_LINKIFY) {
			displayed = g_strdup(message);
		} else {
			displayed = purple_markup_linkify(message);
		}
	}

	if(displayed && (priv->features & PURPLE_CONNECTION_FLAG_HTML) &&
	   !(msgflags & PURPLE_MESSAGE_RAW))
	{
		sent = displayed;
	} else {
		sent = message;
	}

	msgflags |= PURPLE_MESSAGE_SEND;

	handle = purple_conversations_get_handle();

	if(PURPLE_IS_IM_CONVERSATION(conv)) {
		const gchar *name = NULL;
		PurpleMessage *msg = NULL;

		name = purple_conversation_get_name(conv);
		msg = purple_message_new_outgoing(me, name, sent, msgflags);

		purple_signal_emit(handle, "sending-im-msg", account, msg);

		if(!purple_message_is_empty(msg)) {
			err = purple_serv_send_im(gc, msg);

			if((err > 0) && (displayed != NULL)) {
				/* revert the contents in case sending-im-msg changed it */
				purple_message_set_contents(msg, displayed);
				purple_conversation_write_message(conv, msg);
			}

			purple_signal_emit(handle, "sent-im-msg", account, msg);
		}

		g_object_unref(G_OBJECT(msg));
	} else if(PURPLE_IS_CHAT_CONVERSATION(conv)) {
		PurpleMessage *msg;
		gint id = purple_chat_conversation_get_id(PURPLE_CHAT_CONVERSATION(conv));

		msg = purple_message_new_outgoing(me, NULL, sent, msgflags);

		purple_signal_emit(handle, "sending-chat-msg", account, msg, id);

		if(!purple_message_is_empty(msg)) {
			err = purple_serv_chat_send(gc, id, msg);

			purple_signal_emit(handle, "sent-chat-msg", account, msg, id);
		}

		g_object_unref(G_OBJECT(msg));
	}

	if(err < 0) {
		const gchar *who;
		const gchar *msg;

		who = purple_conversation_get_name(conv);

		if(err == -E2BIG) {
			msg = _("Unable to send message: The message is too large.");

			if(!purple_conversation_present_error(who, account, msg)) {
				gchar *msg2 = g_strdup_printf(_("Unable to send message to %s."),
				                              who);
				purple_notify_error(gc, NULL, msg2,
				                    _("The message is too large."),
				                    purple_request_cpar_from_connection(gc));
				g_free(msg2);
			}
		} else if(err == -ENOTCONN) {
			purple_debug_error("conversation", "Not yet connected.");
		} else {
			msg = _("Unable to send message.");

			if(!purple_conversation_present_error(who, account, msg)) {
				gchar *msg2 = g_strdup_printf(_("Unable to send message to %s."),
				                              who);
				purple_notify_error(gc, NULL, msg2, NULL,
				                    purple_request_cpar_from_connection(gc));
				g_free(msg2);
			}
		}
	}

	g_free(displayed);
}

static void
purple_conversation_send_confirm_cb(gpointer *data) {
	PurpleConversation *conv = data[0];
	gchar *message = data[1];

	g_free(data);

	if(!PURPLE_IS_CONVERSATION(conv)) {
		/* Maybe it was closed before this callback was called. */
		return;
	}

	common_send(conv, message, 0);
}

/**************************************************************************
 * GObject Implementation
 **************************************************************************/
static void
purple_conversation_set_property(GObject *obj, guint param_id,
                                 const GValue *value, GParamSpec *pspec)
{
	PurpleConversation *conv = PURPLE_CONVERSATION(obj);
	PurpleConversationPrivate *priv = NULL;

	priv = purple_conversation_get_instance_private(conv);

	switch (param_id) {
		case PROP_ID:
			purple_conversation_set_id(conv, g_value_get_string(value));
			break;
		case PROP_ACCOUNT:
			purple_conversation_set_account(conv, g_value_get_object(value));
			break;
		case PROP_NAME:
			g_free(priv->name);
			priv->name = g_value_dup_string(value);
			break;
		case PROP_TITLE:
			purple_conversation_set_title(conv, g_value_get_string(value));
			break;
		case PROP_FEATURES:
			purple_conversation_set_features(conv, g_value_get_flags(value));
			break;
		case PROP_AGE_RESTRICTED:
			purple_conversation_set_age_restricted(conv,
			                                       g_value_get_boolean(value));
			break;
		case PROP_DESCRIPTION:
			purple_conversation_set_description(conv,
			                                    g_value_get_string(value));
			break;
		case PROP_TOPIC:
			purple_conversation_set_topic(conv, g_value_get_string(value));
			break;
		case PROP_TOPIC_AUTHOR:
			purple_conversation_set_topic_author(conv,
			                                     g_value_get_object(value));
			break;
		case PROP_TOPIC_UPDATED:
			purple_conversation_set_topic_updated(conv,
			                                      g_value_get_boxed(value));
			break;
		case PROP_USER_NICKNAME:
			purple_conversation_set_user_nickname(conv,
			                                      g_value_get_string(value));
			break;
		case PROP_FAVORITE:
			purple_conversation_set_favorite(conv, g_value_get_boolean(value));
			break;
		case PROP_CREATED_ON:
			purple_conversation_set_created_on(conv, g_value_get_boxed(value));
			break;
		case PROP_CREATOR:
			purple_conversation_set_creator(conv, g_value_get_object(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_conversation_get_property(GObject *obj, guint param_id, GValue *value,
                                 GParamSpec *pspec)
{
	PurpleConversation *conv = PURPLE_CONVERSATION(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_conversation_get_id(conv));
			break;
		case PROP_ACCOUNT:
			g_value_set_object(value, purple_conversation_get_account(conv));
			break;
		case PROP_NAME:
			g_value_set_string(value, purple_conversation_get_name(conv));
			break;
		case PROP_TITLE:
			g_value_set_string(value, purple_conversation_get_title(conv));
			break;
		case PROP_FEATURES:
			g_value_set_flags(value, purple_conversation_get_features(conv));
			break;
		case PROP_AGE_RESTRICTED:
			g_value_set_boolean(value,
			                    purple_conversation_get_age_restricted(conv));
			break;
		case PROP_DESCRIPTION:
			g_value_set_string(value,
			                   purple_conversation_get_description(conv));
			break;
		case PROP_TOPIC:
			g_value_set_string(value, purple_conversation_get_topic(conv));
			break;
		case PROP_TOPIC_AUTHOR:
			g_value_set_object(value,
			                   purple_conversation_get_topic_author(conv));
			break;
		case PROP_TOPIC_UPDATED:
			g_value_set_boxed(value,
			                  purple_conversation_get_topic_updated(conv));
			break;
		case PROP_USER_NICKNAME:
			g_value_set_string(value,
			                   purple_conversation_get_user_nickname(conv));
			break;
		case PROP_FAVORITE:
			g_value_set_boolean(value, purple_conversation_get_favorite(conv));
			break;
		case PROP_CREATED_ON:
			g_value_set_boxed(value, purple_conversation_get_created_on(conv));
			break;
		case PROP_CREATOR:
			g_value_set_object(value, purple_conversation_get_creator(conv));
			break;
		case PROP_TAGS:
			g_value_set_object(value, purple_conversation_get_tags(conv));
			break;
		case PROP_MEMBERS:
			g_value_set_object(value, purple_conversation_get_members(conv));
			break;
		case PROP_MESSAGES:
			g_value_set_object(value, purple_conversation_get_messages(conv));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_conversation_init(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	priv = purple_conversation_get_instance_private(conv);

	priv->tags = purple_tags_new();
	priv->members = g_list_store_new(PURPLE_TYPE_CONVERSATION_MEMBER);
	priv->messages = g_list_store_new(PURPLE_TYPE_MESSAGE);
}

static void
purple_conversation_constructed(GObject *object) {
	PurpleConversation *conv = PURPLE_CONVERSATION(object);
	PurpleAccount *account;
	PurpleConnection *gc;
	PurpleConversationManager *manager;
	PurpleConversationUiOps *ops;

	G_OBJECT_CLASS(purple_conversation_parent_class)->constructed(object);

	g_object_get(object, "account", &account, NULL);
	gc = purple_account_get_connection(account);

	/* Check if we have a connection before we use it. The unit tests are one
	 * case where we will not have a connection.
	 */
	if(PURPLE_IS_CONNECTION(gc)) {
		purple_conversation_set_features(conv,
		                                 purple_connection_get_flags(gc));
	}

	/* add the conversation to the appropriate lists */
	manager = purple_conversation_manager_get_default();
	purple_conversation_manager_register(manager, conv);

	/* Auto-set the title. */
	purple_conversation_autoset_title(conv);

	/* Don't move this.. it needs to be one of the last things done otherwise
	 * it causes mysterious crashes on my system.
	 *  -- Gary
	 */
	ops  = purple_conversations_get_ui_ops();
	purple_conversation_set_ui_ops(conv, ops);
	if(ops != NULL && ops->create_conversation != NULL) {
		ops->create_conversation(conv);
	}

	purple_signal_emit(purple_conversations_get_handle(),
	                   "conversation-created", conv);

	g_object_unref(account);
}

static void
purple_conversation_dispose(GObject *obj) {
	g_object_set_data(obj, "is-finalizing", GINT_TO_POINTER(TRUE));
}

static void
purple_conversation_finalize(GObject *object) {
	PurpleConversation *conv = PURPLE_CONVERSATION(object);
	PurpleConversationManager *manager;
	PurpleConversationPrivate *priv =
			purple_conversation_get_instance_private(conv);
	PurpleConversationUiOps *ops  = purple_conversation_get_ui_ops(conv);

	purple_request_close_with_handle(conv);

	/* remove from conversations and im/chats lists prior to emit */
	manager = purple_conversation_manager_get_default();
	if(PURPLE_IS_CONVERSATION_MANAGER(manager)) {
		purple_conversation_manager_unregister(manager, conv);
	}

	purple_signal_emit(purple_conversations_get_handle(),
	                   "deleting-conversation", conv);

	if(ops != NULL && ops->destroy_conversation != NULL) {
		ops->destroy_conversation(conv);
	}

	g_clear_pointer(&priv->id, g_free);
	g_clear_pointer(&priv->name, g_free);
	g_clear_pointer(&priv->title, g_free);

	g_clear_pointer(&priv->description, g_free);
	g_clear_pointer(&priv->topic, g_free);
	g_clear_object(&priv->topic_author);
	g_clear_pointer(&priv->topic_updated, g_date_time_unref);
	g_clear_pointer(&priv->user_nickname, g_free);
	g_clear_pointer(&priv->created_on, g_date_time_unref);
	g_clear_object(&priv->creator);
	g_clear_object(&priv->tags);
	g_clear_object(&priv->members);
	g_clear_object(&priv->messages);

	G_OBJECT_CLASS(purple_conversation_parent_class)->finalize(object);
}

static void
purple_conversation_class_init(PurpleConversationClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	klass->write_message = _purple_conversation_write_common;

	obj_class->constructed = purple_conversation_constructed;
	obj_class->dispose = purple_conversation_dispose;
	obj_class->finalize = purple_conversation_finalize;
	obj_class->get_property = purple_conversation_get_property;
	obj_class->set_property = purple_conversation_set_property;

	/**
	 * PurpleConversation::id:
	 *
	 * An opaque identifier for this conversation. Generally speaking this is
	 * protocol dependent and should only be used as a unique identifier.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The identifier for the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	properties[PROP_ACCOUNT] = g_param_spec_object(
		"account", "Account",
		"The account for the conversation.",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_NAME] = g_param_spec_string(
		"name", "Name",
		"The name of the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT | G_PARAM_STATIC_STRINGS);

	properties[PROP_TITLE] = g_param_spec_string(
		"title", "Title",
		"The title of the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	properties[PROP_FEATURES] = g_param_spec_flags(
		"features", "Connection features",
		"The connection features of the conversation.",
		PURPLE_TYPE_CONNECTION_FLAGS,
		0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:age-restricted:
	 *
	 * Whether or not the conversation is age restricted.
	 *
	 * This is typically set only by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_AGE_RESTRICTED] = g_param_spec_boolean(
		"age-restricted", "age-restricted",
		"Whether or not the conversation is age restricted.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:description:
	 *
	 * Sets the description of the conversation. This field is typically used
	 * to give more information about a conversation than that which would fit
	 * in [property@Conversation:topic].
	 *
	 * This is typically set only by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DESCRIPTION] = g_param_spec_string(
		"description", "description",
		"The description for the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:topic:
	 *
	 * The topic of the conversation.
	 *
	 * This is normally controlled by the protocol plugin and often times
	 * requires permission for the user to set.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TOPIC] = g_param_spec_string(
		"topic", "topic",
		"The topic for the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:topic-author:
	 *
	 * Sets the author of the topic for the conversation.
	 *
	 * This should typically only be set by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TOPIC_AUTHOR] = g_param_spec_object(
		"topic-author", "topic-author",
		"The author of the topic for the conversation.",
		PURPLE_TYPE_CONTACT_INFO,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:topic-updated:
	 *
	 * Set to the time that the topic was last updated.
	 *
	 * This should typically only be set by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TOPIC_UPDATED] = g_param_spec_boxed(
		"topic-updated", "topic-updated",
		"The time when the topic was last updated for the conversation.",
		G_TYPE_DATE_TIME,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:user-nickname:
	 *
	 * The user's nickname in this conversation.
	 *
	 * Some protocols allow the user to use a nickname rather than their normal
	 * contact information when joining a conversation. This field holds that
	 * value.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_USER_NICKNAME] = g_param_spec_string(
		"user-nickname", "user-nickname",
		"The nickname for the user in the conversation.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:favorite:
	 *
	 * Whether or not the conversation has been marked as favorite by the user.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_FAVORITE] = g_param_spec_boolean(
		"favorite", "favorite",
		"Whether or not the conversation is a favorite.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:created-on:
	 *
	 * The [struct@GLib.DateTime] when this conversation was created. This can
	 * be %NULL if the value is not known or supported.
	 *
	 * This should typically only be set by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CREATED_ON] = g_param_spec_boxed(
		"created-on", "created-on",
		"When the conversation was created.",
		G_TYPE_DATE_TIME,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:creator:
	 *
	 * The [class@ContactInfo] that created the conversation.
	 *
	 * This should typically only be set by a protocol plugin.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_CREATOR] = g_param_spec_object(
		"creator", "creator",
		"The contact info of who created the conversation.",
		PURPLE_TYPE_CONTACT_INFO,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:tags:
	 *
	 * [class@Tags] for the conversation.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TAGS] = g_param_spec_object(
		"tags", "tags",
		"The tags for the conversation.",
		PURPLE_TYPE_TAGS,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:members:
	 *
	 * The members that are currently in this conversation.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MEMBERS] = g_param_spec_object(
		"members", "members",
		"The members that are currently in this conversation",
		G_TYPE_LIST_MODEL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleConversation:messages:
	 *
	 * A [iface.Gio.ListModel] of all the messages in this conversation.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MESSAGES] = g_param_spec_object(
		"messages", "messages",
		"All of the messages in this conversation's history.",
		G_TYPE_LIST_MODEL,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);

	/**
	 * PurpleConversation::member-added:
	 * @conversation: The instance.
	 * @member: The [class@Purple.ConversationMember] instance.
	 * @announce: Whether or not this addition should be announced.
	 * @message: (nullable): An optional message to use in the announcement.
	 *
	 * Emitted when a new member is added to this conversation.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_MEMBER_ADDED] = g_signal_new_class_handler(
		"member-added",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		3,
		PURPLE_TYPE_CONVERSATION_MEMBER,
		G_TYPE_BOOLEAN,
		G_TYPE_STRING);

	/**
	 * PurpleConversation::member-removed:
	 * @conversation: The instance.
	 * @member: The [class@Purple.ConversationMember] instance.
	 * @announce: Whether or not this removal should be announced.
	 * @message: (nullable): An optional message to use in the announcement.
	 *
	 * Emitted when member is removed from this conversation.
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_MEMBER_REMOVED] = g_signal_new_class_handler(
		"member-removed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		3,
		PURPLE_TYPE_CONVERSATION_MEMBER,
		G_TYPE_BOOLEAN,
		G_TYPE_STRING);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_conversation_present(PurpleConversation *conv) {
	PurpleConversationUiOps *ops;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	ops = purple_conversation_get_ui_ops(conv);
	if(ops && ops->present) {
		ops->present(conv);
	}
}

void
purple_conversation_set_features(PurpleConversation *conv,
                                 PurpleConnectionFlags features)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	priv = purple_conversation_get_instance_private(conv);
	priv->features = features;

	g_object_notify_by_pspec(G_OBJECT(conv), properties[PROP_FEATURES]);

	purple_conversation_update(conv, PURPLE_CONVERSATION_UPDATE_FEATURES);
}

PurpleConnectionFlags
purple_conversation_get_features(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), 0);

	priv = purple_conversation_get_instance_private(conv);

	return priv->features;
}

void
purple_conversation_set_ui_ops(PurpleConversation *conv,
                               PurpleConversationUiOps *ops)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	priv = purple_conversation_get_instance_private(conv);

	if(priv->ui_ops == ops) {
		return;
	}

	if(priv->ui_ops != NULL && priv->ui_ops->destroy_conversation != NULL) {
		priv->ui_ops->destroy_conversation(conv);
	}

	priv->ui_ops = ops;
}

PurpleConversationUiOps *
purple_conversation_get_ui_ops(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	priv = purple_conversation_get_instance_private(conv);

	return priv->ui_ops;
}

const char *
purple_conversation_get_id(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->id;
}

void
purple_conversation_set_account(PurpleConversation *conv,
                                PurpleAccount *account)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	priv = purple_conversation_get_instance_private(conv);

	if(g_set_object(&priv->account, account)) {
		g_object_notify_by_pspec(G_OBJECT(conv), properties[PROP_ACCOUNT]);

		purple_conversation_update(conv, PURPLE_CONVERSATION_UPDATE_ACCOUNT);
	}
}

PurpleAccount *
purple_conversation_get_account(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	priv = purple_conversation_get_instance_private(conv);

	return priv->account;
}

PurpleConnection *
purple_conversation_get_connection(PurpleConversation *conv) {
	PurpleAccount *account;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	account = purple_conversation_get_account(conv);

	if(account == NULL) {
		return NULL;
	}

	return purple_account_get_connection(account);
}

void
purple_conversation_set_title(PurpleConversation *conv, const gchar *title) {
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	g_return_if_fail(title != NULL);

	priv = purple_conversation_get_instance_private(conv);
	g_free(priv->title);
	priv->title = g_strdup(title);

	if(!g_object_get_data(G_OBJECT(conv), "is-finalizing")) {
		g_object_notify_by_pspec(G_OBJECT(conv), properties[PROP_TITLE]);
	}

	purple_conversation_update(conv, PURPLE_CONVERSATION_UPDATE_TITLE);
}

const gchar *
purple_conversation_get_title(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	priv = purple_conversation_get_instance_private(conv);

	return priv->title;
}

void
purple_conversation_autoset_title(PurpleConversation *conv) {
	PurpleAccount *account;
	PurpleBuddy *b;
	PurpleChat *chat;
	const gchar *text = NULL, *name;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	account = purple_conversation_get_account(conv);
	name = purple_conversation_get_name(conv);

	if(PURPLE_IS_IM_CONVERSATION(conv)) {
		if(account && ((b = purple_blist_find_buddy(account, name)) != NULL)) {
			text = purple_buddy_get_contact_alias(b);
		}
	} else if(PURPLE_IS_CHAT_CONVERSATION(conv)) {
		if(account && ((chat = purple_blist_find_chat(account, name)) != NULL)) {
			text = purple_chat_get_name(chat);
		}
	}

	if(text == NULL) {
		text = name;
	}

	purple_conversation_set_title(conv, text);
}

void
purple_conversation_set_name(PurpleConversation *conv, const gchar *name) {
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	priv = purple_conversation_get_instance_private(conv);

	g_free(priv->name);
	priv->name = g_strdup(name);

	g_object_notify_by_pspec(G_OBJECT(conv), properties[PROP_NAME]);

	purple_conversation_autoset_title(conv);
	purple_conversation_update(conv, PURPLE_CONVERSATION_UPDATE_NAME);
}

const gchar *
purple_conversation_get_name(PurpleConversation *conv) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	priv = purple_conversation_get_instance_private(conv);

	return priv->name;
}

void
_purple_conversation_write_common(PurpleConversation *conv,
                                  PurpleMessage *pmsg)
{
	PurpleProtocol *protocol = NULL;
	PurpleConnection *gc = NULL;
	PurpleConversationPrivate *priv = NULL;
	PurpleAccount *account;
	PurpleConversationUiOps *ops;
	PurpleBuddy *b;
	gint plugin_return;
	/* int logging_font_options = 0; */

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	g_return_if_fail(pmsg != NULL);

	priv = purple_conversation_get_instance_private(conv);

	ops = purple_conversation_get_ui_ops(conv);

	account = purple_conversation_get_account(conv);

	if(account != NULL) {
		gc = purple_account_get_connection(account);
	}

	if(PURPLE_IS_CHAT_CONVERSATION(conv) && gc != NULL) {
		if(!g_slist_find(purple_connection_get_active_chats(gc), conv)) {
			return;
		}
	} else if(PURPLE_IS_IM_CONVERSATION(conv)) {
		PurpleConversationManager *manager = NULL;

		manager = purple_conversation_manager_get_default();
		if(!purple_conversation_manager_is_registered(manager, conv)) {
			return;
		}
	}

	plugin_return = GPOINTER_TO_INT(purple_signal_emit_return_1(
		purple_conversations_get_handle(),
		(PURPLE_IS_IM_CONVERSATION(conv) ? "writing-im-msg" : "writing-chat-msg"),
		conv, pmsg));

	if(purple_message_is_empty(pmsg)) {
		return;
	}

	if(plugin_return) {
		return;
	}

	if(account != NULL) {
		protocol = purple_account_get_protocol(account);

		if(PURPLE_IS_IM_CONVERSATION(conv) ||
		   !(protocol != NULL && purple_protocol_get_options(protocol) & OPT_PROTO_UNIQUE_CHATNAME))
		{
			if(purple_message_get_flags(pmsg) & PURPLE_MESSAGE_SEND) {
				PurpleContactInfo *info = PURPLE_CONTACT_INFO(account);
				const gchar *alias;

				alias = purple_contact_info_get_name_for_display(info);

				purple_message_set_author_alias(pmsg, alias);
			} else if (purple_message_get_flags(pmsg) & PURPLE_MESSAGE_RECV) {
				/* TODO: PurpleDude - folks not on the buddy list */
				b = purple_blist_find_buddy(account,
					purple_message_get_author(pmsg));

				if(b != NULL) {
					purple_message_set_author_alias(pmsg,
					                                purple_buddy_get_contact_alias(b));
				}
			}
		}
	}

	if(!(purple_message_get_flags(pmsg) & PURPLE_MESSAGE_NO_LOG))
	{
		GError *error = NULL;
		PurpleHistoryManager *manager = NULL;

		manager = purple_history_manager_get_default();
		/* We should probably handle this error somehow, but I don't think that
		 * spamming purple_debug_warning is necessarily the right call.
		 */
		if(!purple_history_manager_write(manager, conv, pmsg, &error)){
			purple_debug_info("conversation", "history manager write returned error: %s", error->message);

			g_clear_error(&error);
		}
	}

	g_list_store_append(priv->messages, pmsg);

	if(ops) {
		if (PURPLE_IS_CHAT_CONVERSATION(conv) && ops->write_chat) {
			ops->write_chat(PURPLE_CHAT_CONVERSATION(conv), pmsg);
		} else if (PURPLE_IS_IM_CONVERSATION(conv) && ops->write_im) {
			ops->write_im(PURPLE_IM_CONVERSATION(conv), pmsg);
		} else if (ops->write_conv) {
			ops->write_conv(conv, pmsg);
		}
	}

	purple_signal_emit(purple_conversations_get_handle(),
		(PURPLE_IS_IM_CONVERSATION(conv) ? "wrote-im-msg" : "wrote-chat-msg"),
		conv, pmsg);
}

void
purple_conversation_write_message(PurpleConversation *conv,
                                  PurpleMessage *msg)
{
	PurpleConversationClass *klass = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	klass = PURPLE_CONVERSATION_GET_CLASS(conv);

	if(klass && klass->write_message) {
		klass->write_message(conv, msg);
	}
}

void
purple_conversation_write_system_message(PurpleConversation *conv,
                                         const gchar *message,
                                         PurpleMessageFlags flags)
{
	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	_purple_conversation_write_common(conv,
	                                  purple_message_new_system(message,
	                                                            flags));
}

void
purple_conversation_send(PurpleConversation *conv, const gchar *message) {
	purple_conversation_send_with_flags(conv, message, 0);
}

void
purple_conversation_send_with_flags(PurpleConversation *conv,
                                    const gchar *message,
                                    PurpleMessageFlags flags)
{
	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	g_return_if_fail(message != NULL);

	common_send(conv, message, flags);
}

gboolean
purple_conversation_has_focus(PurpleConversation *conv) {
	gboolean ret = FALSE;
	PurpleConversationUiOps *ops;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), FALSE);

	ops = purple_conversation_get_ui_ops(conv);

	if(ops != NULL && ops->has_focus != NULL) {
		ret = ops->has_focus(conv);
	}

	return ret;
}

/*
 * TODO: Need to make sure calls to this function happen in the core
 * instead of the UI.  That way UIs have less work to do, and the
 * core/UI split is cleaner.  Also need to make sure this is called
 * when chats are added/removed from the blist.
 */
void
purple_conversation_update(PurpleConversation *conv,
                           PurpleConversationUpdateType type)
{
	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));

	purple_signal_emit(purple_conversations_get_handle(),
	                   "conversation-updated", conv, type);
}

gboolean
purple_conversation_present_error(const gchar *who, PurpleAccount *account,
                                  const gchar *what)
{
	PurpleConversation *conv;
	PurpleConversationManager *manager;

	g_return_val_if_fail(who != NULL, FALSE);
	g_return_val_if_fail(PURPLE_IS_ACCOUNT(account), FALSE);
	g_return_val_if_fail(what != NULL, FALSE);

	manager = purple_conversation_manager_get_default();
	conv = purple_conversation_manager_find(manager, account, who);
	if(PURPLE_IS_CONVERSATION(conv)) {
		purple_conversation_write_system_message(conv, what,
		                                         PURPLE_MESSAGE_ERROR);
		return TRUE;
	}

	return FALSE;
}

void
purple_conversation_send_confirm(PurpleConversation *conv,
                                 const gchar *message)
{
	PurpleConversationPrivate *priv = NULL;
	gchar *text;
	gpointer *data;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conv));
	g_return_if_fail(message != NULL);

	priv = purple_conversation_get_instance_private(conv);
	if(priv->ui_ops != NULL && priv->ui_ops->send_confirm != NULL) {
		priv->ui_ops->send_confirm(conv, message);
		return;
	}

	text = g_strdup_printf("You are about to send the following message:\n%s",
	                       message);
	data = g_new0(gpointer, 2);
	data[0] = conv;
	data[1] = (gpointer)message;

	purple_request_action(conv, NULL, _("Send Message"), text, 0,
		purple_request_cpar_from_account(
			purple_conversation_get_account(conv)),
		data, 2, _("_Send Message"),
		G_CALLBACK(purple_conversation_send_confirm_cb), _("Cancel"), NULL);
}

GList *
purple_conversation_get_extended_menu(PurpleConversation *conv) {
	GList *menu = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conv), NULL);

	purple_signal_emit(purple_conversations_get_handle(),
	                   "conversation-extended-menu", conv, &menu);

	return menu;
}

gboolean
purple_conversation_get_age_restricted(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->age_restricted;
}

void
purple_conversation_set_age_restricted(PurpleConversation *conversation,
                                       gboolean age_restricted)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(priv->age_restricted != age_restricted) {
		priv->age_restricted = age_restricted;

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_AGE_RESTRICTED]);
	}
}

const char *
purple_conversation_get_description(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->description;
}

void
purple_conversation_set_description(PurpleConversation *conversation,
                                    const char *description)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(!purple_strequal(priv->description, description)) {
		g_free(priv->description);
		priv->description = g_strdup(description);

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_DESCRIPTION]);
	}
}

const char *
purple_conversation_get_topic(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->topic;
}

void
purple_conversation_set_topic(PurpleConversation *conversation,
                              const char *topic)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(!purple_strequal(priv->topic, topic)) {
		g_free(priv->topic);
		priv->topic = g_strdup(topic);

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_TOPIC]);
	}
}

PurpleContactInfo *
purple_conversation_get_topic_author(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->topic_author;
}

void
purple_conversation_set_topic_author(PurpleConversation *conversation,
                                     PurpleContactInfo *author)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(g_set_object(&priv->topic_author, author)) {
		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_TOPIC_AUTHOR]);
	}
}

GDateTime *
purple_conversation_get_topic_updated(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->topic_updated;
}

void
purple_conversation_set_topic_updated(PurpleConversation *conversation,
                                      GDateTime *updated)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(priv->topic_updated != updated) {
		g_clear_pointer(&priv->topic_updated, g_date_time_unref);

		if(updated != NULL) {
			priv->topic_updated = g_date_time_ref(updated);
		}

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_TOPIC_UPDATED]);
	}
}

const char *
purple_conversation_get_user_nickname(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->user_nickname;
}

void
purple_conversation_set_user_nickname(PurpleConversation *conversation,
                                      const char *nickname)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(!purple_strequal(priv->user_nickname, nickname)) {
		g_free(priv->user_nickname);
		priv->user_nickname = g_strdup(nickname);

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_USER_NICKNAME]);
	}
}

gboolean
purple_conversation_get_favorite(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->favorite;
}

void
purple_conversation_set_favorite(PurpleConversation *conversation,
                                 gboolean favorite)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(priv->favorite != favorite) {
		priv->favorite = favorite;

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_FAVORITE]);
	}
}

GDateTime *
purple_conversation_get_created_on(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->created_on;
}

void
purple_conversation_set_created_on(PurpleConversation *conversation,
                                   GDateTime *created_on)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);

	if(priv->created_on != created_on) {
		g_clear_pointer(&priv->created_on, g_date_time_unref);

		if(created_on != NULL) {
			priv->created_on = g_date_time_ref(created_on);
		}

		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_CREATED_ON]);
	}
}

PurpleContactInfo *
purple_conversation_get_creator(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->creator;
}

void
purple_conversation_set_creator(PurpleConversation *conversation,
                                PurpleContactInfo *creator)
{
	PurpleConversationPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_CONVERSATION(conversation));

	priv = purple_conversation_get_instance_private(conversation);
	if(g_set_object(&priv->creator, creator)) {
		g_object_notify_by_pspec(G_OBJECT(conversation),
		                         properties[PROP_CREATOR]);
	}
}

PurpleTags *
purple_conversation_get_tags(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return priv->tags;
}

GListModel *
purple_conversation_get_members(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	return G_LIST_MODEL(priv->members);
}

gboolean
purple_conversation_has_member(PurpleConversation *conversation,
                               PurpleContactInfo *info, guint *position)
{
	PurpleConversationPrivate *priv = NULL;
	PurpleConversationMember *needle = NULL;
	gboolean found = FALSE;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), FALSE);

	priv = purple_conversation_get_instance_private(conversation);

	needle = purple_conversation_member_new(info);
	found = g_list_store_find_with_equal_func(priv->members, needle,
	                                          purple_conversation_check_member_equal,
	                                          position);

	g_clear_object(&needle);

	return found;
}

PurpleConversationMember *
purple_conversation_find_member(PurpleConversation *conversation,
                                PurpleContactInfo *info)
{
	PurpleConversationMember *member = NULL;
	guint position = 0;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	if(purple_conversation_has_member(conversation, info, &position)) {
		PurpleConversationPrivate *priv = NULL;

		priv = purple_conversation_get_instance_private(conversation);

		member = g_list_model_get_item(G_LIST_MODEL(priv->members), position);

		/* We don't return a reference, but get_item does, so we need to get
		 * rid of that.
		 */
		g_object_unref(member);
	}

	return member;
}

PurpleConversationMember *
purple_conversation_add_member(PurpleConversation *conversation,
                               PurpleContactInfo *info, gboolean announce,
                               const char *message)
{
	PurpleConversationMember *member = NULL;
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);
	g_return_val_if_fail(PURPLE_IS_CONTACT_INFO(info), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	member = purple_conversation_find_member(conversation, info);
	if(PURPLE_IS_CONVERSATION_MEMBER(member)) {
		return member;
	}

	member = purple_conversation_member_new(info);
	g_list_store_append(priv->members, member);

	g_signal_emit(conversation, signals[SIG_MEMBER_ADDED], 0, member, announce,
	              message);

	g_object_unref(member);

	return member;
}

gboolean
purple_conversation_remove_member(PurpleConversation *conversation,
                                  PurpleConversationMember *member,
                                  gboolean announce, const char *message)
{
	PurpleConversationPrivate *priv = NULL;
	guint position = 0;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), FALSE);
	g_return_val_if_fail(PURPLE_IS_CONVERSATION_MEMBER(member), FALSE);

	priv = purple_conversation_get_instance_private(conversation);

	if(!g_list_store_find(priv->members, member, &position)) {
		return FALSE;
	}

	/* We need to ref member to make sure it stays around long enough for us
	 * to emit the signal.
	 */
	g_object_ref(member);

	g_list_store_remove(priv->members, position);

	g_signal_emit(conversation, signals[SIG_MEMBER_REMOVED], 0, member,
	              announce, message);

	g_object_unref(member);

	return TRUE;
}

GListModel *
purple_conversation_get_messages(PurpleConversation *conversation) {
	PurpleConversationPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_CONVERSATION(conversation), NULL);

	priv = purple_conversation_get_instance_private(conversation);

	if(G_IS_LIST_MODEL(priv->messages)) {
		return G_LIST_MODEL(priv->messages);
	}

	return NULL;
}
