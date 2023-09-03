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
 * Helpers
 *****************************************************************************/
static gboolean
test_request_page_string_validator(PurpleRequestField *field, char **errmsg,
                                   G_GNUC_UNUSED gpointer data)
{
	/* Validator for a string field that is valid if the value is "valid". */
	PurpleRequestFieldString *strfield = PURPLE_REQUEST_FIELD_STRING(field);
	const char *value = NULL;
	gboolean result = TRUE;

	value = purple_request_field_string_get_value(strfield);
	if(!purple_strequal(value, "valid")) {
		if(errmsg != NULL) {
			*errmsg = g_strdup_printf("String value is not valid: %s", value);
		}
		result = FALSE;
	}

	return result;
}

static PurpleRequestGroup *
test_request_page_new_valid_group(const char *name) {
	PurpleRequestGroup *group = NULL;
	PurpleRequestField *field = NULL;
	char *field_name = NULL;

	group = purple_request_group_new(name);

	/* Field is valid, making the group valid. */
	field_name = g_strdup_printf("%s-string", name);
	field = purple_request_field_string_new(field_name, field_name, "valid",
	                                        FALSE);
	purple_request_field_set_validator(field,
	                                   test_request_page_string_validator,
	                                   NULL, NULL);
	g_free(field_name);
	purple_request_group_add_field(group, field);

	return group;
}

static PurpleRequestGroup *
test_request_page_new_invalid_group(const char *name) {
	PurpleRequestGroup *group = NULL;
	PurpleRequestField *field = NULL;
	char *field_name = NULL;

	group = purple_request_group_new(name);

	/* Field is invalid, making the group invalid. */
	field_name = g_strdup_printf("%s-string", name);
	field = purple_request_field_string_new(field_name, field_name, "invalid",
	                                        FALSE);
	purple_request_field_set_validator(field,
	                                   test_request_page_string_validator,
	                                   NULL, NULL);
	g_free(field_name);
	purple_request_group_add_field(group, field);

	return group;
}

static void
test_request_page_make_group_valid(PurpleRequestGroup *group) {
	GListModel *model = G_LIST_MODEL(group);
	guint n_items;

	n_items = g_list_model_get_n_items(model);
	for(guint index = 0; index < n_items; index++) {
		PurpleRequestFieldString *field = g_list_model_get_item(model, index);
		purple_request_field_string_set_value(field, "valid");
		g_object_unref(field);
	}
}

static void
test_request_page_make_group_invalid(PurpleRequestGroup *group) {
	GListModel *model = G_LIST_MODEL(group);
	guint n_items;

	n_items = g_list_model_get_n_items(model);
	for(guint index = 0; index < n_items; index++) {
		PurpleRequestFieldString *field = g_list_model_get_item(model, index);
		purple_request_field_string_set_value(field, "invalid");
		g_object_unref(field);
	}
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_request_page_valid_changed_cb(G_GNUC_UNUSED GObject *obj,
                                   G_GNUC_UNUSED GParamSpec *pspec,
                                   gpointer data)
{
	gint *called = data;

	*called += 1;
}

static void
test_request_page_valid(void) {
	PurpleRequestPage *page = NULL;
	PurpleRequestGroup *group1 = NULL, *group2 = NULL, *group3 = NULL;
	gint called = FALSE;

	page = purple_request_page_new();
	g_signal_connect(page, "notify::valid",
	                 G_CALLBACK(test_request_page_valid_changed_cb), &called);

	/* Empty pages are always valid. */
	g_assert_true(purple_request_page_is_valid(page));

	/* An added valid group keeps the page valid. */
	called = 0;
	group1 = test_request_page_new_valid_group("group1");
	purple_request_page_add_group(page, group1);
	g_assert_true(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 0);

	/* Making the group invalid makes the page invalid. */
	called = 0;
	test_request_page_make_group_invalid(group1);
	g_assert_false(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 1);

	/* Adding an invalid group keeps the page invalid. */
	called = 0;
	group2 = test_request_page_new_invalid_group("group2");
	purple_request_page_add_group(page, group2);
	g_assert_false(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 0);

	/* Adding a valid group to an already invalid page does not change it to
	 * valid accidentally. */
	called = 0;
	group3 = test_request_page_new_valid_group("group3");
	purple_request_page_add_group(page, group3);
	g_assert_false(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 0);

	/* Making one group valid while others are still invalid keeps the group
	 * invalid. */
	called = 0;
	test_request_page_make_group_valid(group1);
	g_assert_false(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 0);

	/* Making last invalid group valid makes the page valid again. */
	called = 0;
	test_request_page_make_group_valid(group2);
	g_assert_true(purple_request_page_is_valid(page));
	g_assert_cmpint(called, ==, 1);

	g_object_unref(page);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/request-page/valid", test_request_page_valid);

	return g_test_run();
}
