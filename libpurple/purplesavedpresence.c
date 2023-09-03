/*
 * purple
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

#include "purpleenums.h"
#include "purplesavedpresence.h"
#include "util.h"

struct _PurpleSavedPresence {
	GObject parent;

	GSettings *settings;

	GDateTime *last_used;
	guint64 use_count;

	char *id;
	char *name;

	PurplePresencePrimitive primitive;
	char *message;
	char *emoji;
};

enum {
	PROP_0,
	PROP_SETTINGS,
	PROP_LAST_USED,
	PROP_USE_COUNT,
	PROP_ID,
	PROP_NAME,
	PROP_PRIMITIVE,
	PROP_MESSAGE,
	PROP_EMOJI,
	N_PROPERTIES
};
static GParamSpec *properties[N_PROPERTIES];

G_DEFINE_TYPE(PurpleSavedPresence, purple_saved_presence, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static gboolean
purple_saved_presence_get_last_used_mapping(GValue *value, GVariant *variant,
                                            G_GNUC_UNUSED gpointer data)
{
	GDateTime *datetime = NULL;
	GTimeZone *tz = NULL;
	const char *timestamp = NULL;

	timestamp = g_variant_get_string(variant, NULL);
	tz = g_time_zone_new_utc();
	datetime = g_date_time_new_from_iso8601(timestamp, tz);
	g_time_zone_unref(tz);

	if(datetime != NULL) {
		g_value_take_boxed(value, datetime);
	}

	return TRUE;
}

static GVariant *
purple_saved_presence_set_last_used_mapping(const GValue *value,
                                            const GVariantType *expected,
                                            G_GNUC_UNUSED gpointer data)
{
	GDateTime *datetime = NULL;
	char *timestamp = NULL;

	if(!g_variant_type_equal(expected, G_VARIANT_TYPE_STRING)) {
		return NULL;
	}

	datetime = g_value_get_boxed(value);
	if(datetime == NULL) {
		return NULL;
	}

	timestamp = g_date_time_format_iso8601(datetime);
	if(timestamp != NULL) {
		GVariant *variant = NULL;

		variant = g_variant_new_string(timestamp);
		g_free(timestamp);

		return variant;
	}

	return NULL;
}

static void
purple_saved_presence_bind_settings(PurpleSavedPresence *presence,
                                    GSettings *settings)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));
	g_return_if_fail(G_IS_SETTINGS(settings));

	g_settings_bind_with_mapping(settings, "last-used",
	                             presence, "last-used",
	                             G_SETTINGS_BIND_DEFAULT,
	                             purple_saved_presence_get_last_used_mapping,
	                             purple_saved_presence_set_last_used_mapping,
	                             NULL, NULL);
	g_settings_bind(settings, "use-count", presence, "use-count",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "name", presence, "name",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "primitive", presence, "primitive",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "message", presence, "message",
	                G_SETTINGS_BIND_DEFAULT);
	g_settings_bind(settings, "emoji", presence, "emoji",
	                G_SETTINGS_BIND_DEFAULT);
}

static void
purple_saved_presence_set_settings(PurpleSavedPresence *presence,
                                   GSettings *settings)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(G_IS_SETTINGS(settings)) {
		char *schema_id = NULL;

		g_object_get(G_OBJECT(settings), "schema-id", &schema_id, NULL);

		if(!purple_strequal("im.pidgin.Purple.SavedPresence", schema_id)) {
			g_warning("expected schema id of im.pidgin.Purple.SavedPresence, "
			          "but found %s", schema_id);

			g_free(schema_id);

			return;
		}

		g_free(schema_id);
	}

	if(g_set_object(&presence->settings, settings)) {
		g_object_notify_by_pspec(G_OBJECT(presence),
		                         properties[PROP_SETTINGS]);

		purple_saved_presence_bind_settings(presence, settings);
	}
}

static void
purple_saved_presence_set_id(PurpleSavedPresence *presence, const char *id) {
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(!purple_strequal(presence->id, id)) {
		g_free(presence->id);
		presence->id = g_strdup(id);

		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_ID]);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_saved_presence_constructed(GObject *obj) {
	PurpleSavedPresence *presence = PURPLE_SAVED_PRESENCE(obj);

	G_OBJECT_CLASS(purple_saved_presence_parent_class)->constructed(obj);

	if(purple_strempty(presence->id)) {
		char *id = g_uuid_string_random();

		purple_saved_presence_set_id(presence, id);

		g_free(id);
	}
}

static void
purple_saved_presence_set_property(GObject *obj, guint param_id,
                                   const GValue *value, GParamSpec *pspec)
{
	PurpleSavedPresence *presence = PURPLE_SAVED_PRESENCE(obj);

	switch(param_id) {
		case PROP_SETTINGS:
			purple_saved_presence_set_settings(presence,
			                                   g_value_get_object(value));
			break;
		case PROP_LAST_USED:
			purple_saved_presence_set_last_used(presence,
			                                    g_value_get_boxed(value));
			break;
		case PROP_USE_COUNT:
			purple_saved_presence_set_use_count(presence,
			                                    g_value_get_uint64(value));
			break;
		case PROP_ID:
			purple_saved_presence_set_id(presence, g_value_get_string(value));
			break;
		case PROP_NAME:
			purple_saved_presence_set_name(presence,
			                               g_value_get_string(value));
			break;
		case PROP_PRIMITIVE:
			purple_saved_presence_set_primitive(presence,
			                                    g_value_get_enum(value));
			break;
		case PROP_MESSAGE:
			purple_saved_presence_set_message(presence,
			                                  g_value_get_string(value));
			break;
		case PROP_EMOJI:
			purple_saved_presence_set_emoji(presence,
			                                g_value_get_string(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_saved_presence_get_property(GObject *obj, guint param_id, GValue *value,
                                   GParamSpec *pspec)
{
	PurpleSavedPresence *presence = PURPLE_SAVED_PRESENCE(obj);

	switch(param_id) {
		case PROP_LAST_USED:
			g_value_set_boxed(value,
			                  purple_saved_presence_get_last_used(presence));
			break;
		case PROP_USE_COUNT:
			g_value_set_uint64(value,
			                   purple_saved_presence_get_use_count(presence));
			break;
		case PROP_ID:
			g_value_set_string(value,
			                   purple_saved_presence_get_id(presence));
			break;
		case PROP_NAME:
			g_value_set_string(value,
			                   purple_saved_presence_get_name(presence));
			break;
		case PROP_PRIMITIVE:
			g_value_set_enum(value,
			                 purple_saved_presence_get_primitive(presence));
			break;
		case PROP_MESSAGE:
			g_value_set_string(value,
			                   purple_saved_presence_get_message(presence));
			break;
		case PROP_EMOJI:
			g_value_set_string(value,
			                   purple_saved_presence_get_emoji(presence));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_saved_presence_init(G_GNUC_UNUSED PurpleSavedPresence *presence) {
}

static void
purple_saved_presence_finalize(GObject *obj) {
	PurpleSavedPresence *presence = PURPLE_SAVED_PRESENCE(obj);

	g_clear_object(&presence->settings);

	g_clear_pointer(&presence->last_used, g_date_time_unref);

	g_clear_pointer(&presence->id, g_free);
	g_clear_pointer(&presence->name, g_free);
	g_clear_pointer(&presence->message, g_free);
	g_clear_pointer(&presence->emoji, g_free);

	G_OBJECT_CLASS(purple_saved_presence_parent_class)->finalize(obj);
}

static void
purple_saved_presence_class_init(PurpleSavedPresenceClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->constructed = purple_saved_presence_constructed;
	obj_class->finalize = purple_saved_presence_finalize;
	obj_class->get_property = purple_saved_presence_get_property;
	obj_class->set_property = purple_saved_presence_set_property;

	/**
	 * PurpleSavedPresence:settings:
	 *
	 * The [class@Gio.Settings] for this saved presence. This settings object
	 * is typically created by PurplePresenceManager and is expecting the
	 * im.pidgin.Purple.SavedPresence schema.
	 *
	 * When this is non-null, this saved presence will bind all of its
	 * properties to the settings object.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SETTINGS] = g_param_spec_object(
		"settings", "settings",
		"The GSettings for this saved presence.",
		G_TYPE_SETTINGS,
		G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:last-used:
	 *
	 * The [struct@GLib.DateTime] when this saved presence was last used.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_LAST_USED] = g_param_spec_boxed(
		"last-used", "last-used",
		"The time this presence was last used.",
		G_TYPE_DATE_TIME,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:use-count:
	 *
	 * The number of times this saved presence has been used.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_USE_COUNT] = g_param_spec_uint64(
		"use-count", "use-count",
		"The number of times this saved presence has been used.",
		0, G_MAXUINT64, 0,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:id:
	 *
	 * The identifier of the saved presence. If not specified, one will be
	 * randomly generated.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The identifier of this saved presence.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:name:
	 *
	 * The name of the saved presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_NAME] = g_param_spec_string(
		"name", "name",
		"The name of this saved presence.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:primitive:
	 *
	 * The [enum@Purple.StatusPrimitive] for this saved presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_PRIMITIVE] = g_param_spec_enum(
		"primitive", "primitive",
		"The primitive for this saved presence.",
		PURPLE_TYPE_PRESENCE_PRIMITIVE,
		PURPLE_PRESENCE_PRIMITIVE_OFFLINE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:message:
	 *
	 * The status message of this saved presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_MESSAGE] = g_param_spec_string(
		"message", "message",
		"The status message of this saved presence.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleSavedPresence:emoji:
	 *
	 * The emoji or mood of the presence.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_EMOJI] = g_param_spec_string(
		"emoji", "emoji",
		"The emoji for this saved presence.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GDateTime *
purple_saved_presence_get_last_used(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), NULL);

	return presence->last_used;
}

void
purple_saved_presence_set_last_used(PurpleSavedPresence *presence,
                                    GDateTime *last_used)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(presence->last_used != last_used) {
		g_clear_pointer(&presence->last_used, g_date_time_unref);
		if(last_used != NULL) {
			presence->last_used = g_date_time_ref(last_used);
		}

		g_object_notify_by_pspec(G_OBJECT(presence),
		                         properties[PROP_LAST_USED]);
	}
}

guint64
purple_saved_presence_get_use_count(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), 0);

	return presence->use_count;
}

void
purple_saved_presence_set_use_count(PurpleSavedPresence *presence,
                                    guint64 use_count)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(presence->use_count != use_count) {
		presence->use_count = use_count;

		g_object_notify_by_pspec(G_OBJECT(presence),
		                         properties[PROP_USE_COUNT]);
	}
}

const char *
purple_saved_presence_get_id(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), NULL);

	return presence->id;
}

const char *
purple_saved_presence_get_name(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), NULL);

	return presence->name;
}

void
purple_saved_presence_set_name(PurpleSavedPresence *presence,
                               const char *name)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(!purple_strequal(presence->name, name)) {
		g_free(presence->name);
		presence->name = g_strdup(name);

		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_NAME]);
	}
}

PurplePresencePrimitive
purple_saved_presence_get_primitive(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence),
	                     PURPLE_PRESENCE_PRIMITIVE_OFFLINE);

	return presence->primitive;
}

void
purple_saved_presence_set_primitive(PurpleSavedPresence *presence,
                                    PurplePresencePrimitive primitive)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(presence->primitive != primitive) {
		presence->primitive = primitive;

		g_object_notify_by_pspec(G_OBJECT(presence),
		                         properties[PROP_PRIMITIVE]);
	}
}

const char *
purple_saved_presence_get_message(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), NULL);

	return presence->message;
}

void
purple_saved_presence_set_message(PurpleSavedPresence *presence,
                                  const char *message)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(!purple_strequal(presence->message, message)) {
		g_free(presence->message);
		presence->message = g_strdup(message);

		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_MESSAGE]);
	}
}

const char *
purple_saved_presence_get_emoji(PurpleSavedPresence *presence) {
	g_return_val_if_fail(PURPLE_IS_SAVED_PRESENCE(presence), NULL);

	return presence->emoji;
}

void
purple_saved_presence_set_emoji(PurpleSavedPresence *presence,
                                const char *emoji)
{
	g_return_if_fail(PURPLE_IS_SAVED_PRESENCE(presence));

	if(!purple_strequal(presence->emoji, emoji)) {
		g_free(presence->emoji);
		presence->emoji = g_strdup(emoji);

		g_object_notify_by_pspec(G_OBJECT(presence), properties[PROP_EMOJI]);
	}
}

gboolean
purple_saved_presence_equal(PurpleSavedPresence *a, PurpleSavedPresence *b) {
	/* Check if both objects are null. */
	if(a == NULL && b == NULL) {
		return TRUE;
	}

	/* Check if either object is null. */
	if(a == NULL || b == NULL) {
		return FALSE;
	}

	/* Check that both objects are a saved presence. */
	if(!PURPLE_IS_SAVED_PRESENCE(a) || !PURPLE_IS_SAVED_PRESENCE(b)) {
		return FALSE;
	}

	/* If last used is non-null on both compare them. */
	if(a->last_used != NULL && b->last_used != NULL) {
		if(!g_date_time_equal(a->last_used, b->last_used)) {
			return FALSE;
		}
	}

	if(a->last_used == NULL && b->last_used != NULL) {
		return FALSE;
	}

	if(a->last_used != NULL && b->last_used == NULL) {
		return FALSE;
	}

	/* Check the use counts. */
	if(a->use_count != b->use_count) {
		return FALSE;
	}

	/* Check the name. */
	if(!purple_strequal(a->name, b->name)) {
		return FALSE;
	}

	/* Check the primitive. */
	if(a->primitive != b->primitive) {
		return FALSE;
	}

	/* Check the message. */
	if(!purple_strequal(a->message, b->message)) {
		return FALSE;
	}

	/* Check the emoji. */
	if(!purple_strequal(a->emoji, b->emoji)) {
		return FALSE;
	}

	return TRUE;
}
