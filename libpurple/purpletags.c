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

#include "purpletags.h"

#include "util.h"

enum {
	SIG_ADDED,
	SIG_REMOVED,
	N_SIGNALS,
};
static guint signals[N_SIGNALS];

struct _PurpleTags {
	GObject parent;

	GList *tags;
};

G_DEFINE_TYPE(PurpleTags, purple_tags, G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_tags_real_add(PurpleTags *tags, const char *tag, const char *name,
                     const char *value)
{
	/* If this tag exists, remove it. */
	purple_tags_remove(tags, tag);

	/* Add the new tag. */
	tags->tags = g_list_append(tags->tags, g_strdup(tag));

	/* Finally emit the signal. */
	g_signal_emit(tags, signals[SIG_ADDED], 0, tag, name, value);
}

static gboolean
purple_tags_real_remove(PurpleTags *tags, const char *tag, const char *name,
                        const char *value)
{
	/* Walk through the tags looking for the one that was passed in. */
	for(GList *l = tags->tags; l != NULL; l = l->next) {
		gchar *etag = l->data;

		/* If we found it, remove it and exit early. */
		if(purple_strequal(etag, tag)) {
			g_free(etag);
			tags->tags = g_list_delete_link(tags->tags, l);

			g_signal_emit(tags, signals[SIG_REMOVED], 0, tag, name, value);

			return TRUE;
		}
	}

	return FALSE;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_tags_dispose(GObject *obj) {
	PurpleTags *tags = PURPLE_TAGS(obj);

	g_clear_list(&tags->tags, g_free);

	G_OBJECT_CLASS(purple_tags_parent_class)->dispose(obj);
}

static void
purple_tags_init(G_GNUC_UNUSED PurpleTags *tags) {
}

static void
purple_tags_class_init(PurpleTagsClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->dispose = purple_tags_dispose;

	/**
	 * PurpleTags::added:
	 * @tags: The instance.
	 * @tag: The tag value.
	 * @name: The name of the tag.
	 * @value: The value of the tag.
	 *
	 * Emitted when a tag is added. The tag as well as its name and value are
	 * provided to be as flexible as possible.
	 *
	 * > NOTE: When a duplicate tag is added, the original one will be removed
	 * > which will emit the [signal@Tags::removed] signal and then the new tag
	 * > will be added which will emit [signal@Tags::added].
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_ADDED] = g_signal_new_class_handler(
		"added",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		3,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING);

	/**
	 * PurpleTags::removed:
	 * @tags: The instance.
	 * @tag: The tag value.
	 * @name: The name of the tag.
	 * @value: The value of the tag.
	 *
	 * Emitted when a tag is removed. The tag as well as its name and value are
	 * provided to be as flexible as possible.
	 *
	 * > NOTE: When a duplicate tag is added, the original one will be removed
	 * > which will emit the [signal@Tags::removed] signal and then the new tag
	 * > will be added which will emit [signal@Tags::added].
	 *
	 * Since: 3.0.0
	 */
	signals[SIG_REMOVED] = g_signal_new_class_handler(
		"removed",
		G_OBJECT_CLASS_TYPE(klass),
		G_SIGNAL_RUN_LAST,
		NULL,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		3,
		G_TYPE_STRING,
		G_TYPE_STRING,
		G_TYPE_STRING);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleTags *
purple_tags_new(void) {
	return g_object_new(PURPLE_TYPE_TAGS, NULL);
}

const gchar *
purple_tags_lookup(PurpleTags *tags, const gchar *name, gboolean *found) {
	size_t name_len = 0;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), FALSE);
	g_return_val_if_fail(name != NULL, FALSE);

	/* Assume we're going to find the tag, if we don't we set found to false
	 * before we return. This sounds silly, but it saves some additional logic
	 * below.
	 */
	if(found) {
		*found = TRUE;
	}

	name_len = strlen(name);

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		const gchar *tag = l->data;

		if(g_str_has_prefix(tag, name)) {
			const gchar *value = tag + name_len;

			if(*value == '\0') {
				return NULL;
			} else if(*value == ':') {
				return value+1;
			}
		}
	}

	/* We didn't find the tag, so set found to false if necessary. */
	if(found) {
		*found = FALSE;
	}

	return NULL;
}

