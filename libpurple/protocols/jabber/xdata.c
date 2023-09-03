/*
 * purple - Jabber Protocol Plugin
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include "xdata.h"

typedef enum {
	JABBER_X_DATA_IGNORE = 0,
	JABBER_X_DATA_TEXT_SINGLE,
	JABBER_X_DATA_TEXT_MULTI,
	JABBER_X_DATA_LIST_SINGLE,
	JABBER_X_DATA_LIST_MULTI,
	JABBER_X_DATA_BOOLEAN,
	JABBER_X_DATA_JID_SINGLE
} jabber_x_data_field_type;

struct jabber_x_data_data {
	GHashTable *fields;
	GSList *values;
	GCallback cb;
	gpointer user_data;
	JabberStream *js;
	GList *actions;
	PurpleRequestGroup *actiongroup;
};

static void
jabber_x_data_ok_cb(struct jabber_x_data_data *data, PurpleRequestPage *page) {
	PurpleXmlNode *result = purple_xmlnode_new("x");
	GCallback cb = data->cb;
	gpointer user_data = data->user_data;
	JabberStream *js = data->js;
	char *actionhandle = NULL;
	gboolean hasActions = (data->actions != NULL);
	guint n_groups;

	purple_xmlnode_set_namespace(result, "jabber:x:data");
	purple_xmlnode_set_attrib(result, "type", "submit");

	n_groups = g_list_model_get_n_items(G_LIST_MODEL(page));
	for(guint group_index = 0; group_index < n_groups; group_index++) {
		PurpleRequestGroup *group = NULL;
		guint n_fields;

		group = g_list_model_get_item(G_LIST_MODEL(page), group_index);
		n_fields = g_list_model_get_n_items(G_LIST_MODEL(group));

		if(group == data->actiongroup) {
			for(guint field_index = 0; field_index < n_fields; field_index++) {
				PurpleRequestField *field = NULL;
				PurpleRequestFieldChoice *choice = NULL;
				const char *id = NULL;
				int handleindex;

				field = g_list_model_get_item(G_LIST_MODEL(group), field_index);
				choice = PURPLE_REQUEST_FIELD_CHOICE(field);
				id = purple_request_field_get_id(field);

				if(!purple_strequal(id, "libpurple:jabber:xdata:actions")) {
					g_object_unref(field);
					continue;
				}

				handleindex = GPOINTER_TO_INT(purple_request_field_choice_get_value(choice));
				actionhandle = g_strdup(g_list_nth_data(data->actions, handleindex));
				g_object_unref(field);
				break;
			}

			g_object_unref(group);
			continue;
		}

		for(guint field_index = 0; field_index < n_fields; field_index++) {
			PurpleXmlNode *fieldnode, *valuenode;
			PurpleRequestField *field = NULL;
			const char *id = NULL;
			jabber_x_data_field_type type;

			field = g_list_model_get_item(G_LIST_MODEL(group), field_index);
			id = purple_request_field_get_id(field);
			type = GPOINTER_TO_INT(g_hash_table_lookup(data->fields, id));

			switch(type) {
				case JABBER_X_DATA_TEXT_SINGLE:
				case JABBER_X_DATA_JID_SINGLE:
					{
					PurpleRequestFieldString *sfield = PURPLE_REQUEST_FIELD_STRING(field);
					const char *value = purple_request_field_string_get_value(sfield);
					if (value == NULL)
						break;
					fieldnode = purple_xmlnode_new_child(result, "field");
					purple_xmlnode_set_attrib(fieldnode, "var", id);
					valuenode = purple_xmlnode_new_child(fieldnode, "value");
					purple_xmlnode_insert_data(valuenode, value, -1);
					break;
					}
				case JABBER_X_DATA_TEXT_MULTI:
					{
					PurpleRequestFieldString *sfield = PURPLE_REQUEST_FIELD_STRING(field);
					const char *value = purple_request_field_string_get_value(sfield);
					char **pieces, **p;
					if (value == NULL)
						break;
					fieldnode = purple_xmlnode_new_child(result, "field");
					purple_xmlnode_set_attrib(fieldnode, "var", id);

					pieces = g_strsplit(value, "\n", -1);
					for(p = pieces; *p != NULL; p++) {
						valuenode = purple_xmlnode_new_child(fieldnode, "value");
						purple_xmlnode_insert_data(valuenode, *p, -1);
					}
					g_strfreev(pieces);
					}
					break;
				case JABBER_X_DATA_LIST_SINGLE:
				case JABBER_X_DATA_LIST_MULTI:
					{
					PurpleRequestFieldList *lfield = PURPLE_REQUEST_FIELD_LIST(field);
					GList *selected = purple_request_field_list_get_selected(lfield);
					char *value;
					fieldnode = purple_xmlnode_new_child(result, "field");
					purple_xmlnode_set_attrib(fieldnode, "var", id);

					while(selected) {
						value = purple_request_field_list_get_data(lfield, selected->data);
						valuenode = purple_xmlnode_new_child(fieldnode, "value");
						if(value)
							purple_xmlnode_insert_data(valuenode, value, -1);
						selected = selected->next;
					}
					}
					break;
				case JABBER_X_DATA_BOOLEAN:
					{
					PurpleRequestFieldBool *bfield = PURPLE_REQUEST_FIELD_BOOL(field);
					fieldnode = purple_xmlnode_new_child(result, "field");
					purple_xmlnode_set_attrib(fieldnode, "var", id);
					valuenode = purple_xmlnode_new_child(fieldnode, "value");
					if(purple_request_field_bool_get_value(bfield)) {
						purple_xmlnode_insert_data(valuenode, "1", -1);
					} else {
						purple_xmlnode_insert_data(valuenode, "0", -1);
					}
					}
					break;
				case JABBER_X_DATA_IGNORE:
					break;
			}

			g_object_unref(field);
		}

		g_object_unref(group);
	}

	g_hash_table_destroy(data->fields);
	g_slist_free_full(data->values, g_free);
	g_list_free_full(data->actions, g_free);
	g_free(data);

	if(hasActions) {
		((jabber_x_data_action_cb)cb)(js, result, actionhandle, user_data);
	} else {
		((jabber_x_data_cb)cb)(js, result, user_data);
	}

	g_free(actionhandle);
}

static void
jabber_x_data_cancel_cb(struct jabber_x_data_data *data,
                        G_GNUC_UNUSED PurpleRequestPage *page)
{
	PurpleXmlNode *result = purple_xmlnode_new("x");
	GCallback cb = data->cb;
	gpointer user_data = data->user_data;
	JabberStream *js = data->js;
	gboolean hasActions = (data->actions != NULL);
	g_hash_table_destroy(data->fields);
	g_slist_free_full(data->values, g_free);
	g_list_free_full(data->actions, g_free);
	g_free(data);

	purple_xmlnode_set_namespace(result, "jabber:x:data");
	purple_xmlnode_set_attrib(result, "type", "cancel");

	if(hasActions) {
		((jabber_x_data_action_cb)cb)(js, result, NULL, user_data);
	} else {
		((jabber_x_data_cb)cb)(js, result, user_data);
	}
}

void *jabber_x_data_request(JabberStream *js, PurpleXmlNode *packet, jabber_x_data_cb cb, gpointer user_data)
{
	return jabber_x_data_request_with_actions(js, packet, NULL, 0,
	                                          (jabber_x_data_action_cb)(GCallback)cb,
	                                          user_data);
}

void *jabber_x_data_request_with_actions(JabberStream *js, PurpleXmlNode *packet, GList *actions, int defaultaction, jabber_x_data_action_cb cb, gpointer user_data)
{
	void *handle;
	PurpleXmlNode *fn, *x;
	PurpleRequestPage *page;
	PurpleRequestGroup *group;
	PurpleRequestField *field = NULL;

	char *title = NULL;
	char *instructions = NULL;

	struct jabber_x_data_data *data = g_new0(struct jabber_x_data_data, 1);

	data->fields = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, NULL);
	data->user_data = user_data;
	data->cb = G_CALLBACK(cb);
	data->js = js;

	page = purple_request_page_new();
	group = purple_request_group_new(NULL);
	purple_request_page_add_group(page, group);

	for(fn = purple_xmlnode_get_child(packet, "field"); fn; fn = purple_xmlnode_get_next_twin(fn)) {
		PurpleXmlNode *valuenode;
		const char *type = purple_xmlnode_get_attrib(fn, "type");
		const char *label = purple_xmlnode_get_attrib(fn, "label");
		const char *var = purple_xmlnode_get_attrib(fn, "var");
		char *value = NULL;

		if(!type)
			type = "text-single";

		if(!var && !purple_strequal(type, "fixed"))
			continue;
		if(!label)
			label = var;

		if(purple_strequal(type, "text-private")) {
			if((valuenode = purple_xmlnode_get_child(fn, "value")))
				value = purple_xmlnode_get_data(valuenode);

			field = purple_request_field_string_new(var, label,
					value ? value : "", FALSE);
			purple_request_field_string_set_masked(PURPLE_REQUEST_FIELD_STRING(field),
			                                       TRUE);
			purple_request_group_add_field(group, field);

			g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_TEXT_SINGLE));

			g_free(value);
		} else if(purple_strequal(type, "text-multi") || purple_strequal(type, "jid-multi")) {
			GString *str = g_string_new("");

			for(valuenode = purple_xmlnode_get_child(fn, "value"); valuenode;
					valuenode = purple_xmlnode_get_next_twin(valuenode)) {

				if(!(value = purple_xmlnode_get_data(valuenode)))
					continue;

				g_string_append_printf(str, "%s\n", value);
				g_free(value);
			}

			field = purple_request_field_string_new(var, label,
					str->str, TRUE);
			purple_request_group_add_field(group, field);

			g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_TEXT_MULTI));

			g_string_free(str, TRUE);
		} else if(purple_strequal(type, "list-single") || purple_strequal(type, "list-multi")) {
			PurpleRequestFieldList *list_field = NULL;
			PurpleXmlNode *optnode;
			GList *selected = NULL;

			field = purple_request_field_list_new(var, label);
			list_field = PURPLE_REQUEST_FIELD_LIST(field);

			if(purple_strequal(type, "list-multi")) {
				purple_request_field_list_set_multi_select(list_field, TRUE);
				g_hash_table_replace(data->fields, g_strdup(var),
						GINT_TO_POINTER(JABBER_X_DATA_LIST_MULTI));
			} else {
				g_hash_table_replace(data->fields, g_strdup(var),
						GINT_TO_POINTER(JABBER_X_DATA_LIST_SINGLE));
			}

			for(valuenode = purple_xmlnode_get_child(fn, "value"); valuenode;
					valuenode = purple_xmlnode_get_next_twin(valuenode)) {
				char *data = purple_xmlnode_get_data(valuenode);
				if (data != NULL) {
					selected = g_list_prepend(selected, data);
				}
			}

			for(optnode = purple_xmlnode_get_child(fn, "option"); optnode;
					optnode = purple_xmlnode_get_next_twin(optnode)) {
				const char *lbl;

				if(!(valuenode = purple_xmlnode_get_child(optnode, "value")))
					continue;

				if(!(value = purple_xmlnode_get_data(valuenode)))
					continue;

				if(!(lbl = purple_xmlnode_get_attrib(optnode, "label")))
					lbl = value;

				data->values = g_slist_prepend(data->values, value);

				purple_request_field_list_add_icon(list_field, lbl, NULL, value);
				if(g_list_find_custom(selected, value, (GCompareFunc)strcmp)) {
					purple_request_field_list_add_selected(list_field, lbl);
				}
			}
			purple_request_group_add_field(group, field);

			g_list_free_full(selected, g_free);
		} else if(purple_strequal(type, "boolean")) {
			gboolean def = FALSE;

			if((valuenode = purple_xmlnode_get_child(fn, "value")))
				value = purple_xmlnode_get_data(valuenode);

			if(value && (!g_ascii_strcasecmp(value, "yes") ||
						!g_ascii_strcasecmp(value, "true") || !g_ascii_strcasecmp(value, "1")))
				def = TRUE;

			field = purple_request_field_bool_new(var, label, def);
			purple_request_group_add_field(group, field);

			g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_BOOLEAN));

			g_free(value);
		} else if(purple_strequal(type, "fixed")) {
			if((valuenode = purple_xmlnode_get_child(fn, "value")))
				value = purple_xmlnode_get_data(valuenode);

			if(value != NULL) {
				field = purple_request_field_label_new("", value);
				purple_request_group_add_field(group, field);

				g_free(value);
			}
		} else if(purple_strequal(type, "hidden")) {
			if((valuenode = purple_xmlnode_get_child(fn, "value")))
				value = purple_xmlnode_get_data(valuenode);

			field = purple_request_field_string_new(var, "", value ? value : "",
					FALSE);
			purple_request_field_set_visible(field, FALSE);
			purple_request_group_add_field(group, field);

			g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_TEXT_SINGLE));

			g_free(value);
		} else { /* text-single, jid-single, and the default */
			if((valuenode = purple_xmlnode_get_child(fn, "value")))
				value = purple_xmlnode_get_data(valuenode);

			field = purple_request_field_string_new(var, label,
					value ? value : "", FALSE);
			purple_request_group_add_field(group, field);

			if(purple_strequal(type, "jid-single")) {
				purple_request_field_set_type_hint(field, "screenname");
				g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_JID_SINGLE));
			} else {
				g_hash_table_replace(data->fields, g_strdup(var), GINT_TO_POINTER(JABBER_X_DATA_TEXT_SINGLE));
			}

			g_free(value);
		}

		if(field && purple_xmlnode_get_child(fn, "required"))
			purple_request_field_set_required(field,TRUE);
	}

	if(actions != NULL) {
		PurpleRequestField *field = NULL;
		PurpleRequestFieldChoice *choice = NULL;
		GList *action;
		int i;

		data->actiongroup = group = purple_request_group_new(_("Actions"));
		purple_request_page_add_group(page, group);
		field = purple_request_field_choice_new("libpurple:jabber:xdata:actions",
		                                        _("Select an action"),
		                                        GINT_TO_POINTER(defaultaction));
		choice = PURPLE_REQUEST_FIELD_CHOICE(field);

		for(i = 0, action = actions; action; action = g_list_next(action), i++) {
			JabberXDataAction *a = action->data;

			purple_request_field_choice_add(choice, a->name, GINT_TO_POINTER(i));
			data->actions = g_list_append(data->actions, g_strdup(a->handle));
		}
		purple_request_field_set_required(field, TRUE);
		purple_request_group_add_field(group, field);
	}

	if((x = purple_xmlnode_get_child(packet, "title")))
		title = purple_xmlnode_get_data(x);

	if((x = purple_xmlnode_get_child(packet, "instructions")))
		instructions = purple_xmlnode_get_data(x);

	handle = purple_request_fields(js->gc, title, title, instructions, page,
			_("OK"), G_CALLBACK(jabber_x_data_ok_cb),
			_("Cancel"), G_CALLBACK(jabber_x_data_cancel_cb),
			purple_request_cpar_from_connection(js->gc),
			data);

	g_free(title);
	g_free(instructions);

	return handle;
}

gchar *
jabber_x_data_get_formtype(const PurpleXmlNode *form)
{
	PurpleXmlNode *field;

	g_return_val_if_fail(form != NULL, NULL);

	for (field = purple_xmlnode_get_child((PurpleXmlNode *)form, "field"); field;
			field = purple_xmlnode_get_next_twin(field)) {
		const char *var = purple_xmlnode_get_attrib(field, "var");
		if (purple_strequal(var, "FORM_TYPE")) {
			PurpleXmlNode *value = purple_xmlnode_get_child(field, "value");
			if (value)
				return purple_xmlnode_get_data(value);
			else
				/* An interesting corner case... Looking for a second
				 * FORM_TYPE would be more considerate, but I'm in favor
				 * of not helping broken clients.
				 */
				return NULL;
		}
	}

	/* Erm, none found :( */
	return NULL;
}

