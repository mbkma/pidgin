/*
 * Purple
 *
 * Purple is the legal property of its developers, whose names are too
 * numerous to list here. Please refer to the COPYRIGHT file distributed
 * with this source distribution
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02111-1301 USA
 */

#include <glib.h>
#include <string.h>

#include <purple.h>

#include "test_ui.h"

/******************************************************************************
 * PurpleProtcolXfer Implementations
 *****************************************************************************/
static GType test_purple_protocol_xfer_get_type(void);

typedef struct {
	PurpleProtocol parent;

	gboolean can_send;
	gboolean new_xfer_called;
	gboolean send_called;
} TestPurpleProtocolXfer;

typedef struct {
	PurpleProtocolClass parent;
} TestPurpleProtocolXferClass;


static gboolean
test_purple_protocol_xfer_can_receive(PurpleProtocolXfer *prplxfer,
                                      G_GNUC_UNUSED PurpleConnection *c,
                                      G_GNUC_UNUSED const gchar *who)
{
	TestPurpleProtocolXfer *test_xfer = (TestPurpleProtocolXfer *)prplxfer;

	return test_xfer->can_send;
}

static void
test_purple_protocol_xfer_send_file(PurpleProtocolXfer *prplxfer,
                                    G_GNUC_UNUSED PurpleConnection *c,
                                    G_GNUC_UNUSED const gchar *who,
                                    G_GNUC_UNUSED const gchar *filename)
{
	TestPurpleProtocolXfer *test_xfer = (TestPurpleProtocolXfer *)prplxfer;

	test_xfer->send_called = TRUE;
}

static PurpleXfer *
test_purple_protocol_xfer_new_xfer(PurpleProtocolXfer *prplxfer,
                                   PurpleConnection *c,
                                   const gchar *who)
{
	TestPurpleProtocolXfer *test_xfer = (TestPurpleProtocolXfer *)prplxfer;
	PurpleAccount *a = purple_connection_get_account(c);

	test_xfer->new_xfer_called = TRUE;

	return purple_xfer_new(a, PURPLE_XFER_TYPE_SEND, who);
}

static void
test_purple_protocol_xfer_iface_init(PurpleProtocolXferInterface *iface) {
	iface->can_receive = test_purple_protocol_xfer_can_receive;
	iface->send_file = test_purple_protocol_xfer_send_file;
	iface->new_xfer = test_purple_protocol_xfer_new_xfer;
}

G_DEFINE_TYPE_WITH_CODE(
	TestPurpleProtocolXfer,
	test_purple_protocol_xfer,
	PURPLE_TYPE_PROTOCOL,
	G_IMPLEMENT_INTERFACE(
		PURPLE_TYPE_PROTOCOL_XFER,
		test_purple_protocol_xfer_iface_init
	)
);

static void
test_purple_protocol_xfer_init(G_GNUC_UNUSED TestPurpleProtocolXfer *prplxfer) {
}

static void
test_purple_protocol_xfer_class_init(G_GNUC_UNUSED TestPurpleProtocolXferClass *klass) {
}

static TestPurpleProtocolXfer *
test_purple_protocol_xfer_new(void) {
	return (TestPurpleProtocolXfer *)g_object_new(
		test_purple_protocol_xfer_get_type(),
		"id", "prpl-xfer",
		NULL);
}

/******************************************************************************
 * Tests
 *****************************************************************************/
static void
test_purple_protocol_xfer_can_receive_func(void) {
	TestPurpleProtocolXfer *xfer = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	gboolean actual = FALSE;

	xfer = test_purple_protocol_xfer_new();
	account = purple_account_new("prpl-xfer-can-receive", "prpl-xfer");
	connection = g_object_new(PURPLE_TYPE_CONNECTION, "account", account, NULL);

	g_assert_true(PURPLE_IS_PROTOCOL_XFER(xfer));

	xfer->can_send = FALSE;
	actual = purple_protocol_xfer_can_receive(PURPLE_PROTOCOL_XFER(xfer),
	                                          connection, "foo");
	g_assert_false(actual);

	xfer->can_send = TRUE;
	actual = purple_protocol_xfer_can_receive(
		PURPLE_PROTOCOL_XFER(xfer),
		connection,
		"foo"
	);
	g_assert_true(actual);

	g_clear_object(&account);
	g_clear_object(&xfer);
}

static void
test_purple_protocol_xfer_send_file_func(void) {
	TestPurpleProtocolXfer *prplxfer = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;

	prplxfer = g_object_new(test_purple_protocol_xfer_get_type(), NULL);
	account = purple_account_new("prpl-xfer-send", "prpl-xfer");
	connection = g_object_new(PURPLE_TYPE_CONNECTION, "account", account, NULL);

	purple_protocol_xfer_send_file(PURPLE_PROTOCOL_XFER(prplxfer), connection,
	                               "foo", "somefile");
	g_assert_true(prplxfer->send_called);

	g_clear_object(&account);
	g_clear_object(&prplxfer);
}

static void
test_purple_protocol_xfer_new_func(void) {
	TestPurpleProtocolXfer *prplxfer = NULL;
	PurpleAccount *account = NULL;
	PurpleConnection *connection = NULL;
	PurpleXfer *xfer = NULL;

	prplxfer = g_object_new(test_purple_protocol_xfer_get_type(), NULL);
	account = purple_account_new("prpl-xfer-new-xfer", "prpl-xfer");
	connection = g_object_new(PURPLE_TYPE_CONNECTION, "account", account, NULL);

	xfer = purple_protocol_xfer_new_xfer(PURPLE_PROTOCOL_XFER(prplxfer),
	                                     connection, "foo");
	g_assert_true(PURPLE_IS_XFER(xfer));
	g_assert_cmpstr("foo", ==, purple_xfer_get_remote_user(xfer));
	g_assert_true(prplxfer->new_xfer_called);

	g_clear_object(&account);
	g_clear_object(&prplxfer);
}

/******************************************************************************
 * Main
 *****************************************************************************/
gint
main(gint argc, gchar **argv) {
	gint res = 0;

	g_test_init(&argc, &argv, NULL);

	g_test_set_nonfatal_assertions();

	test_ui_purple_init();

	g_test_add_func(
		"/protocol-xfer/can-receive",
		test_purple_protocol_xfer_can_receive_func
	);

	g_test_add_func(
		"/protocol-xfer/send-file",
		test_purple_protocol_xfer_send_file_func
	);

	g_test_add_func(
		"/protocol-xfer/new",
		test_purple_protocol_xfer_new_func
	);

	res = g_test_run();

	/* FIXME: We cannot call test_ui_purple_uninit() here because connections
	 * are not easily destroyed if they haven't been fully implemented. */

	return res;
}
