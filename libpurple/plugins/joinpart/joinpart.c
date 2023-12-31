/**
 * purple
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <gplugin.h>
#include <gplugin-native.h>

#include <purple.h>

#define JOINPART_PLUGIN_ID "core-rlaager-joinpart"


/* Preferences */
#define SETTINGS_SCHEMA_ID "im.pidgin.Purple.plugin.JoinPart"

/* The number of minutes before a person is considered to have stopped being
 * part of active conversation. */
#define DELAY_PREF "delay"

/* The number of people that must be in a room for this plugin to have any
 * effect */
#define THRESHOLD_PREF "threshold"

/* Hide buddies */
#define HIDE_BUDDIES_PREF "hide-buddies"

struct joinpart_key
{
	PurpleConversation *conv;
	char *user;
};

static guint joinpart_key_hash(const struct joinpart_key *key)
{
	g_return_val_if_fail(key != NULL, 0);

	return g_direct_hash(key->conv) + g_str_hash(key->user);
}

static gboolean joinpart_key_equal(const struct joinpart_key *a, const struct joinpart_key *b)
{
	if (a == NULL) {
		return (b == NULL);
	} else if (b == NULL) {
		return FALSE;
	}

	return (a->conv == b->conv) && purple_strequal(a->user, b->user);
}

static void joinpart_key_destroy(struct joinpart_key *key)
{
	g_return_if_fail(key != NULL);

	g_free(key->user);
	g_free(key);
}

static gboolean should_hide_notice(PurpleConversation *conv, const char *name,
                                   GHashTable *users)
{
	GSettings *settings = NULL;
	PurpleChatConversation *chat;
	guint threshold;
	struct joinpart_key key;
	time_t *last_said;

	g_return_val_if_fail(conv != NULL, FALSE);
	g_return_val_if_fail(PURPLE_IS_CHAT_CONVERSATION(conv), FALSE);

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());

	/* If the room is small, don't bother. */
	chat = PURPLE_CHAT_CONVERSATION(conv);
	threshold = g_settings_get_int(settings, THRESHOLD_PREF);
	if (purple_chat_conversation_get_users_count(chat) < threshold) {
		g_object_unref(settings);
		return FALSE;
	}

	if (!g_settings_get_boolean(settings, HIDE_BUDDIES_PREF) &&
	    purple_blist_find_buddy(purple_conversation_get_account(conv), name)) {
		g_object_unref(settings);
		return FALSE;
	}

	/* Only show the notice if the user has spoken recently. */
	key.conv = conv;
	key.user = (gchar *)name;
	last_said = g_hash_table_lookup(users, &key);
	if (last_said != NULL) {
		int delay = g_settings_get_int(settings, DELAY_PREF);
		if (delay > 0 && (*last_said + (delay * 60)) >= time(NULL)) {
			g_object_unref(settings);
			return FALSE;
		}
	}

	g_object_unref(settings);

	return TRUE;
}

static gboolean
chat_user_leaving_cb(PurpleConversation *conv, const char *name,
                     G_GNUC_UNUSED const char *reason, GHashTable *users)
{
	return should_hide_notice(conv, name, users);
}

static gboolean
chat_user_joining_cb(PurpleConversation *conv, const char *name,
                     G_GNUC_UNUSED PurpleChatUserFlags flags,
                     GHashTable *users)
{
	return should_hide_notice(conv, name, users);
}

static void
received_chat_msg_cb(G_GNUC_UNUSED PurpleAccount *account, char *sender,
                     G_GNUC_UNUSED char *message, PurpleConversation *conv,
                     G_GNUC_UNUSED PurpleMessageFlags flags, GHashTable *users)
{
	struct joinpart_key key;
	time_t *last_said;

	/* Most of the time, we'll already have tracked the user,
	 * so we avoid memory allocation here. */
	key.conv = conv;
	key.user = sender;
	last_said = g_hash_table_lookup(users, &key);
	if (last_said != NULL) {
		/* They just said something, so update the time. */
		time(last_said);
	} else {
		struct joinpart_key *key2;

		key2 = g_new(struct joinpart_key, 1);
		key2->conv = conv;
		key2->user = g_strdup(sender);

		last_said = g_new(time_t, 1);
		time(last_said);

		g_hash_table_insert(users, key2, last_said);
	}
}

