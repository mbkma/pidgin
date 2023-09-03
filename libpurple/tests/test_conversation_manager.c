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
 * Callbacks
 *****************************************************************************/
static void
test_purple_conversation_manager_counter_cb(G_GNUC_UNUSED PurpleConversationManager *manager,
                                            G_GNUC_UNUSED PurpleConversation *conversation,
                                            gpointer data)
{
	guint *counter = data;

	*counter = *counter + 1;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_conversation_manager_register_unregister(void) {
	PurpleAccount *account = NULL;
	PurpleConversationManager *manager = NULL;
	PurpleConversation *conversation = NULL;
	GList *conversations = NULL;
	guint registered_counter = 0, unregistered_counter = 0;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_CONVERSATION_MANAGER, NULL);

	g_assert_true(PURPLE_IS_CONVERSATION_MANAGER(manager));

	/* Wire up our signals. */
	g_signal_connect(manager, "registered",
	                 G_CALLBACK(test_purple_conversation_manager_counter_cb),
	                 &registered_counter);
	g_signal_connect(manager, "unregistered",
	                 G_CALLBACK(test_purple_conversation_manager_counter_cb),
	                 &unregistered_counter);

	/* Create our account. */
	account = purple_account_new("test", "test");

	/* Create the conversation. */
	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"name", "purple_conversation_autoset_title sucks",
		NULL);

	/* Add the conversation to the manager. */
	result = purple_conversation_manager_register(manager, conversation);
	g_assert_true(result);

	/* Make sure the conversation is available. */
	conversations = purple_conversation_manager_get_all(manager);
	g_assert_nonnull(conversations);
	g_assert_cmpuint(g_list_length(conversations), ==, 1);
	g_list_free(conversations);

	/* Make sure the registered signal was called. */
	g_assert_cmpuint(registered_counter, ==, 1);

	/* Remove the contact. */
	result = purple_conversation_manager_unregister(manager, conversation);
	g_assert_true(result);
	g_assert_cmpuint(unregistered_counter, ==, 1);

	/* Clean up.*/
	g_clear_object(&conversation);
	g_clear_object(&account);
	g_clear_object(&manager);
}

/******************************************************************************
 * Signal Tests
 *****************************************************************************/
static void
test_purple_conversation_manager_signal_conversation_changed_cb(PurpleConversationManager *manager,
                                                                PurpleConversation *conversation,
                                                                GParamSpec *pspec,
                                                                gpointer data)
{
	guint *counter = data;

	g_assert_true(PURPLE_IS_CONVERSATION_MANAGER(manager));
	g_assert_true(PURPLE_IS_CONVERSATION(conversation));
	g_assert_true(G_IS_PARAM_SPEC(pspec));

	*counter = *counter + 1;
}

static void
test_purple_conversation_manager_signal_conversation_changed(void) {
	PurpleAccount *account = NULL;
	PurpleConversationManager *manager = NULL;
	PurpleConversation *conversation = NULL;
	guint counter = 0;
	guint detailed_counter = 0;
	gboolean result = FALSE;

	manager = g_object_new(PURPLE_TYPE_CONVERSATION_MANAGER, NULL);
	g_signal_connect(manager, "conversation-changed",
	                 G_CALLBACK(test_purple_conversation_manager_signal_conversation_changed_cb),
	                 &counter);
	g_signal_connect(manager, "conversation-changed::title",
	                 G_CALLBACK(test_purple_conversation_manager_signal_conversation_changed_cb),
	                 &detailed_counter);

	account = purple_account_new("test", "test");

	conversation = g_object_new(
		PURPLE_TYPE_CONVERSATION,
		"account", account,
		"name", "kool kats",
		NULL);
	purple_conversation_manager_register(manager, conversation);

	/* Set the features and title of the conversation, and verify our counters. */
	purple_conversation_set_features(conversation, PURPLE_CONNECTION_FLAG_NO_IMAGES);
	purple_conversation_set_title(conversation, "A place for kool kats");

	g_assert_cmpuint(counter, ==, 2);
	g_assert_cmpuint(detailed_counter, ==, 1);

	/* Unregister the conversation, to make sure signals are no longer
	 * propagated.
	 */
	result = purple_conversation_manager_unregister(manager, conversation);
	g_assert_true(result);

	counter = 0;
	detailed_counter = 0;
	purple_conversation_set_title(conversation,
	                              "no longer a place for kool kats");
	g_assert_cmpuint(counter, ==, 0);
	g_assert_cmpuint(detailed_counter, ==, 0);

	/* Clean up. */
	g_clear_object(&conversation);
	g_clear_object(&account);
	g_clear_object(&manager);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	g_test_add_func("/conversation-manager/register-unregister",
	                test_purple_conversation_manager_register_unregister);

	g_test_add_func("/conversation-manager/signals/conversation-changed",
	                test_purple_conversation_manager_signal_conversation_changed);

	ret = g_test_run();

	test_ui_purple_uninit();

	return ret;
}
