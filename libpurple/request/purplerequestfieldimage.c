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
#include "purplerequestfieldimage.h"

struct _PurpleRequestFieldImage {
	PurpleRequestField parent;

	unsigned int scale_x;
	unsigned int scale_y;
	char *buffer;
	gsize size;
};

enum {
	PROP_0,
	PROP_SCALE_X,
	PROP_SCALE_Y,
	PROP_BUFFER,
	PROP_SIZE,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldImage, purple_request_field_image,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_image_get_property(GObject *obj, guint param_id,
                                        GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldImage *field = PURPLE_REQUEST_FIELD_IMAGE(obj);

	switch(param_id) {
		case PROP_SCALE_X:
			g_value_set_uint(value,
			                 purple_request_field_image_get_scale_x(field));
			break;
		case PROP_SCALE_Y:
			g_value_set_uint(value,
			                 purple_request_field_image_get_scale_y(field));
			break;
		case PROP_BUFFER:
			g_value_set_pointer(value,
			                    (gpointer)purple_request_field_image_get_buffer(field));
			break;
		case PROP_SIZE:
			g_value_set_uint64(value,
			                   purple_request_field_image_get_size(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_image_set_property(GObject *obj, guint param_id,
                                        const GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldImage *field = PURPLE_REQUEST_FIELD_IMAGE(obj);

	switch(param_id) {
		case PROP_SCALE_X:
			purple_request_field_image_set_scale(field,
			                                     g_value_get_uint(value),
			                                     field->scale_y);
			break;
		case PROP_SCALE_Y:
			purple_request_field_image_set_scale(field, field->scale_x,
			                                     g_value_get_uint(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_image_finalize(GObject *obj) {
	PurpleRequestFieldImage *field = PURPLE_REQUEST_FIELD_IMAGE(obj);

	g_free(field->buffer);

	G_OBJECT_CLASS(purple_request_field_image_parent_class)->finalize(obj);
}

static void
purple_request_field_image_init(PurpleRequestFieldImage *field) {
	field->scale_x = 1;
	field->scale_y = 1;
}

static void
purple_request_field_image_class_init(PurpleRequestFieldImageClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_image_finalize;
	obj_class->get_property = purple_request_field_image_get_property;
	obj_class->set_property = purple_request_field_image_set_property;

	/**
	 * PurpleRequestFieldImage:scale-x:
	 *
	 * The X scale coefficient of the image.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SCALE_X] = g_param_spec_uint(
		"scale-x", "scale-x",
		"The X scale coefficient of the image.",
		1, G_MAXUINT, 1,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldImage:scale-y:
	 *
	 * The Y scale coefficient of the image.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SCALE_Y] = g_param_spec_uint(
		"scale-y", "scale-y",
		"The Y scale coefficient of the image.",
		1, G_MAXUINT, 1,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldImage:buffer:
	 *
	 * The contents of the image.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_BUFFER] = g_param_spec_pointer(
		"buffer", "buffer",
		"The contents of the image.",
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestFieldImage:size:
	 *
	 * The size in bytes of the image.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SIZE] = g_param_spec_uint64(
		"size", "size",
		"The size in bytes of the image.",
		0, G_MAXSIZE, 0,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_image_new(const char *id, const char *text,
                               const char *buf, gsize size)
{
	PurpleRequestFieldImage *field;

	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);
	g_return_val_if_fail(buf  != NULL, NULL);
	g_return_val_if_fail(size > 0, NULL);

	field = g_object_new(PURPLE_TYPE_REQUEST_FIELD_IMAGE,
	                     "id", id,
	                     "label", text,
	                     NULL);

	field->buffer = g_memdup2(buf, size);
	field->size = size;

	return PURPLE_REQUEST_FIELD(field);
}

void
purple_request_field_image_set_scale(PurpleRequestFieldImage *field,
                                     unsigned int x, unsigned int y)
{
	g_return_if_fail(PURPLE_IS_REQUEST_FIELD_IMAGE(field));

	if(field->scale_x == x && field->scale_y == y) {
		return;
	}

	field->scale_x = x;
	field->scale_y = y;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_SCALE_X]);
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_SCALE_Y]);
	g_object_thaw_notify(G_OBJECT(field));
}

const char *
purple_request_field_image_get_buffer(PurpleRequestFieldImage *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_IMAGE(field), NULL);

	return field->buffer;
}

gsize
purple_request_field_image_get_size(PurpleRequestFieldImage *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_IMAGE(field), 0);

	return field->size;
}

unsigned int
purple_request_field_image_get_scale_x(PurpleRequestFieldImage *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_IMAGE(field), 0);

	return field->scale_x;
}

unsigned int
purple_request_field_image_get_scale_y(PurpleRequestFieldImage *field)
{
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_IMAGE(field), 0);

	return field->scale_y;
}
