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

/******************************************************************************
 * Main
 *****************************************************************************/
static void
test_purple_presence_new(void) {
	PurplePresence *presence = NULL;

	presence = purple_presence_new();
	g_assert_true(PURPLE_IS_PRESENCE(presence));

	/* Cleanup. */
	g_clear_object(&presence);
}

static void
test_purple_presence_properties(void) {
	PurplePresence *presence = NULL;
	PurplePresencePrimitive primitive = PURPLE_PRESENCE_PRIMITIVE_OFFLINE;
	GDateTime *now = NULL;
	GDateTime *login = NULL;
	GDateTime *login1 = NULL;
	GDateTime *idle = NULL;
	GDateTime *idle1 = NULL;
	char *message = NULL;
	char *emoji = NULL;
	gboolean mobile = FALSE;

	/* Create the login and idle times. */
	now = g_date_time_new_now_utc();
	login = g_date_time_add_hours(now, -1);
	idle = g_date_time_add_minutes(now, -10);
	g_clear_pointer(&now, g_date_time_unref);

	/* Create the presence using g_object_new to make sure set_property is
	 * wired up correctly.
	 */
	presence = g_object_new(
		PURPLE_TYPE_PRESENCE,
		"primitive", PURPLE_PRESENCE_PRIMITIVE_AVAILABLE,
		"message", "I'll be back!",
		"emoji", "🤖",
		"login-time", login,
		"idle-time", idle,
		"mobile", TRUE,
		NULL);

	/* Grab the values via g_object_get to make sure get_property is wired up
	 * correctly.
	 */
	g_object_get(
		presence,
		"primitive", &primitive,
		"message", &message,
		"emoji", &emoji,
		"login-time", &login1,
		"idle-time", &idle1,
		"mobile", &mobile,
		NULL);

	/* Validate! */
	g_assert_cmpint(primitive, ==, PURPLE_PRESENCE_PRIMITIVE_AVAILABLE);
	g_assert_cmpstr(message, ==, "I'll be back!");
	g_assert_cmpstr(emoji, ==, "🤖");
	g_assert_nonnull(login1);
	g_assert_true(g_date_time_equal(login, login1));
	g_assert_nonnull(idle1);
	g_assert_true(g_date_time_equal(idle, idle1));
	g_assert_true(mobile);

	/* Cleanup. */
	g_clear_pointer(&message, g_free);
	g_clear_pointer(&emoji, g_free);
	g_clear_pointer(&login, g_date_time_unref);
	g_clear_pointer(&login1, g_date_time_unref);
	g_clear_pointer(&idle, g_date_time_unref);
	g_clear_pointer(&idle1, g_date_time_unref);
	g_clear_object(&presence);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/presence/new", test_purple_presence_new);
	g_test_add_func("/presence/properties", test_purple_presence_properties);

	return g_test_run();
}
