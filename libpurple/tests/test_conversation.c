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

#include <glib.h>

#include <purple.h>

#include "test_ui.h"

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_conversation_properties(void) {
	PurpleAccount *account = NULL;
	PurpleAccount *account1 = NULL;
	PurpleConnectionFlags features = 0;
	PurpleContactInfo *creator = NULL;
	PurpleContactInfo *creator1 = NULL;
	PurpleContactInfo *topic_author = NULL;
	PurpleContactInfo *topic_author1 = NULL;
	PurpleConversation *conversation = NULL;
	PurpleConversationManager *conversation_manager = NULL;
	PurpleTags *tags = NULL;
	GDateTime *created_on = NULL;
	GDateTime *created_on1 = NULL;
	GDateTime *topic_updated = NULL;
	GDateTime *topic_updated1 = NULL;
	GListModel *members = NULL;
	char *description = NULL;
	char *id = NULL;
	char *name = NULL;
	char *topic = NULL;
	char *user_nickname = NULL;
	gboolean age_restricted = FALSE;
	gboolean favorite = FALSE;
	gboolean unregistered = FALSE;

	account = purple_account_new("test", "test");
	creator = purple_contact_info_new(NULL);
	created_on = g_date_time_new_now_utc();
	topic_author = purple_contact_info_new(NULL);
	topic_updated = g_date_time_new_now_utc();

	/* Use g_object_new so we can test setting properties by name. All of them
	 * call the setter methods, so by doing it this way we exercise more of the
	 * code.
	 *
	 * We don't currently test title because purple_conversation_autoset_title
	 * makes it something we don't expect it to be.
	 */
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"age-restricted", TRUE,
		"created-on", created_on,
		"creator", creator,
		"description", "to describe or not to describe...",
		"favorite", TRUE,
		"features", PURPLE_CONNECTION_FLAG_HTML,
		"id", "id1",
		"name", "name1",
		"topic", "the topic...",
		"topic-author", topic_author,
		"topic-updated", topic_updated,
		"user-nickname", "knick-knack",
		NULL);

	/* Now use g_object_get to read all of the properties. */
	g_object_get(conversation,
		"account", &account1,
		"age-restricted", &age_restricted,
		"created-on", &created_on1,
		"creator", &creator1,
		"description", &description,
		"favorite", &favorite,
		"features", &features,
		"id", &id,
		"members", &members,
		"name", &name,
		"tags", &tags,
		"topic", &topic,
		"topic-author", &topic_author1,
		"topic-updated", &topic_updated1,
		"user-nickname", &user_nickname,
		NULL);

	/* Compare all the things. */
	g_assert_true(account1 == account);
	g_clear_object(&account1);

	g_assert_true(age_restricted);

	g_assert_nonnull(created_on1);
	g_assert_true(g_date_time_equal(created_on1, created_on));
	g_clear_pointer(&created_on1, g_date_time_unref);

	g_assert_true(creator1 == creator);
	g_clear_object(&creator1);

	g_assert_cmpstr(description, ==, "to describe or not to describe...");
	g_clear_pointer(&description, g_free);

	g_assert_true(favorite);

	g_assert_cmpint(features, ==, PURPLE_CONNECTION_FLAG_HTML);

	g_assert_cmpstr(id, ==, "id1");
	g_clear_pointer(&id, g_free);

	g_assert_true(G_IS_LIST_MODEL(members));
	g_clear_object(&members);

	g_assert_cmpstr(name, ==, "name1");
	g_clear_pointer(&name, g_free);

	g_assert_true(PURPLE_IS_TAGS(tags));
	g_clear_object(&tags);

	g_assert_cmpstr(topic, ==, "the topic...");
	g_clear_pointer(&topic, g_free);

	g_assert_true(topic_author == topic_author1);
	g_clear_object(&topic_author1);

	g_assert_nonnull(topic_updated1);
	g_assert_true(g_date_time_equal(topic_updated1, topic_updated));
	g_clear_pointer(&topic_updated1, g_date_time_unref);

	g_assert_cmpstr(user_nickname, ==, "knick-knack");
	g_clear_pointer(&user_nickname, g_free);

	/* TODO: Conversations are automatically registered on construction for
	 * legacy reasons, so we need to explicitly unregister to clean them up,
	 * but this can go away once that stops happening. */
	conversation_manager = purple_conversation_manager_get_default();
	unregistered = purple_conversation_manager_unregister(conversation_manager,
	                                                      conversation);
	g_assert_true(unregistered);

	g_clear_object(&topic_author);
	g_clear_pointer(&topic_updated, g_date_time_unref);
	g_clear_pointer(&created_on, g_date_time_unref);
	g_clear_object(&creator);
	g_clear_object(&account);
	g_clear_object(&conversation);
}