const gchar *
purple_tags_get(PurpleTags *tags, const gchar *name) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);
	g_return_val_if_fail(name != NULL, NULL);

	return purple_tags_lookup(tags, name, NULL);
}

void
purple_tags_add(PurpleTags *tags, const gchar *tag) {
	char *name = NULL;
	char *value = NULL;

	g_return_if_fail(PURPLE_IS_TAGS(tags));
	g_return_if_fail(tag != NULL);

	purple_tag_parse(tag, &name, &value);
	purple_tags_real_add(tags, tag, name, value);
	g_clear_pointer(&name, g_free);
	g_clear_pointer(&value, g_free);
}

void
purple_tags_add_with_value(PurpleTags *tags, const char *name,
                           const char *value)
{
	char *tag = NULL;

	g_return_if_fail(PURPLE_IS_TAGS(tags));
	g_return_if_fail(name != NULL);

	if(value != NULL) {
		tag = g_strdup_printf("%s:%s", name, value);
	} else {
		tag = g_strdup(name);
	}

	purple_tags_real_add(tags, tag, name, value);

	g_free(tag);
}

gboolean
purple_tags_remove(PurpleTags *tags, const gchar *tag) {
	gchar *name = NULL;
	gchar *value = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), FALSE);
	g_return_val_if_fail(tag != NULL, FALSE);

	purple_tag_parse(tag, &name, &value);
	ret = purple_tags_real_remove(tags, tag, name, value);
	g_clear_pointer(&name, g_free);
	g_clear_pointer(&value, g_free);

	return ret;
}

gboolean
purple_tags_remove_with_value(PurpleTags *tags, const char *name, const char *value) {
	char *tag = NULL;
	gboolean ret = FALSE;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), FALSE);
	g_return_val_if_fail(name != NULL, FALSE);

	/* If there's no value, the tag and name are the same so we can avoid an
	 * unnecessary allocation.
	 */
	if(value == NULL) {
		return purple_tags_real_remove(tags, name, name, NULL);
	}

	tag = g_strdup_printf("%s:%s", name, value);
	ret = purple_tags_real_remove(tags, tag, name, value);
	g_free(tag);

	return ret;
}

guint
purple_tags_get_count(PurpleTags *tags) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), 0);

	return g_list_length(tags->tags);
}

GList *
purple_tags_get_all(PurpleTags *tags) {
	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);

	return tags->tags;
}

GList *
purple_tags_get_all_with_name(PurpleTags *tags, const char *name) {
	GList *ret = NULL;
	size_t name_len = 0;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);
	g_return_val_if_fail(!purple_strempty(name), NULL);

	name_len = strlen(name);

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		char *tag = l->data;

		if(g_str_has_prefix(tag, name)) {
			const char *value = tag + name_len;

			/* Make sure this is a tag with no value and that we didn't get a
			 * partial match on the name.
			 */
			if(*value == '\0' || *value == ':') {
				ret = g_list_prepend(ret, tag);
			}
		}
	}

	return g_list_reverse(ret);
}

gchar *
purple_tags_to_string(PurpleTags *tags, const gchar *separator) {
	GString *value = NULL;

	g_return_val_if_fail(PURPLE_IS_TAGS(tags), NULL);

	value = g_string_new("");

	for(GList *l = tags->tags; l != NULL; l = l->next) {
		const gchar *tag = l->data;

		g_string_append(value, tag);

		if(separator != NULL && l->next != NULL) {
			g_string_append(value, separator);
		}
	}

	return g_string_free(value, FALSE);
}

void
purple_tag_parse(const char *tag, char **name, char **value) {
	const char *colon = NULL;

	g_return_if_fail(tag != NULL);

	colon = g_strstr_len(tag, -1, ":");
	if(colon == NULL) {
		if(name != NULL) {
			*name = g_strdup(tag);
		}
		if(value != NULL) {
			*value = NULL;
		}
	} else {
		if(name != NULL) {
			*name = g_strndup(tag, colon - tag);
		}
		if(value != NULL) {
			*value = g_strdup(colon + 1);
		}
	}
}
