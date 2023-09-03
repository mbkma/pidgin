/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <glib.h>

#include <purple.h>

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
test_purple_tags_counter_cb(G_GNUC_UNUSED PurpleTags *tags,
                            G_GNUC_UNUSED const char *tag,
                            G_GNUC_UNUSED const char *name,
                            G_GNUC_UNUSED const char *value,
                            gpointer data)
{
	guint *counter = data;

	/* Increment the counter so we know we were called. */
	*counter = *counter + 1;
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_tags_lookup_exists(void) {
	PurpleTags *tags = NULL;
	gboolean found = FALSE;
	const gchar *value = NULL;
	guint counter = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &counter);

	purple_tags_add(tags, "foo");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);
	g_assert_cmpuint(counter, ==, 1);
	value = purple_tags_lookup(tags, "foo", &found);
	g_assert_null(value);
	g_assert_true(found);

	purple_tags_add(tags, "bar:baz");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 2);
	g_assert_cmpuint(counter, ==, 2);
	value = purple_tags_lookup(tags, "bar", &found);
	g_assert_cmpstr(value, ==, "baz");
	g_assert_true(found);

	/* make sure that a name of pur doesn't match a tag of purple */
	purple_tags_add(tags, "purple");
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 3);
	g_assert_cmpuint(counter, ==, 3);
	value = purple_tags_lookup(tags, "pur", &found);
	g_assert_null(value);
	g_assert_false(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_lookup_non_existent(void) {
	PurpleTags *tags = purple_tags_new();
	gboolean found = FALSE;
	const gchar *value;

	value = purple_tags_lookup(tags, "foo", &found);
	g_assert_null(value);
	g_assert_false(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove_bare(void) {
	PurpleTags *tags = NULL;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_remove(tags, "tag1");
	g_assert_cmpuint(removed, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_duplicate_bare(void) {
	PurpleTags *tags = NULL;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(removed, ==, 0);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_add(tags, "tag1");
	g_assert_cmpuint(added, ==, 2);
	g_assert_cmpuint(removed, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	g_clear_object(&tags);
}

static void
test_purple_tags_remove_non_existent_bare(void) {
	PurpleTags *tags = NULL;
	gboolean ret = FALSE;
	guint counter = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &counter);

	ret = purple_tags_remove(tags, "tag1");
	g_assert_cmpuint(counter, ==, 0);
	g_assert_false(ret);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove(void) {
	PurpleTags *tags = NULL;
	gboolean ret = FALSE;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	ret = purple_tags_remove(tags, "tag1:purple");
	g_assert_cmpuint(removed, ==, 1);
	g_assert_true(ret);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove_with_null_value(void) {
	PurpleTags *tags = NULL;
	gboolean ret = FALSE;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add_with_value(tags, "tag1", NULL);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	ret = purple_tags_remove_with_value(tags, "tag1", NULL);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);
	g_assert_cmpuint(removed, ==, 1);
	g_assert_true(ret);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_remove_with_value(void) {
	PurpleTags *tags = NULL;
	gboolean ret = FALSE;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add_with_value(tags, "tag1", "purple");
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	ret = purple_tags_remove_with_value(tags, "tag1", "purple");
	g_assert_cmpuint(removed, ==, 1);
	g_assert_true(ret);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_duplicate_with_value(void) {
	PurpleTags *tags = NULL;
	guint added = 0;
	guint removed = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &added);
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &removed);

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(added, ==, 1);
	g_assert_cmpuint(removed, ==, 0);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	purple_tags_add(tags, "tag1:purple");
	g_assert_cmpuint(added, ==, 2);
	g_assert_cmpuint(removed, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_with_value(void) {
	PurpleTags *tags = NULL;
	const char *value = NULL;
	gboolean found = FALSE;
	guint counter = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &counter);

	purple_tags_add_with_value(tags, "tag1", "purple");
	g_assert_cmpuint(counter, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	value = purple_tags_lookup(tags, "tag1", &found);
	g_assert_cmpstr(value, ==, "purple");
	g_assert_true(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_add_with_value_null(void) {
	PurpleTags *tags = NULL;
	const char *value = NULL;
	gboolean found = FALSE;
	guint counter = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "added", G_CALLBACK(test_purple_tags_counter_cb),
	                 &counter);

	purple_tags_add_with_value(tags, "tag1", NULL);
	g_assert_cmpuint(counter, ==, 1);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 1);

	value = purple_tags_lookup(tags, "tag1", &found);
	g_assert_null(value);
	g_assert_true(found);

	g_clear_object(&tags);
}

static void
test_purple_tags_remove_non_existent_with_value(void) {
	PurpleTags *tags = NULL;
	guint counter = 0;

	tags = purple_tags_new();
	g_signal_connect(tags, "removed", G_CALLBACK(test_purple_tags_counter_cb),
	                 &counter);

	purple_tags_remove(tags, "tag1:purple");
	g_assert_cmpuint(counter, ==, 0);
	g_assert_cmpuint(purple_tags_get_count(tags), ==, 0);

	g_clear_object(&tags);
}

static void
test_purple_tags_get_single(void) {
	PurpleTags *tags = purple_tags_new();
	const gchar *value = NULL;

	purple_tags_add(tags, "tag1:purple");
	value = purple_tags_get(tags, "tag1");

	g_assert_cmpstr(value, ==, "purple");

	g_clear_object(&tags);
}

static void
test_purple_tags_get_multiple(void) {
	PurpleTags *tags = purple_tags_new();
	const gchar *value = NULL;

	purple_tags_add(tags, "tag1:purple");
	purple_tags_add(tags, "tag1:pink");

	value = purple_tags_get(tags, "tag1");

	g_assert_cmpstr(value, ==, "purple");

	g_clear_object(&tags);
}

static void
test_purple_tags_get_all(void) {
	PurpleTags *tags = purple_tags_new();
	GList *all_tags = NULL;
	const gchar *values[] = {"foo", "bar", "baz", "qux", "quux", NULL};
	gint i = 0;
	gint len = 0;

	for(i = 0; values[i] != NULL; i++) {
		purple_tags_add(tags, values[i]);
		len++;
	}

	all_tags = purple_tags_get_all(tags);
	i = 0;

	for(GList *l = all_tags; l != NULL; l = l->next) {
		const gchar *value = l->data;

		g_assert_cmpint(i, <, len);
		g_assert_cmpstr(value, ==, values[i]);

		i++;
	}

	g_assert_cmpint(i, ==, len);

	g_clear_object(&tags);
}

static void
test_purple_tags_get_all_with_name(void) {
	PurpleTags *tags = purple_tags_new();
	GList *named = NULL;
	GList *expected = NULL;

	/* Make sure we get null back on no matches. */
	named = purple_tags_get_all_with_name(tags, "group");
	g_assert_null(named);

	/* Add some tags and make sure we get the right ones back. */
	purple_tags_add(tags, "group");
	purple_tags_add(tags, "groups");
	purple_tags_add(tags, "group:");
	purple_tags_add(tags, "grouping");
	purple_tags_add(tags, "group:a");
	purple_tags_add(tags, "grouped");

	/* Setup our expected list in the order that the items should be in. */
	expected = g_list_prepend(expected, "group");
	expected = g_list_prepend(expected, "group:");
	expected = g_list_prepend(expected, "group:a");
	expected = g_list_reverse(expected);

	/* Do the look up and start asserting! */
	named = purple_tags_get_all_with_name(tags, "group");
	g_assert_nonnull(named);
	g_assert_cmpuint(g_list_length(named), ==, g_list_length(expected));

	for(guint i = 0; i < g_list_length(expected); i++) {
		const char *a = g_list_nth_data(expected, i);
		const char *b = g_list_nth_data(named, i);

		g_assert_cmpstr(a, ==, b);
	}

	g_clear_list(&named, NULL);
	g_clear_list(&expected, NULL);
	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_single(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	value = purple_tags_to_string(tags, NULL);

	g_assert_cmpstr(value, ==, "foo");

	g_free(value);

	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_multiple_with_separator(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	purple_tags_add(tags, "bar");
	purple_tags_add(tags, "baz");
	value = purple_tags_to_string(tags, ", ");

	g_assert_cmpstr(value, ==, "foo, bar, baz");

	g_free(value);

	g_clear_object(&tags);
}

static void
test_purple_tags_to_string_multiple_with_null_separator(void) {
	PurpleTags *tags = purple_tags_new();
	gchar *value = NULL;

	purple_tags_add(tags, "foo");
	purple_tags_add(tags, "bar");
	purple_tags_add(tags, "baz");
	value = purple_tags_to_string(tags, NULL);

	g_assert_cmpstr(value, ==, "foobarbaz");

	g_free(value);

	g_clear_object(&tags);
}

/******************************************************************************
 * purple_tag_parse Tests
 *****************************************************************************/
typedef struct {
	char *tag;
	char *name;
	char *value;
} TestPurpleTagParseData;

static void
test_purple_tag_parse(void) {
	TestPurpleTagParseData data[] = {
		{"", "", NULL},
		{"foo", "foo", NULL},
		{"🐦", "🐦", NULL},
		{":", "", ""},
		{"foo:bar", "foo", "bar"},
		{"🐦:", "🐦", ""},
		{":🐦", "", "🐦"},
		{NULL},
	};

	for(int i = 0; data[i].tag != NULL; i++) {
		char *name = NULL;
		char *value = NULL;

		purple_tag_parse(data[i].tag, &name, &value);

		g_assert_cmpstr(name, ==, data[i].name);
		g_assert_cmpstr(value, ==, data[i].value);

		g_free(name);
		g_free(value);
	}

	/* Finally make sure that we don't crash if neither optional argument was
	 * passed.
	 */
	purple_tag_parse("", NULL, NULL);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/tags/lookup-exists", test_purple_tags_lookup_exists);
	g_test_add_func("/tags/lookup-non-existent",
	                test_purple_tags_lookup_non_existent);

	g_test_add_func("/tags/add-remove-bare",
	                test_purple_tags_add_remove_bare);
	g_test_add_func("/tags/add-duplicate-bare",
	                test_purple_tags_add_duplicate_bare);
	g_test_add_func("/tags/remove-non-existent-bare",
	                test_purple_tags_remove_non_existent_bare);

	g_test_add_func("/tags/add-with-value",
	                test_purple_tags_add_with_value);
	g_test_add_func("/tags/add-with-value-null",
	                test_purple_tags_add_with_value_null);

	g_test_add_func("/tags/add-remove",
	                test_purple_tags_add_remove);
	g_test_add_func("/tags/add-remove-with-null-value",
	                test_purple_tags_add_remove_with_null_value);
	g_test_add_func("/tags/add-remove-with-value",
	                test_purple_tags_add_remove_with_value);
	g_test_add_func("/tags/add-duplicate-with-value",
	                test_purple_tags_add_duplicate_with_value);
	g_test_add_func("/tags/remove-non-existent-with-value",
	                test_purple_tags_remove_non_existent_with_value);

	g_test_add_func("/tags/get-single", test_purple_tags_get_single);
	g_test_add_func("/tags/get-multiple", test_purple_tags_get_multiple);

	g_test_add_func("/tags/get-all", test_purple_tags_get_all);
	g_test_add_func("/tags/get-all-with-name",
	                test_purple_tags_get_all_with_name);

	g_test_add_func("/tags/to-string-single",
	                test_purple_tags_to_string_single);
	g_test_add_func("/tags/to-string-multiple-with-separator",
	                test_purple_tags_to_string_multiple_with_separator);
	g_test_add_func("/tags/to-string-multiple-with-null-separator",
	                test_purple_tags_to_string_multiple_with_null_separator);

	g_test_add_func("/tag/parse", test_purple_tag_parse);

	return g_test_run();
}
