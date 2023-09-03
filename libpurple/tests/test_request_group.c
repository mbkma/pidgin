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
 * Tests
 *****************************************************************************/
static void
test_request_group_valid_changed_cb(G_GNUC_UNUSED GObject *obj,
                                    G_GNUC_UNUSED GParamSpec *pspec,
                                    gpointer data)
{
	gint *called = data;

	*called += 1;
}

static void
test_request_group_valid(void) {
	PurpleRequestGroup *group = NULL;
	PurpleRequestField *field1 = NULL, *field2 = NULL, *field3 = NULL;
	gint called = 0;

	group = purple_request_group_new("test-group");
	g_signal_connect(group, "notify::valid",
	                 G_CALLBACK(test_request_group_valid_changed_cb), &called);

	/* Empty groups are always valid. */
	g_assert_true(purple_request_group_is_valid(group));

	/* An added valid field keeps the group valid. */
	called = 0;
	field1 = purple_request_field_int_new("test-int", "Test int", 50, 0, 100);
	purple_request_group_add_field(group, field1);
	g_assert_true(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 0);

	/* Making the field invalid makes the group invalid. */
	called = 0;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field1), -42);
	g_assert_false(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 1);

	/* Adding an invalid field keeps the group invalid. */
	called = 0;
	field2 = purple_request_field_int_new("invalid", "Invalid", -42, 0, 100);
	purple_request_group_add_field(group, field2);
	g_assert_false(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 0);

	/* Adding a valid field to an already invalid group does not change it to
	 * valid accidentally. */
	called = 0;
	field3 = purple_request_field_int_new("valid", "Valid", 42, 0, 100);
	purple_request_group_add_field(group, field3);
	g_assert_false(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 0);

	/* Making one field valid while others are still invalid keeps the group
	 * invalid. */
	called = 0;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field1), 42);
	g_assert_false(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 0);

	/* Making last invalid field valid makes the group valid again. */
	called = 0;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field2), 42);
	g_assert_true(purple_request_group_is_valid(group));
	g_assert_cmpint(called, ==, 1);

	g_object_unref(group);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/request-group/valid", test_request_group_valid);

	return g_test_run();
}