/******************************************************************************
 * Membership tests and helpers
 *****************************************************************************/
static void
test_purple_conversation_membership_signal_cb(PurpleConversation *conversation,
                                              PurpleConversationMember *member,
                                              gboolean announce,
                                              const char *message,
                                              gpointer data)
{
	/* We use int's for called to make sure it was only called once. */
	gint *called = data;

	g_assert_true(PURPLE_IS_CONVERSATION(conversation));
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member));
	g_assert_true(announce);
	g_assert_cmpstr(message, ==, "announcement message");

	*called = *called + 1;
}

static void
test_purple_conversation_members_add_remove(void) {
	PurpleAccount *account = NULL;
	PurpleContactInfo *info = NULL;
	PurpleConversation *conversation = NULL;
	PurpleConversationManager *conversation_manager = NULL;
	PurpleConversationMember *member = NULL;
	PurpleConversationMember *member1 = NULL;
	gboolean removed = FALSE;
	gint added_called = 0;
	gint removed_called = 0;

	/* Create our instances. */
	info = purple_contact_info_new(NULL);
	account = purple_account_new("test", "test");
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"name", "test-conversation",
		NULL);

	/* Connect our signals. */
	g_signal_connect(conversation, "member-added",
	                 G_CALLBACK(test_purple_conversation_membership_signal_cb),
	                 &added_called);
	g_signal_connect(conversation, "member-removed",
	                 G_CALLBACK(test_purple_conversation_membership_signal_cb),
	                 &removed_called);

	/* Add the member. */
	member = purple_conversation_add_member(conversation, info, TRUE,
	                                        "announcement message");
	g_assert_cmpint(added_called, ==, 1);
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member));

	/* Add our own reference to the returned member as we use it later to
	 * verify that double remove doesn't do anything.
	 */
	g_object_ref(member);

	/* Try to add the member again, which would just return the existing
	 * member and not emit the signal.
	 */
	member1 = purple_conversation_add_member(conversation, info, TRUE,
	                                         "announcement message");
	g_assert_cmpint(added_called, ==, 1);
	g_assert_true(PURPLE_IS_CONVERSATION_MEMBER(member1));
	g_assert_true(member1 == member);

	/* Now remove the member and verify the signal was called. */
	removed = purple_conversation_remove_member(conversation, member, TRUE,
	                                            "announcement message");
	g_assert_true(removed);
	g_assert_cmpint(removed_called, ==, 1);

	/* Try to remove the member again and verify that nothing was removed and
	 * that the signal wasn't emitted.
	 */
	removed = purple_conversation_remove_member(conversation, member, TRUE,
	                                            "announcement message");
	g_assert_false(removed);
	g_assert_cmpint(removed_called, ==, 1);

	/* TODO: Conversations are automatically registered on construction for
	 * legacy reasons, so we need to explicitly unregister to clean them up,
	 * but this can go away once that stops happening. */
	conversation_manager = purple_conversation_manager_get_default();
	purple_conversation_manager_unregister(conversation_manager, conversation);

	/* Clean up everything. */
	g_clear_object(&info);
	g_clear_object(&member);
	g_clear_object(&account);
	g_clear_object(&conversation);
}

/******************************************************************************
 * Message Tests
 *****************************************************************************/
static void
test_purple_conversation_message_write_one(void) {
	PurpleAccount *account = NULL;
	PurpleConversation *conversation = NULL;
	PurpleMessage *message = NULL;
	GListModel *messages = NULL;

	account = purple_account_new("test", "test");
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"name", "this is required",
		NULL);

	messages = purple_conversation_get_messages(conversation);
	g_assert_nonnull(messages);
	g_assert_cmpuint(g_list_model_get_n_items(messages), ==, 0);

	message = g_object_new(
		PURPLE_TYPE_MESSAGE,
		"contents", "hello world!",
		NULL);

	purple_conversation_write_message(conversation, message);
	g_assert_cmpuint(g_list_model_get_n_items(messages), ==, 1);

	g_clear_object(&message);
	g_clear_object(&account);
	g_clear_object(&conversation);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/conversation/properties",
	                test_purple_conversation_properties);

	g_test_add_func("/conversation/members/add-remove",
	                test_purple_conversation_members_add_remove);

	g_test_add_func("/conversation/message/write-one",
	                test_purple_conversation_message_write_one);

	ret = g_test_run();

	test_ui_purple_uninit();

	return ret;
}
