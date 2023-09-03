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

#define TEST_PURPLE_PROTOCOL_CONTACTS_DOMAIN (g_quark_from_static_string("test-protocol-contacts"))

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
test_purple_protocol_contacts_timeout_cb(gpointer data) {
	g_main_loop_quit(data);

	g_warning("timed out waiting for the callback function to be called");

	return G_SOURCE_REMOVE;
}

/******************************************************************************
 * TestProtocolContactsEmpty implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(TestPurpleProtocolContactsEmpty,
                     test_purple_protocol_contacts_empty,
                     TEST_PURPLE, PROTOCOL_CONTACTS_EMPTY, PurpleProtocol)

struct _TestPurpleProtocolContactsEmpty {
	PurpleProtocol parent;
};

static void
test_purple_protocol_contacts_empty_iface_init(G_GNUC_UNUSED PurpleProtocolContactsInterface *iface)
{
}

G_DEFINE_TYPE_WITH_CODE(TestPurpleProtocolContactsEmpty,
                        test_purple_protocol_contacts_empty,
                        PURPLE_TYPE_PROTOCOL,
                        G_IMPLEMENT_INTERFACE(PURPLE_TYPE_PROTOCOL_CONTACTS,
                                              test_purple_protocol_contacts_empty_iface_init))

static void
test_purple_protocol_contacts_empty_init(G_GNUC_UNUSED TestPurpleProtocolContactsEmpty *empty)
{
}

static void
test_purple_protocol_contacts_empty_class_init(G_GNUC_UNUSED TestPurpleProtocolContactsEmptyClass *klass)
{
}

/******************************************************************************
 * TestProtocolContactsEmpty Tests
 *****************************************************************************/
static void
test_purple_protocol_contacts_empty_get_profile_async(void) {
	if(g_test_subprocess()) {
		PurpleContactInfo *info = NULL;
		PurpleProtocolContacts *protocol_contacts = NULL;

		protocol_contacts = g_object_new(test_purple_protocol_contacts_empty_get_type(),
		                                 NULL);
		info = purple_contact_info_new(NULL);

		purple_protocol_contacts_get_profile_async(protocol_contacts, info,
		                                           NULL, NULL, NULL);

		g_clear_object(&info);
		g_clear_object(&protocol_contacts);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-WARNING*TestPurpleProtocolContactsEmpty*get_profile_async*");
}

static void
test_purple_protocol_contacts_empty_get_profile_finish(void) {
	if(g_test_subprocess()) {
		PurpleProtocolContacts *protocol_contacts = NULL;
		GError *error = NULL;
		GTask *task = NULL;
		char *result = NULL;

		protocol_contacts = g_object_new(test_purple_protocol_contacts_empty_get_type(),
		                                 NULL);

		task = g_task_new(protocol_contacts, NULL, NULL, NULL);

		result = purple_protocol_contacts_get_profile_finish(protocol_contacts,
		                                                     G_ASYNC_RESULT(task),
		                                                     &error);
		g_assert_no_error(error);
		g_assert_false(result);

		g_clear_object(&task);
		g_clear_object(&protocol_contacts);
	}

	g_test_trap_subprocess(NULL, 0, 0);
	g_test_trap_assert_stderr("*Purple-WARNING*TestPurpleProtocolContactsEmpty*get_profile_finish*");
}

static void
test_purple_protocol_contacts_empty_get_actions(void) {
	PurpleProtocolContacts *protocol_contacts = NULL;
	PurpleContactInfo *info = NULL;
	GActionGroup *group = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_empty_get_type(),
	                                 NULL);
	info = purple_contact_info_new(NULL);

	group = purple_protocol_contacts_get_actions(protocol_contacts, info);
	g_assert_null(group);

	g_clear_object(&info);
	g_clear_object(&protocol_contacts);
}

static void
test_purple_protocol_contacts_empty_get_menu(void) {
	PurpleProtocolContacts *protocol_contacts = NULL;
	PurpleContactInfo *info = NULL;
	GMenuModel *model = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_empty_get_type(),
	                                 NULL);
	info = purple_contact_info_new(NULL);

	model = purple_protocol_contacts_get_menu(protocol_contacts, info);
	g_assert_null(model);

	g_clear_object(&info);
	g_clear_object(&protocol_contacts);
}

/******************************************************************************
 * TestProtocolContacts implementation
 *****************************************************************************/
