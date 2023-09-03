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

#include <glib/gi18n-lib.h>

#include "purpleaccountmanager.h"
#include "purplerequestfield.h"
#include "purplerequestfieldaccount.h"

struct _PurpleRequestFieldAccount {
	PurpleRequestField parent;

	PurpleAccount *default_account;
	PurpleAccount *account;
	gboolean show_all;

	GClosure *filter_func;
};

enum {
	PROP_0,
	PROP_DEFAULT_VALUE,
	PROP_VALUE,
	PROP_SHOW_ALL,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldAccount, purple_request_field_account,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_account_get_property(GObject *obj, guint param_id,
                                          GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldAccount *field = PURPLE_REQUEST_FIELD_ACCOUNT(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			g_value_set_object(value,
			                   purple_request_field_account_get_default_value(field));
			break;
		case PROP_VALUE:
			g_value_set_object(value,
			                   purple_request_field_account_get_value(field));
			break;
		case PROP_SHOW_ALL:
			g_value_set_boolean(value,
			                    purple_request_field_account_get_show_all(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_account_set_property(GObject *obj, guint param_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
	PurpleRequestFieldAccount *field = PURPLE_REQUEST_FIELD_ACCOUNT(obj);

	switch(param_id) {
		case PROP_DEFAULT_VALUE:
			purple_request_field_account_set_default_value(field,
			                                               g_value_get_object(value));
			break;
		case PROP_VALUE:
			purple_request_field_account_set_value(field,
			                                       g_value_get_object(value));
			break;
		case PROP_SHOW_ALL:
			purple_request_field_account_set_show_all(field,
			                                          g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_account_finalize(GObject *obj) {
	PurpleRequestFieldAccount *field = PURPLE_REQUEST_FIELD_ACCOUNT(obj);

	g_clear_object(&field->default_account);
	g_clear_object(&field->account);
	g_clear_pointer(&field->filter_func, g_closure_unref);

	G_OBJECT_CLASS(purple_request_field_account_parent_class)->finalize(obj);
}

static void
purple_request_field_account_init(G_GNUC_UNUSED PurpleRequestFieldAccount *field)
{
}

static void
purple_request_field_account_class_init(PurpleRequestFieldAccountClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_account_finalize;
	obj_class->get_property = purple_request_field_account_get_property;
	obj_class->set_property = purple_request_field_account_set_property;

	/**
	 * PurpleRequestFieldAccount:default-value:
	 *
	 * The default value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_DEFAULT_VALUE] = g_param_spec_object(
		"default-value", "default-value",
		"The default value of the field.",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldAccount:value:
	 *
	 * The value of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALUE] = g_param_spec_object(
		"value", "value",
		"The value of the field.",
		PURPLE_TYPE_ACCOUNT,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldAccount:show-all:
	 *
	 * Whether to show all accounts in an account field.
	 *
	 * If %TRUE, all accounts, online or offline, will be shown. If %FALSE,
	 * only online accounts will be shown.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SHOW_ALL] = g_param_spec_boolean(
		"show-all", "show-all",
		"Whether to show all accounts in an account field.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_account_new(const char *id, const char *text,
                                 PurpleAccount *account)
{
	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	if(account == NULL) {
		PurpleAccountManager *manager = purple_account_manager_get_default();
		GList *accounts = purple_account_manager_get_connected(manager);

		if(accounts != NULL) {
			account = accounts->data;
			g_list_free(accounts);
		}
	}

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_ACCOUNT,
	                    "id", id,
	                    "label", text,
	                    "default-value", account,
	                    "value", account,
	                    NULL);
}

void
purple_request_field_account_set_default_value(PurpleRequestFieldAccount *field,
                                               PurpleAccount *default_value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field));

	if(g_set_object(&field->default_account, default_value)) {
		g_object_notify_by_pspec(G_OBJECT(field),
		                         properties[PROP_DEFAULT_VALUE]);
	}
}

void
purple_request_field_account_set_value(PurpleRequestFieldAccount *field,
                                       PurpleAccount *value)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field));

	if(g_set_object(&field->account, value)) {
		g_object_freeze_notify(G_OBJECT(field));
		g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALUE]);
		g_object_notify(G_OBJECT(field), "valid");
		g_object_thaw_notify(G_OBJECT(field));
	}
}

void
purple_request_field_account_set_show_all(PurpleRequestFieldAccount *field,
                                          gboolean show_all)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field));

	if(field->show_all == show_all) {
		return;
	}

	field->show_all = show_all;

	g_object_freeze_notify(G_OBJECT(field));

	if(!show_all) {
		PurpleAccountManager *manager = purple_account_manager_get_default();
		GList *accounts = purple_account_manager_get_connected(manager);

		if(purple_account_is_connected(field->default_account)) {
			purple_request_field_account_set_default_value(field,
			                                               accounts->data);
		}

		if(purple_account_is_connected(field->account)) {
			purple_request_field_account_set_value(field, accounts->data);
		}

		g_list_free(accounts);
	}

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_SHOW_ALL]);

	g_object_thaw_notify(G_OBJECT(field));
}

void
purple_request_field_account_set_filter(PurpleRequestFieldAccount *field,
                                        PurpleRequestFieldAccountFilterFunc filter_func,
                                        gpointer user_data,
                                        GDestroyNotify destroy_data)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field));

	g_clear_pointer(&field->filter_func, g_closure_unref);
	if(filter_func != NULL) {
		field->filter_func = g_cclosure_new(G_CALLBACK(filter_func), user_data,
		                                    (GClosureNotify)G_CALLBACK(destroy_data));
		g_closure_ref(field->filter_func);
		g_closure_sink(field->filter_func);
		g_closure_set_marshal(field->filter_func, g_cclosure_marshal_generic);
	}
}

PurpleAccount *
purple_request_field_account_get_default_value(PurpleRequestFieldAccount *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field), NULL);

	return field->default_account;
}

PurpleAccount *
purple_request_field_account_get_value(PurpleRequestFieldAccount *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field), NULL);

	return field->account;
}

gboolean
purple_request_field_account_get_show_all(PurpleRequestFieldAccount *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field), FALSE);

	return field->show_all;
}

gboolean
purple_request_field_account_match(PurpleRequestFieldAccount *field,
                                   PurpleAccount *account)
{
	gboolean ret = TRUE;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_ACCOUNT(field), FALSE);

	if(field->filter_func != NULL) {
		GValue result = G_VALUE_INIT;
		GValue params[] = {G_VALUE_INIT};
		g_value_init(&result, G_TYPE_BOOLEAN);
		g_value_set_instance(g_value_init(&params[0], PURPLE_TYPE_ACCOUNT),
		                     account);
		g_closure_invoke(field->filter_func, &result,
		                 G_N_ELEMENTS(params), params, NULL);
		ret = g_value_get_boolean(&result);
		g_value_unset(&result);
		for(gsize i = 0; i < G_N_ELEMENTS(params); i++) {
			g_value_unset(&params[i]);
		}
	}


	return ret;
}
