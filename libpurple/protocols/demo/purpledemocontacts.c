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

#include <json-glib/json-glib.h>

#include "purpledemocontacts.h"

#include "purpledemoresource.h"

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_demo_contacts_load_contact_icon(PurpleContactInfo *info,
                                       const char *name)
{
	GdkPixbuf *pixbuf = NULL;
	GError *error = NULL;
	char *path = NULL;

	path = g_strdup_printf("/im/pidgin/purple/demo/buddy_icons/%s.png", name);
	pixbuf = gdk_pixbuf_new_from_resource(path, &error);

	if(error != NULL) {
		g_message("Failed to load find an icon for %s: %s", path,
		          error->message);

		g_free(path);
		g_clear_error(&error);

		return;
	}

	g_free(path);

	if(GDK_IS_PIXBUF(pixbuf)) {
		purple_contact_info_set_avatar(info, pixbuf);

		g_clear_object(&pixbuf);
	}
}

static void
purple_demo_contacts_load_contact_person(JsonObject *person_object,
                                         PurpleContactInfo *info)
{
	PurplePerson *person = NULL;
	gboolean new_person = FALSE;
	const char *value = NULL;

	/* If the person has an id, grab it so we can use it when constructing the
	 * person object.
	 */
	if(json_object_has_member(person_object, "id")) {
		value = json_object_get_string_member(person_object, "id");
	}

	/* See if the contact has an existing person. */
	person = purple_contact_info_get_person(info);
	if(PURPLE_IS_PERSON(person)) {
		const char *existing_id = NULL;

		/* If the existing person's id doesn't match the new one, NULL out
		 * person so it'll be recreated with the new id.
		 */
		existing_id = purple_person_get_id(person);
		if(!purple_strequal(existing_id, value)) {
			person = NULL;
		}
	}

	/* If the person didn't exist or it had a different id, create a new person
	 * with the id.
	 */
	if(!PURPLE_IS_PERSON(person)) {
		person = g_object_new(PURPLE_TYPE_PERSON, "id", value, NULL);
		new_person = TRUE;
	}

	/* Alias */
	if(json_object_has_member(person_object, "alias")) {
		value = json_object_get_string_member(person_object, "alias");
		if(!purple_strempty(value)) {
			purple_person_set_alias(person, value);
		}
	}

	/* Create the link between the person and the contact info. */
	if(new_person) {
		purple_person_add_contact_info(person, info);
		purple_contact_info_set_person(info, person);

		g_clear_object(&person);
	}
}

static void
purple_demo_contacts_load_contact_presence(JsonObject *presence_object,
                                           PurpleContactInfo *info)
{
	PurplePresence *presence = NULL;
	const gchar *value = NULL;

	presence = purple_contact_info_get_presence(info);

	/* Emoji */
	if(json_object_has_member(presence_object, "emoji")) {
		value = json_object_get_string_member(presence_object, "emoji");
		if(!purple_strempty(value)) {
			purple_presence_set_emoji(presence, value);
		}
	}

	/* Idle Time */
	if(json_object_has_member(presence_object, "idle")) {
		GDateTime *now = NULL;
		GDateTime *idle_since = NULL;
		gint64 ivalue = 0;

		ivalue = json_object_get_int_member(presence_object, "idle");

		now = g_date_time_new_now_local();
		idle_since = g_date_time_add_minutes(now, -1 * ivalue);

		purple_presence_set_idle(presence, TRUE, idle_since);

		g_date_time_unref(idle_since);
		g_date_time_unref(now);
	}

	/* Message */
	if(json_object_has_member(presence_object, "message")) {
		value = json_object_get_string_member(presence_object, "message");
		if(!purple_strempty(value)) {
			purple_presence_set_message(presence, value);
		}
	}

	/* Mobile */
	if(json_object_has_member(presence_object, "mobile")) {
		gboolean bvalue = FALSE;
		bvalue = json_object_get_boolean_member(presence_object, "mobile");
		purple_presence_set_mobile(presence, bvalue);
	}

	/* Primitive */
	if(json_object_has_member(presence_object, "primitive")) {
		PurplePresencePrimitive primitive = PURPLE_PRESENCE_PRIMITIVE_OFFLINE;

		value = json_object_get_string_member(presence_object, "primitive");
		if(!purple_strempty(value)) {
			GEnumClass *enum_class = NULL;
			GEnumValue *enum_value = NULL;

			enum_class = g_type_class_ref(PURPLE_TYPE_PRESENCE_PRIMITIVE);
			enum_value = g_enum_get_value_by_nick(enum_class, value);

			if(enum_value != NULL) {
				primitive = enum_value->value;
			}

			g_type_class_unref(enum_class);
		}

		purple_presence_set_primitive(presence, primitive);
	}
}