G_DECLARE_FINAL_TYPE(TestPurpleProtocolContacts,
                     test_purple_protocol_contacts, TEST_PURPLE,
                     PROTOCOL_CONTACTS, PurpleProtocol)

struct _TestPurpleProtocolContacts {
	PurpleProtocol parent;

	gboolean should_error;

	guint get_profile_async;
	guint get_profile_finish;
};

static void
test_purple_protocol_contacts_get_profile_async(PurpleProtocolContacts *r,
                                                G_GNUC_UNUSED PurpleContactInfo *info,
                                                GCancellable *cancellable,
                                                GAsyncReadyCallback callback,
                                                gpointer data)
{
	TestPurpleProtocolContacts *protocol_contacts = NULL;
	GTask *task = NULL;

	protocol_contacts = TEST_PURPLE_PROTOCOL_CONTACTS(r);
	protocol_contacts->get_profile_async += 1;

	task = g_task_new(r, cancellable, callback, data);
	if(protocol_contacts->should_error) {
		GError *error = g_error_new_literal(TEST_PURPLE_PROTOCOL_CONTACTS_DOMAIN,
		                                    0, "error");
		g_task_return_error(task, error);
	} else {
		g_task_return_pointer(task, g_strdup("profile data"), g_free);
	}

	g_clear_object(&task);
}

static char *
test_purple_protocol_contacts_get_profile_finish(PurpleProtocolContacts *r,
                                                 GAsyncResult *result,
                                                 GError **error)
{
	TestPurpleProtocolContacts *protocol_contacts = NULL;

	protocol_contacts = TEST_PURPLE_PROTOCOL_CONTACTS(r);
	protocol_contacts->get_profile_finish += 1;

	return g_task_propagate_pointer(G_TASK(result), error);
}

static GActionGroup *
test_purple_protocol_contacts_get_actions(G_GNUC_UNUSED PurpleProtocolContacts *protocol_contacts,
                                          G_GNUC_UNUSED PurpleContactInfo *info)
{
	GSimpleActionGroup *ag = g_simple_action_group_new();

	return G_ACTION_GROUP(ag);
}

static GMenuModel *
test_purple_protocol_contacts_get_menu(G_GNUC_UNUSED PurpleProtocolContacts *protocol_contacts,
                                       G_GNUC_UNUSED PurpleContactInfo *info)
{
	GMenu *menu = g_menu_new();

	return G_MENU_MODEL(menu);
}

static void
test_purple_protocol_contacts_iface_init(PurpleProtocolContactsInterface *iface) {
	iface->get_profile_async = test_purple_protocol_contacts_get_profile_async;
	iface->get_profile_finish = test_purple_protocol_contacts_get_profile_finish;

	iface->get_actions = test_purple_protocol_contacts_get_actions;
	iface->get_menu = test_purple_protocol_contacts_get_menu;
}

G_DEFINE_TYPE_WITH_CODE(TestPurpleProtocolContacts, test_purple_protocol_contacts,
                        PURPLE_TYPE_PROTOCOL,
                        G_IMPLEMENT_INTERFACE(PURPLE_TYPE_PROTOCOL_CONTACTS,
                                              test_purple_protocol_contacts_iface_init))

static void
test_purple_protocol_contacts_init(TestPurpleProtocolContacts *protocol_contacts)
{
	protocol_contacts->get_profile_async = 0;
	protocol_contacts->get_profile_finish = 0;
}

static void
test_purple_protocol_contacts_class_init(G_GNUC_UNUSED TestPurpleProtocolContactsClass *klass)
{
}

/******************************************************************************
 * TestProtocolContacts search test
 *****************************************************************************/
static void
test_purple_protocol_contacts_get_profile_cb(GObject *obj, GAsyncResult *res,
                                             gpointer data)
{
	TestPurpleProtocolContacts *test_protocol_contacts = NULL;
	PurpleContactInfo *info = data;
	PurpleProtocolContacts *protocol_contacts = NULL;
	GError *error = NULL;
	char *result = NULL;

	protocol_contacts = PURPLE_PROTOCOL_CONTACTS(obj);
	test_protocol_contacts = TEST_PURPLE_PROTOCOL_CONTACTS(obj);

	result = purple_protocol_contacts_get_profile_finish(protocol_contacts,
	                                                     res, &error);

	if(test_protocol_contacts->should_error) {
		g_assert_error(error, TEST_PURPLE_PROTOCOL_CONTACTS_DOMAIN, 0);
		g_clear_error(&error);
		g_assert_null(result);
	} else {
		g_assert_no_error(error);
		g_assert_cmpstr(result, ==, "profile data");
		g_clear_pointer(&result, g_free);

	}

	g_clear_object(&info);

	g_main_loop_quit(loop);
}