static gboolean check_expire_time(struct joinpart_key *key,
                                  time_t *last_said, time_t *limit)
{
	purple_debug_info("joinpart", "Removing key for %s\n", key->user);
	return (*last_said < *limit);
}

static gboolean clean_users_hash(GHashTable *users)
{
	GSettings *settings = NULL;
	gint delay = 0;
	time_t limit = 0;

	settings = g_settings_new_with_backend(SETTINGS_SCHEMA_ID,
	                                       purple_core_get_settings_backend());
	delay = g_settings_get_int(settings, DELAY_PREF);
	g_object_unref(settings);

	limit = time(NULL) - (60 * delay);

	g_hash_table_foreach_remove(users, (GHRFunc)check_expire_time, &limit);

	return TRUE;
}

static GPluginPluginInfo *
join_part_query(G_GNUC_UNUSED GError **error)
{
	const gchar * const authors[] = {
		"Richard Laager <rlaager@pidgin.im>",
		NULL
	};

	return purple_plugin_info_new(
		"id",             JOINPART_PLUGIN_ID,
		"name",           N_("Join/Part Hiding"),
		"version",        DISPLAY_VERSION,
		"category",       N_("User interface"),
		"summary",        N_("Hides extraneous join/part messages."),
		"description",    N_("This plugin hides join/part messages in "
		                     "large rooms, except for those users actively "
		                     "taking part in a conversation."),
		"authors",        authors,
		"website",        PURPLE_WEBSITE,
		"abi-version",    PURPLE_ABI_VERSION,
		"settings-schema", SETTINGS_SCHEMA_ID,
		NULL
	);
}

static gboolean
join_part_load(GPluginPlugin *plugin, G_GNUC_UNUSED GError **error)
{
	void *conv_handle;
	GHashTable *users;
	guint id;

	users = g_hash_table_new_full((GHashFunc)joinpart_key_hash,
	                              (GEqualFunc)joinpart_key_equal,
	                              (GDestroyNotify)joinpart_key_destroy,
	                              g_free);

	conv_handle = purple_conversations_get_handle();
	purple_signal_connect(conv_handle, "chat-user-joining", plugin,
	                    G_CALLBACK(chat_user_joining_cb), users);
	purple_signal_connect(conv_handle, "chat-user-leaving", plugin,
	                    G_CALLBACK(chat_user_leaving_cb), users);
	purple_signal_connect(conv_handle, "received-chat-msg", plugin,
	                    G_CALLBACK(received_chat_msg_cb), users);

	/* Cleanup every 5 minutes */
	id = g_timeout_add_seconds(60 * 5, G_SOURCE_FUNC(clean_users_hash), users);

	g_object_set_data(G_OBJECT(plugin), "users", users);
	g_object_set_data(G_OBJECT(plugin), "id", GUINT_TO_POINTER(id));

	return TRUE;
}

static gboolean
join_part_unload(GPluginPlugin *plugin,
                 G_GNUC_UNUSED gboolean shutdown,
                 G_GNUC_UNUSED GError **error)
{
	/* Destroy the hash table. The core plugin code will
	 * disconnect the signals, and since Purple is single-threaded,
	 * we don't have to worry one will be called after this. */
	g_hash_table_destroy((GHashTable *)g_object_get_data(G_OBJECT(plugin), "users"));

	g_source_remove(GPOINTER_TO_UINT(g_object_get_data(G_OBJECT(plugin), "id")));

	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(join_part)
