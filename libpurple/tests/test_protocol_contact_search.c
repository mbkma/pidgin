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

#include "test_ui.h"

/******************************************************************************
 * Globals
 *****************************************************************************/

/* Since we're using GTask to test asynchronous functions, we need to use a
 * main loop.
 */
static GMainLoop *loop = NULL;

#define TEST_PURPLE_PROTOCOL_CONTACT_SEARCH_DOMAIN (g_quark_from_static_string("test-purple-protocol-contact-search"))

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
test_purple_protocol_contact_search_timeout_cb(gpointer data) {
	g_main_loop_quit(data);

	g_warning("timed out waiting for the callback function to be called");

	return G_SOURCE_REMOVE;
}

/******************************************************************************
 * TestProtocolContactSearchEmpty implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(TestPurpleProtocolContactSearchEmpty,
                     test_purple_protocol_contact_search_empty,
                     TEST_PURPLE, PROTOCOL_CONTACT_SEARCH_EMPTY, PurpleProtocol)

struct _TestPurpleProtocolContactSearchEmpty {
	PurpleProtocol parent;
};

static void
test_purple_protocol_contact_search_empty_iface_init(G_GNUC_UNUSED PurpleProtocolContactSearchInterface *iface)
{
}

G_DEFINE_TYPE_WITH_CODE(TestPurpleProtocolContactSearchEmpty,
                        test_purple_protocol_contact_search_empty,
                        PURPLE_TYPE_PROTOCOL,
                        G_IMPLEMENT_INTERFACE(PURPLE_TYPE_PROTOCOL_CONTACT_SEARCH,
                                              test_purple_protocol_contact_search_empty_iface_init))

static void
test_purple_protocol_contact_search_empty_init(G_GNUC_UNUSED TestPurpleProtocolContactSearchEmpty *empty)
{
}

static void
test_purple_protocol_contact_search_empty_class_init(G_GNUC_UNUSED TestPurpleProtocolContactSearchEmptyClass *klass)
{
}

/******************************************************************************
 * TestProtocolContactSearchEmpty Tests
 *****************************************************************************/