static void
purple_demo_contacts_load_contact(PurpleContactManager *manager,
                                  PurpleAccount *account,
                                  JsonObject *contact_object)
{
	PurpleContact *contact = NULL;
	PurpleContactInfo *info = NULL;
	gboolean new_contact = FALSE;
	const char *id = NULL;
	const char *value = NULL;

	/* If we have an id, grab so we can create the contact with it. */
	if(json_object_has_member(contact_object, "id")) {
		id = json_object_get_string_member(contact_object, "id");
	}

	/* Look for an existing contact before creating a new one. This stops us
	 * from getting multiples when we trigger connection errors.
	 */
	if(!purple_strempty(id)) {
		contact = purple_contact_manager_find_with_id(manager, account, id);
	}

	/* If we didn't find an existing contact, create it now with the provided
	 * id.
	 */
	if(!PURPLE_IS_CONTACT(contact)) {
		contact = purple_contact_new(account, id);
		new_contact = TRUE;
	}

	info = PURPLE_CONTACT_INFO(contact);

	/* Alias */
	if(json_object_has_member(contact_object, "alias")) {
		value = json_object_get_string_member(contact_object, "alias");
		if(!purple_strempty(value)) {
			purple_contact_info_set_alias(info, value);
		}
	}

	/* Color */
	if(json_object_has_member(contact_object, "color")) {
		value = json_object_get_string_member(contact_object, "color");
		if(!purple_strempty(value)) {
			purple_contact_info_set_color(info, value);
		}
	}

	/* Display Name */
	if(json_object_has_member(contact_object, "display_name")) {
		value = json_object_get_string_member(contact_object, "display_name");
		if(!purple_strempty(value)) {
			purple_contact_info_set_display_name(info, value);
		}
	}

	/* Username */
	if(json_object_has_member(contact_object, "username")) {
		value = json_object_get_string_member(contact_object, "username");
		if(!purple_strempty(value)) {
			purple_contact_info_set_username(info, value);

			purple_demo_contacts_load_contact_icon(info, value);
		}
	}

	/* Load the tags. */
	if(json_object_has_member(contact_object, "tags")) {
		PurpleTags *tags = purple_contact_info_get_tags(info);
		JsonArray *array = NULL;
		GList *elements = NULL;

		array = json_object_get_array_member(contact_object, "tags");
		elements = json_array_get_elements(array);
		while(elements != NULL) {
			JsonNode *tag_node = elements->data;
			const char *tag = json_node_get_string(tag_node);

			purple_tags_add(tags, tag);

			elements = g_list_delete_link(elements, elements);
		}
	}

	/* Load the person. */
	if(json_object_has_member(contact_object, "person")) {
		JsonObject *person_object = NULL;

		person_object = json_object_get_object_member(contact_object,
		                                              "person");

		purple_demo_contacts_load_contact_person(person_object, info);
	}

	/* Load the presence. */
	if(json_object_has_member(contact_object, "presence")) {
		JsonObject *presence_object = NULL;

		presence_object = json_object_get_object_member(contact_object,
		                                                "presence");

		purple_demo_contacts_load_contact_presence(presence_object, info);
	}

	/* Finally add the contact to the contact manager if it's new. */
	if(new_contact) {
		purple_contact_manager_add(manager, contact);
	}

	g_clear_object(&contact);
}

/******************************************************************************
 * Local Exports
 *****************************************************************************/
void
purple_demo_contacts_load(PurpleAccount *account) {
	PurpleContactManager *manager = NULL;
	GError *error = NULL;
	GInputStream *istream = NULL;
	GList *contacts = NULL;
	JsonArray *contacts_array = NULL;
	JsonNode *root_node = NULL;
	JsonParser *parser = NULL;

	/* get a stream to the contacts.json resource */
	istream = g_resource_open_stream(purple_demo_get_resource(),
	                                 "/im/pidgin/purple/demo/contacts.json",
	                                 G_RESOURCE_LOOKUP_FLAGS_NONE, NULL);

	/* create our parser */
	parser = json_parser_new();

	if(!json_parser_load_from_stream(parser, istream, NULL, &error)) {
		g_critical("%s", error->message);
		g_clear_error(&error);
		return;
	}

	root_node = json_parser_get_root(parser);

	manager = purple_contact_manager_get_default();

	/* Load the contacts! */
	contacts_array = json_node_get_array(root_node);
	contacts = json_array_get_elements(contacts_array);
	while(contacts != NULL) {
		JsonNode *contact_node = NULL;
		JsonObject *contact_object = NULL;

		contact_node = contacts->data;
		contact_object = json_node_get_object(contact_node);

		purple_demo_contacts_load_contact(manager, account, contact_object);

		contacts = g_list_delete_link(contacts, contacts);
	}

	/* Clean up everything else... */
	g_clear_object(&parser);

	g_input_stream_close(istream, NULL, NULL);
	g_object_unref(istream);
}
