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

#include "purplerequestpage.h"
#include "request/purplerequestfieldaccount.h"
#include "request/purplerequestfieldbool.h"
#include "request/purplerequestfieldchoice.h"
#include "request/purplerequestfieldint.h"
#include "request/purplerequestfieldstring.h"
#include "purpleprivate.h"

struct _PurpleRequestPage {
	GObject parent;

	GPtrArray *groups;
	GHashTable *invalid_groups;

	GHashTable *fields;
};

enum {
	PROP_0,
	PROP_VALID,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_request_page_notify_group_cb(GObject *obj,
                                    G_GNUC_UNUSED GParamSpec *pspec,
                                    gpointer data)
{
	PurpleRequestPage *page = PURPLE_REQUEST_PAGE(data);
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(obj);
	gboolean before, after;

	before = purple_request_page_is_valid(page);
	if(purple_request_group_is_valid(group)) {
		g_hash_table_remove(page->invalid_groups, group);
	} else {
		g_hash_table_add(page->invalid_groups, group);
	}
	after = purple_request_page_is_valid(page);

	if(before != after) {
		g_object_notify_by_pspec(G_OBJECT(page), properties[PROP_VALID]);
	}
}

static void
purple_request_page_items_changed_cb(GListModel *list, guint position,
                                     guint removed, guint added, gpointer data)
{
	PurpleRequestPage *page = data;

	/* Groups don't support removing fields, nor do pages support removing
	 * groups, so we don't attempt to support that here. */
	g_return_if_fail(removed == 0);

	for(guint offset = 0; offset < added; offset++) {
		PurpleRequestField *field;
		field = g_list_model_get_item(list, position + offset);
		g_hash_table_insert(page->fields,
		                    g_strdup(purple_request_field_get_id(field)),
		                    field);
		g_object_unref(field);
	}
}

/******************************************************************************
 * GListModel Implementation
 *****************************************************************************/
static GType
purple_request_page_get_item_type(G_GNUC_UNUSED GListModel *model) {
	return PURPLE_TYPE_REQUEST_GROUP;
}

static guint
purple_request_page_get_n_items(GListModel *model) {
	PurpleRequestPage *page = PURPLE_REQUEST_PAGE(model);

	return page->groups->len;
}

static gpointer
purple_request_page_get_item(GListModel *model, guint index) {
	PurpleRequestPage *page = PURPLE_REQUEST_PAGE(model);
	PurpleRequestGroup *group = NULL;

	if(index < page->groups->len) {
		group = g_ptr_array_index(page->groups, index);
		g_object_ref(group);
	}

	return group;
}

static void
purple_request_page_list_model_init(GListModelInterface *iface) {
	iface->get_item_type = purple_request_page_get_item_type;
	iface->get_item = purple_request_page_get_item;
	iface->get_n_items = purple_request_page_get_n_items;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_WITH_CODE(PurpleRequestPage, purple_request_page, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL,
                                              purple_request_page_list_model_init))

static void
purple_request_page_get_property(GObject *obj, guint param_id, GValue *value,
                                 GParamSpec *pspec)
{
	PurpleRequestPage *page = PURPLE_REQUEST_PAGE(obj);

	switch(param_id) {
		case PROP_VALID:
			g_value_set_boolean(value, purple_request_page_is_valid(page));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_page_finalize(GObject *obj) {
	PurpleRequestPage *page = PURPLE_REQUEST_PAGE(obj);

	g_clear_pointer(&page->groups, g_ptr_array_unref);
	g_clear_pointer(&page->invalid_groups, g_hash_table_destroy);
	g_hash_table_destroy(page->fields);

	G_OBJECT_CLASS(purple_request_page_parent_class)->finalize(obj);
}

static void
purple_request_page_init(PurpleRequestPage *page) {
	page->groups = g_ptr_array_new_with_free_func(g_object_unref);
	page->fields = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	page->invalid_groups = g_hash_table_new(g_direct_hash, g_direct_equal);
}

static void
purple_request_page_class_init(PurpleRequestPageClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_page_finalize;
	obj_class->get_property = purple_request_page_get_property;

	/**
	 * PurpleRequestPage:valid:
	 *
	 * Whether all fields in a page are valid.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALID] = g_param_spec_boolean(
		"valid", "valid",
		"Whether all fields in a page are valid.",
		TRUE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestPage *
purple_request_page_new(void) {
	return g_object_new(PURPLE_TYPE_REQUEST_PAGE, NULL);
}

void
purple_request_page_add_group(PurpleRequestPage *page,
                              PurpleRequestGroup *group)
{
	guint position;
	GListModel *model = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_PAGE(page));
	g_return_if_fail(PURPLE_IS_REQUEST_GROUP(group));

	position = page->groups->len;
	g_ptr_array_add(page->groups, group);

	purple_request_page_notify_group_cb(G_OBJECT(group), NULL, page);
	g_signal_connect(group, "notify::valid",
	                 G_CALLBACK(purple_request_page_notify_group_cb), page);

	model = G_LIST_MODEL(group);
	purple_request_page_items_changed_cb(model, 0, 0,
	                                     g_list_model_get_n_items(model),
	                                     page);
	g_signal_connect(group, "items-changed",
	                 G_CALLBACK(purple_request_page_items_changed_cb), page);

	g_list_model_items_changed(G_LIST_MODEL(page), position, 0, 1);
}

gboolean
purple_request_page_exists(PurpleRequestPage *page, const char *id) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), FALSE);
	g_return_val_if_fail(id     != NULL, FALSE);

	return (g_hash_table_lookup(page->fields, id) != NULL);
}

gboolean
purple_request_page_is_field_required(PurpleRequestPage *page, const char *id)
{
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), FALSE);
	g_return_val_if_fail(id     != NULL, FALSE);

	if((field = purple_request_page_get_field(page, id)) == NULL) {
		return FALSE;
	}

	return purple_request_field_is_required(field);
}

gboolean
purple_request_page_is_valid(PurpleRequestPage *page) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), FALSE);

	return g_hash_table_size(page->invalid_groups) == 0;
}

PurpleRequestField *
purple_request_page_get_field(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), NULL);
	g_return_val_if_fail(id     != NULL, NULL);

	field = g_hash_table_lookup(page->fields, id);

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), NULL);

	return field;
}

const char *
purple_request_page_get_string(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), NULL);
	g_return_val_if_fail(id     != NULL, NULL);

	field = purple_request_page_get_field(page, id);
	if(!PURPLE_IS_REQUEST_FIELD_STRING(field)) {
		return NULL;
	}

	return purple_request_field_string_get_value(PURPLE_REQUEST_FIELD_STRING(field));
}

int
purple_request_page_get_integer(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), 0);
	g_return_val_if_fail(id     != NULL, 0);

	field = purple_request_page_get_field(page, id);
	if(!PURPLE_IS_REQUEST_FIELD_INT(field)) {
		return 0;
	}

	return purple_request_field_int_get_value(PURPLE_REQUEST_FIELD_INT(field));
}

gboolean
purple_request_page_get_bool(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), FALSE);
	g_return_val_if_fail(id     != NULL, FALSE);

	field = purple_request_page_get_field(page, id);
	if(!PURPLE_IS_REQUEST_FIELD_BOOL(field)) {
		return FALSE;
	}

	return purple_request_field_bool_get_value(PURPLE_REQUEST_FIELD_BOOL(field));
}

gpointer
purple_request_page_get_choice(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), NULL);
	g_return_val_if_fail(id != NULL, NULL);

	field = purple_request_page_get_field(page, id);
	if(!PURPLE_IS_REQUEST_FIELD_CHOICE(field)) {
		return NULL;
	}

	return purple_request_field_choice_get_value(PURPLE_REQUEST_FIELD_CHOICE(field));
}

PurpleAccount *
purple_request_page_get_account(PurpleRequestPage *page, const char *id) {
	PurpleRequestField *field;

	g_return_val_if_fail(PURPLE_IS_REQUEST_PAGE(page), NULL);
	g_return_val_if_fail(id     != NULL, NULL);

	field = purple_request_page_get_field(page, id);
	if(!PURPLE_IS_REQUEST_FIELD_ACCOUNT(field)) {
		return NULL;
	}

	return purple_request_field_account_get_value(PURPLE_REQUEST_FIELD_ACCOUNT(field));
}