static void
test_purple_protocol_contact_search_empty_search_async(void) {
	if(g_test_subprocess()) {
		PurpleAccount *account = NULL;
		PurpleProtocolContactSearch *search = NULL;

		search = g_object_new(test_purple_protocol_contact_search_empty_get_type(),
		                      NULL);

		account = purple_account_new("test", "test");

		purple_protocol_contact_search_search_async(search, account, "alice",
		                                            NULL, NULL, NULL);

		g_clear_object(&account);
		g_clear_object(&search);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-WARNING*TestPurpleProtocolContactSearchEmpty*search_async*");
}

static void
test_purple_protocol_contact_search_empty_search_finish(void) {
	if(g_test_subprocess()) {
		PurpleProtocolContactSearch *search = NULL;
		GTask *task = NULL;

		search = g_object_new(test_purple_protocol_contact_search_empty_get_type(),
		                      NULL);

		task = g_task_new(search, NULL, NULL, NULL);

		purple_protocol_contact_search_search_finish(search,
		                                             G_ASYNC_RESULT(task),
		                                             NULL);

		g_clear_object(&task);
		g_clear_object(&search);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-WARNING*TestPurpleProtocolContactSearchEmpty*search_finish*");
}

/******************************************************************************
 * TestProtocolContactSearch implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(TestPurpleProtocolContactSearch,
                     test_purple_protocol_contact_search, TEST_PURPLE,
                     PROTOCOL_CONTACT_SEARCH, PurpleProtocol)

struct _TestPurpleProtocolContactSearch {
	PurpleProtocol parent;

	gboolean should_error;

	gboolean search_async;
	gboolean search_finish;
};

static void
test_purple_protocol_contact_search_search_async(PurpleProtocolContactSearch *r,
                                                 G_GNUC_UNUSED PurpleAccount *account,
                                                 G_GNUC_UNUSED const char *text,
                                                 GCancellable *cancellable,
                                                 GAsyncReadyCallback callback,
                                                 gpointer data)
{
	TestPurpleProtocolContactSearch *search = TEST_PURPLE_PROTOCOL_CONTACT_SEARCH(r);
	GTask *task = NULL;

	search->search_async = TRUE;

	task = g_task_new(r, cancellable, callback, data);
	if(search->should_error) {
		GError *error = g_error_new_literal(TEST_PURPLE_PROTOCOL_CONTACT_SEARCH_DOMAIN,
		                                    0, "error");
		g_task_return_error(task, error);
	} else {
		g_task_return_pointer(task,
		                      g_list_store_new(PURPLE_TYPE_CONTACT_INFO),
		                      g_object_unref);
	}

	g_clear_object(&task);
}

static GListModel *
test_purple_protocol_contact_search_search_finish(PurpleProtocolContactSearch *r,
                                                  GAsyncResult *result,
                                                  GError **error)
{
	TestPurpleProtocolContactSearch *search = TEST_PURPLE_PROTOCOL_CONTACT_SEARCH(r);

	search->search_finish = TRUE;

	return g_task_propagate_pointer(G_TASK(result), error);
}

static void
test_purple_protocol_contact_search_iface_init(PurpleProtocolContactSearchInterface *iface) {
	iface->search_async = test_purple_protocol_contact_search_search_async;
	iface->search_finish = test_purple_protocol_contact_search_search_finish;
}

G_DEFINE_TYPE_WITH_CODE(TestPurpleProtocolContactSearch, test_purple_protocol_contact_search,
                        PURPLE_TYPE_PROTOCOL,
                        G_IMPLEMENT_INTERFACE(PURPLE_TYPE_PROTOCOL_CONTACT_SEARCH,
                                              test_purple_protocol_contact_search_iface_init))

static void
test_purple_protocol_contact_search_init(G_GNUC_UNUSED TestPurpleProtocolContactSearch *search)
{
}

static void
test_purple_protocol_contact_search_class_init(G_GNUC_UNUSED TestPurpleProtocolContactSearchClass *klass)
{
}

/******************************************************************************
 * TestProtocolContactSearch search test
 *****************************************************************************/
static void
test_purple_protocol_contact_search_search_cb(GObject *obj, GAsyncResult *res,
                                              gpointer data)
{
	TestPurpleProtocolContactSearch *test_search = TEST_PURPLE_PROTOCOL_CONTACT_SEARCH(obj);
	PurpleAccount *account = data;
	PurpleProtocolContactSearch *search = PURPLE_PROTOCOL_CONTACT_SEARCH(obj);
	GError *error = NULL;
	GListModel *result = NULL;

	result = purple_protocol_contact_search_search_finish(search, res, &error);
	if(test_search->should_error) {
		g_assert_error(error, TEST_PURPLE_PROTOCOL_CONTACT_SEARCH_DOMAIN, 0);
		g_clear_error(&error);
		g_assert_null(result);
	} else {
		GType type = G_TYPE_INVALID;
		g_assert_no_error(error);
		g_assert_true(G_IS_LIST_MODEL(result));

		type = g_list_model_get_item_type(result);
		g_assert_true(g_type_is_a(type, PURPLE_TYPE_CONTACT_INFO));
		g_clear_object(&result);
	}

	g_clear_object(&account);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_protocol_contact_search_search_idle(gpointer data) {
	PurpleProtocolContactSearch *search = data;
	PurpleAccount *account = NULL;

	account = purple_account_new("test", "test");

	purple_protocol_contact_search_search_async(search, account, "bob", NULL,
	                                            test_purple_protocol_contact_search_search_cb,
	                                            account);

	return G_SOURCE_REMOVE;
}

static void
test_purple_protocol_contact_search_search(void) {
	TestPurpleProtocolContactSearch *search = NULL;

	search = g_object_new(test_purple_protocol_contact_search_get_type(), NULL);

	g_idle_add(test_purple_protocol_contact_search_search_idle, search);
	g_timeout_add_seconds(10, test_purple_protocol_contact_search_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_true(search->search_async);
	g_assert_true(search->search_finish);

	g_clear_object(&search)
;}

static void
test_purple_protocol_contact_search_search_error(void) {
	TestPurpleProtocolContactSearch *search = NULL;

	search = g_object_new(test_purple_protocol_contact_search_get_type(), NULL);
	search->should_error = TRUE;

	g_idle_add(test_purple_protocol_contact_search_search_idle, search);
	g_timeout_add_seconds(10, test_purple_protocol_contact_search_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_true(search->search_async);
	g_assert_true(search->search_finish);

	g_clear_object(&search)
;}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	gint ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_test_add_func("/protocol-contact-search/empty/search-async",
	                test_purple_protocol_contact_search_empty_search_async);
	g_test_add_func("/protocol-contact-search/empty/search-finish",
	                test_purple_protocol_contact_search_empty_search_finish);

	g_test_add_func("/protocol-contact-search/normal/search",
	                test_purple_protocol_contact_search_search);
	g_test_add_func("/protocol-contact-search/normal/search-error",
	                test_purple_protocol_contact_search_search_error);

	ret = g_test_run();

	g_main_loop_unref(loop);

	test_ui_purple_uninit();

	return ret;
}
