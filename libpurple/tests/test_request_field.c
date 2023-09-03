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
test_request_field_notify_filled_cb(G_GNUC_UNUSED GObject *obj,
                                    G_GNUC_UNUSED GParamSpec *pspec,
                                    gpointer data)
{
	gboolean *called = data;

	*called = TRUE;
}

static void
test_request_field_filled_string(void) {
	PurpleRequestField *field = NULL;
	gboolean called = FALSE;

	field = purple_request_field_string_new("test-string", "Test string", NULL,
	                                        FALSE);
	g_signal_connect(field, "notify::filled",
	                 G_CALLBACK(test_request_field_notify_filled_cb), &called);
	g_assert_false(purple_request_field_is_filled(field));

	/* Passing same value should not trigger. */
	called = FALSE;
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      NULL);
	g_assert_false(called);
	g_assert_false(purple_request_field_is_filled(field));

	/* Passing an empty string should not trigger, as NULL and "" are
	 * considered the same. */
	called = FALSE;
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      "");
	g_assert_false(called);
	g_assert_false(purple_request_field_is_filled(field));

	/* Now that there's a change from empty to filled, notify should occur. */
	called = FALSE;
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      "text");
	g_assert_true(called);
	g_assert_true(purple_request_field_is_filled(field));

	/* Passing same value should not trigger. */
	called = FALSE;
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      "text");
	g_assert_false(called);
	g_assert_true(purple_request_field_is_filled(field));

	/* And then going back to empty should notify. */
	called = FALSE;
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      "");
	g_assert_true(called);
	g_assert_false(purple_request_field_is_filled(field));

	g_object_unref(field);
}

static void
test_request_field_filled_nonstring(void) {
	/* Anything that's not a string should always be considered filled and
	 * never notify. */
	PurpleRequestField *field = NULL;
	gboolean called = FALSE;

	field = purple_request_field_int_new("test-int", "Test int", 50, 0, 100);
	g_signal_connect(field, "notify::filled",
	                 G_CALLBACK(test_request_field_notify_filled_cb), &called);
	g_assert_true(purple_request_field_is_filled(field));

	called = FALSE;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), 50);
	g_assert_false(called);
	g_assert_true(purple_request_field_is_filled(field));

	called = FALSE;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), 0);
	g_assert_false(called);
	g_assert_true(purple_request_field_is_filled(field));

	called = FALSE;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), 100);
	g_assert_false(called);
	g_assert_true(purple_request_field_is_filled(field));

	g_object_unref(field);
}

static void
test_request_field_valid_int(void) {
	PurpleRequestField *field = NULL;
	char *errmsg = NULL;
	gboolean result;

	field = purple_request_field_int_new("test-int", "Test int", 50, 0, 100);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_null(errmsg);
	g_assert_true(result);

	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), -42);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, "Int value -42 exceeds lower bound 0");
	g_assert_false(result);
	g_free(errmsg);

	/* Don't crash if no error message is requested. */
	result = purple_request_field_is_valid(field, NULL);
	g_assert_false(result);

	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), 1337);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, "Int value 1337 exceeds upper bound 100");
	g_assert_false(result);
	g_free(errmsg);

	/* Don't crash if no error message is requested. */
	result = purple_request_field_is_valid(field, NULL);
	g_assert_false(result);

	g_object_unref(field);
}

static gboolean
test_request_field_validator_is_even(PurpleRequestField *field, char **errmsg,
                                     G_GNUC_UNUSED gpointer data)
{
	gboolean *called = data;
	gint value;

	*called = TRUE;
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_INT(field), FALSE);

	value = purple_request_field_int_get_value(PURPLE_REQUEST_FIELD_INT(field));

	if(value % 2 != 0) {
		*errmsg = g_strdup_printf("Value %d is not even.", value);
		return FALSE;
	}

	return TRUE;
}

static void
test_request_field_valid_custom(void) {
	PurpleRequestField *field = NULL;
	char *errmsg = NULL;
	gboolean result;
	gboolean called = FALSE;

	field = purple_request_field_int_new("test-int", "Test int", 50, 0, 100);
	purple_request_field_set_validator(field,
	                                   test_request_field_validator_is_even,
	                                   &called, NULL);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, NULL);
	g_assert_true(result);

	/* Default validator (i.e., the bounds) is checked first. */
	called = FALSE;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), -42);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, "Int value -42 exceeds lower bound 0");
	g_assert_false(result);
	g_assert_false(called);
	g_free(errmsg);

	/* But if default validator passes, then the custom one is checked. */
	called = FALSE;
	purple_request_field_int_set_value(PURPLE_REQUEST_FIELD_INT(field), 23);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, "Value 23 is not even.");
	g_assert_false(result);
	g_assert_true(called);
	g_free(errmsg);

	g_object_unref(field);
}

static void
test_request_field_required_validity(void) {
	PurpleRequestField *field = NULL;
	char *errmsg = NULL;
	gboolean result;

	field = purple_request_field_string_new("test-string", "Test string", NULL,
	                                        FALSE);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, NULL);
	g_assert_true(result);

	/* Once required, a field must be filled to be valid. */
	purple_request_field_set_required(field, TRUE);
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, "Required field is not filled.");
	g_assert_false(result);
	g_free(errmsg);

	/* But once filled (and there's no other validator), then it can be valid. */
	purple_request_field_string_set_value(PURPLE_REQUEST_FIELD_STRING(field),
	                                      "valid");
	result = purple_request_field_is_valid(field, &errmsg);
	g_assert_cmpstr(errmsg, ==, NULL);
	g_assert_true(result);

	g_object_unref(field);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar *argv[]) {
	g_test_init(&argc, &argv, NULL);

	g_test_add_func("/request-field/filled-string",
	                test_request_field_filled_string);
	g_test_add_func("/request-field/filled-nonstring",
	                test_request_field_filled_nonstring);

	g_test_add_func("/request-field/valid-int", test_request_field_valid_int);
	g_test_add_func("/request-field/valid-custom",
	                test_request_field_valid_custom);

	g_test_add_func("/request-field/required-validity",
	                test_request_field_required_validity);

	return g_test_run();
}
