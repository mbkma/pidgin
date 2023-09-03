/* purple
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

#include "purplerequestgroup.h"
#include "purplerequestpage.h"
#include "purpleprivate.h"

struct _PurpleRequestGroup {
	GObject parent;

	char *title;

	GPtrArray *fields;
	GHashTable *invalid_fields;
};

enum {
	PROP_0,
	PROP_TITLE,
	PROP_VALID,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_request_group_set_title(PurpleRequestGroup *group, const char *title) {
	g_free(group->title);
	group->title = g_strdup(title);

	g_object_notify_by_pspec(G_OBJECT(group), properties[PROP_TITLE]);
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static void
purple_request_group_notify_field_cb(GObject *obj,
                                     G_GNUC_UNUSED GParamSpec *pspec,
                                     gpointer data)
{
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(data);
	PurpleRequestField *field = PURPLE_REQUEST_FIELD(obj);
	gboolean before, after;

	before = purple_request_group_is_valid(group);
	if(purple_request_field_is_valid(field, NULL)) {
		g_hash_table_remove(group->invalid_fields, field);
	} else {
		g_hash_table_add(group->invalid_fields, field);
	}
	after = purple_request_group_is_valid(group);

	if(before != after) {
		g_object_notify_by_pspec(G_OBJECT(group), properties[PROP_VALID]);
	}
}

/******************************************************************************
 * GListModel Implementation
 *****************************************************************************/
static GType
purple_request_group_get_item_type(G_GNUC_UNUSED GListModel *model) {
	return PURPLE_TYPE_REQUEST_FIELD;
}

static guint
purple_request_group_get_n_items(GListModel *model) {
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(model);

	return group->fields->len;
}

static gpointer
purple_request_group_get_item(GListModel *model, guint index) {
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(model);
	PurpleRequestField *field = NULL;

	if(index < group->fields->len) {
		field = g_ptr_array_index(group->fields, index);
		g_object_ref(field);
	}

	return field;
}

static void
purple_request_group_list_model_init(GListModelInterface *iface) {
	iface->get_item_type = purple_request_group_get_item_type;
	iface->get_item = purple_request_group_get_item;
	iface->get_n_items = purple_request_group_get_n_items;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE_WITH_CODE(PurpleRequestGroup, purple_request_group, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(G_TYPE_LIST_MODEL,
                                              purple_request_group_list_model_init))

static void
purple_request_group_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(obj);

	switch(param_id) {
		case PROP_TITLE:
			g_value_set_string(value, purple_request_group_get_title(group));
			break;
		case PROP_VALID:
			g_value_set_boolean(value, purple_request_group_is_valid(group));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_group_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(obj);

	switch(param_id) {
		case PROP_TITLE:
			purple_request_group_set_title(group, g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_group_finalize(GObject *obj) {
	PurpleRequestGroup *group = PURPLE_REQUEST_GROUP(obj);

	g_free(group->title);

	g_clear_pointer(&group->fields, g_ptr_array_unref);
	g_clear_pointer(&group->invalid_fields, g_hash_table_destroy);

	G_OBJECT_CLASS(purple_request_group_parent_class)->finalize(obj);
}

static void
purple_request_group_init(PurpleRequestGroup *group) {
	group->fields = g_ptr_array_new_with_free_func(g_object_unref);
	group->invalid_fields = g_hash_table_new(g_direct_hash, g_direct_equal);
}

static void
purple_request_group_class_init(PurpleRequestGroupClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_group_finalize;
	obj_class->get_property = purple_request_group_get_property;
	obj_class->set_property = purple_request_group_set_property;

	/**
	 * PurpleRequestGroup:title:
	 *
	 * The title of the field group.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TITLE] = g_param_spec_string(
		"title", "title",
		"The title of the field group.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestGroup:valid:
	 *
	 * Whether all fields in a group are valid.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALID] = g_param_spec_boolean(
		"valid", "valid",
		"Whether all fields in a group are valid.",
		TRUE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestGroup *
purple_request_group_new(const char *title) {
	return g_object_new(PURPLE_TYPE_REQUEST_GROUP,
	                    "title", title,
	                    NULL);
}

void
purple_request_group_add_field(PurpleRequestGroup *group,
                               PurpleRequestField *field)
{
	guint position;

	g_return_if_fail(PURPLE_IS_REQUEST_GROUP(group));
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	position = group->fields->len;
	g_ptr_array_add(group->fields, field);

	purple_request_group_notify_field_cb(G_OBJECT(field), NULL, group);
	g_signal_connect(field, "notify::valid",
	                 G_CALLBACK(purple_request_group_notify_field_cb), group);

	g_list_model_items_changed(G_LIST_MODEL(group), position, 0, 1);
}

const char *
purple_request_group_get_title(PurpleRequestGroup *group)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_GROUP(group), NULL);

	return group->title;
}

gboolean
purple_request_group_is_valid(PurpleRequestGroup *group) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_GROUP(group), FALSE);

	return g_hash_table_size(group->invalid_fields) == 0;
}
