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

#include "purplerequestfield.h"
#include "purplerequestfieldlist.h"
#include "purplekeyvaluepair.h"

struct _PurpleRequestFieldList {
	PurpleRequestField parent;

	GList *items;
	gboolean has_icons;
	GHashTable *item_data;
	GList *selected;
	GHashTable *selected_table;

	gboolean multiple_selection;
};

enum {
	PROP_0,
	PROP_MULTI_SELECT,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldList, purple_request_field_list,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_list_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PurpleRequestFieldList *field = PURPLE_REQUEST_FIELD_LIST(obj);

	switch(param_id) {
		case PROP_MULTI_SELECT:
			g_value_set_boolean(value,
			                    purple_request_field_list_get_multi_select(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_list_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldList *field = PURPLE_REQUEST_FIELD_LIST(obj);

	switch(param_id) {
		case PROP_MULTI_SELECT:
			purple_request_field_list_set_multi_select(field,
			                                           g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_list_finalize(GObject *obj) {
	PurpleRequestFieldList *field = PURPLE_REQUEST_FIELD_LIST(obj);

	g_list_free_full(field->items, (GDestroyNotify)purple_key_value_pair_free);
	g_list_free_full(field->selected, g_free);
	g_hash_table_destroy(field->item_data);
	g_hash_table_destroy(field->selected_table);

	G_OBJECT_CLASS(purple_request_field_list_parent_class)->finalize(obj);
}

static void
purple_request_field_list_init(PurpleRequestFieldList *field) {
	field->item_data = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                         g_free, NULL);

	field->selected_table = g_hash_table_new_full(g_str_hash, g_str_equal,
	                                              g_free, NULL);
}

static void
purple_request_field_list_class_init(PurpleRequestFieldListClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_list_finalize;
	obj_class->get_property = purple_request_field_list_get_property;
	obj_class->set_property = purple_request_field_list_set_property;

	/**
	 * PurpleRequestFieldChoice:multi-select:
	 *
	 * Whether the field should allow multiple selections.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MULTI_SELECT] = g_param_spec_boolean(
		"multi-select", "multi-select",
		"Whether the field should allow multiple selections.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_list_new(const char *id, const char *text) {
	g_return_val_if_fail(id   != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_LIST,
	                    "id", id,
	                    "label", text,
	                    NULL);
}

void
purple_request_field_list_set_multi_select(PurpleRequestFieldList *field,
                                           gboolean multi_select)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field));

	if(field->multiple_selection == multi_select) {
		return;
	}

	field->multiple_selection = multi_select;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_MULTI_SELECT]);
}

gboolean
purple_request_field_list_get_multi_select(PurpleRequestFieldList *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), FALSE);

	return field->multiple_selection;
}

gpointer
purple_request_field_list_get_data(PurpleRequestFieldList *field,
                                   const char *text)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), NULL);
	g_return_val_if_fail(text  != NULL, NULL);

	return g_hash_table_lookup(field->item_data, text);
}

void
purple_request_field_list_add_icon(PurpleRequestFieldList *field,
                                   const char *item, const char *icon_path,
                                   gpointer data)
{
	PurpleKeyValuePair *kvp;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field));
	g_return_if_fail(item  != NULL);
	g_return_if_fail(data  != NULL);

	field->has_icons = field->has_icons || (icon_path != NULL);
	kvp = purple_key_value_pair_new_full(item, g_strdup(icon_path), g_free);
	field->items = g_list_append(field->items, kvp);
	g_hash_table_insert(field->item_data, g_strdup(item), data);
	g_object_notify(G_OBJECT(field), "valid");
}

void
purple_request_field_list_add_selected(PurpleRequestFieldList *field,
                                       const char *item)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field));
	g_return_if_fail(item  != NULL);

	if(!field->multiple_selection && field->selected != NULL) {
		g_warning("More than one item added to non-multi-select field %s",
		          purple_request_field_get_id(PURPLE_REQUEST_FIELD(field)));
		return;
	}

	field->selected = g_list_append(field->selected, g_strdup(item));

	g_hash_table_add(field->selected_table, g_strdup(item));
	g_object_notify(G_OBJECT(field), "valid");
}

void
purple_request_field_list_clear_selected(PurpleRequestFieldList *field) {
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field));

	g_clear_list(&field->selected, g_free);

	g_hash_table_remove_all(field->selected_table);
	g_object_notify(G_OBJECT(field), "valid");
}

void
purple_request_field_list_set_selected(PurpleRequestFieldList *field,
                                       GList *items)
{
	GList *l;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field));
	g_return_if_fail(items != NULL);

	purple_request_field_list_clear_selected(field);

	if (!purple_request_field_list_get_multi_select(field) && items->next) {
		g_warning("More than one item added to non-multi-select field %s",
		          purple_request_field_get_id(PURPLE_REQUEST_FIELD(field)));
		return;
	}

	for (l = items; l != NULL; l = l->next) {
		char *selected = l->data;
		field->selected = g_list_append(field->selected, g_strdup(selected));
		g_hash_table_add(field->selected_table, g_strdup(selected));
	}

	g_object_notify(G_OBJECT(field), "valid");
}

gboolean
purple_request_field_list_is_selected(PurpleRequestFieldList *field,
                                      const char *item)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), FALSE);
	g_return_val_if_fail(item  != NULL, FALSE);

	return g_hash_table_contains(field->selected_table, item);
}

GList *
purple_request_field_list_get_selected(PurpleRequestFieldList *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), NULL);

	return field->selected;
}

GList *
purple_request_field_list_get_items(PurpleRequestFieldList *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), NULL);

	return field->items;
}

gboolean
purple_request_field_list_has_icons(PurpleRequestFieldList *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_LIST(field), FALSE);

	return field->has_icons;
}