static gboolean
test_purple_protocol_contacts_get_profile_idle(gpointer data) {
	PurpleProtocolContacts *protocol_contacts = data;
	PurpleContactInfo *info = NULL;

	info = purple_contact_info_new(NULL);

	purple_protocol_contacts_get_profile_async(protocol_contacts, info, NULL,
	                                           test_purple_protocol_contacts_get_profile_cb,
	                                           info);

	return G_SOURCE_REMOVE;
}

static void
test_purple_protocol_contacts_get_profile_normal(void) {
	TestPurpleProtocolContacts *protocol_contacts = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_get_type(),
	                                 NULL);

	g_idle_add(test_purple_protocol_contacts_get_profile_idle,
	           protocol_contacts);
	g_timeout_add_seconds(10, test_purple_protocol_contacts_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_cmpuint(protocol_contacts->get_profile_async, ==, 1);
	g_assert_cmpuint(protocol_contacts->get_profile_finish, ==, 1);

	g_clear_object(&protocol_contacts)
;}

static void
test_purple_protocol_contacts_get_profile_error_normal(void) {
	TestPurpleProtocolContacts *protocol_contacts = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_get_type(),
	                                 NULL);
	protocol_contacts->should_error = TRUE;

	g_idle_add(test_purple_protocol_contacts_get_profile_idle,
	           protocol_contacts);
	g_timeout_add_seconds(10, test_purple_protocol_contacts_timeout_cb, loop);

	g_main_loop_run(loop);

	g_assert_cmpuint(protocol_contacts->get_profile_async, ==, 1);
	g_assert_cmpuint(protocol_contacts->get_profile_finish, ==, 1);

	g_clear_object(&protocol_contacts);
}

static void
test_purple_protocol_contacts_get_actions_normal(void) {
	PurpleProtocolContacts *protocol_contacts = NULL;
	PurpleContactInfo *info = NULL;
	GActionGroup *group = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_get_type(),
	                                 NULL);

	info = purple_contact_info_new(NULL);
	group = purple_protocol_contacts_get_actions(protocol_contacts, info);
	g_assert_true(G_IS_ACTION_GROUP(group));

	g_clear_object(&group);
	g_clear_object(&info);
	g_clear_object(&protocol_contacts);
}

static void
test_purple_protocol_contacts_get_menu_normal(void) {
	PurpleProtocolContacts *protocol_contacts = NULL;
	PurpleContactInfo *info = NULL;
	GMenuModel *menu = NULL;

	protocol_contacts = g_object_new(test_purple_protocol_contacts_get_type(),
	                                 NULL);

	info = purple_contact_info_new(NULL);
	menu = purple_protocol_contacts_get_menu(protocol_contacts, info);
	g_assert_true(G_IS_MENU_MODEL(menu));

	g_clear_object(&menu);
	g_clear_object(&info);
	g_clear_object(&protocol_contacts);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(int argc, char **argv) {
	int ret = 0;

	g_test_init(&argc, &argv, NULL);

	test_ui_purple_init();

	loop = g_main_loop_new(NULL, FALSE);

	g_test_add_func("/protocol-contacts/empty/get-profile-async",
	                test_purple_protocol_contacts_empty_get_profile_async);
	g_test_add_func("/protocol-contacts/empty/get-profile-finish",
	                test_purple_protocol_contacts_empty_get_profile_finish);
	g_test_add_func("/protocol-contacts/empty/get-actions",
	                test_purple_protocol_contacts_empty_get_actions);
	g_test_add_func("/protocol-contacts/empty/get-menu",
	                test_purple_protocol_contacts_empty_get_menu);

	g_test_add_func("/protocol-contacts/normal/get-profile-normal",
	                test_purple_protocol_contacts_get_profile_normal);
	g_test_add_func("/protocol-contacts/normal/get-profile-error",
	                test_purple_protocol_contacts_get_profile_error_normal);
	g_test_add_func("/protocol-contacts/normal/get-actions",
	                test_purple_protocol_contacts_get_actions_normal);
	g_test_add_func("/protocol-contacts/normal/get-menu",
	                test_purple_protocol_contacts_get_menu_normal);

	ret = g_test_run();

	g_main_loop_unref(loop);

	test_ui_purple_uninit();

	return ret;
}
