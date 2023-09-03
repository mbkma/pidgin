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

#if !defined(PURPLE_GLOBAL_HEADER_INSIDE) && !defined(PURPLE_COMPILATION)
# error "only <purple.h> may be included directly"
#endif

#ifndef PURPLE_REQUEST_FIELD_IMAGE_H
#define PURPLE_REQUEST_FIELD_IMAGE_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleRequestFieldImage:
 *
 * An image request field.
 *
 * Since: 3.0.0
 */
typedef struct _PurpleRequestFieldImage PurpleRequestFieldImage;

#include "purplerequestfield.h"

G_BEGIN_DECLS

#define PURPLE_TYPE_REQUEST_FIELD_IMAGE (purple_request_field_image_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldImage, purple_request_field_image,
                     PURPLE, REQUEST_FIELD_IMAGE, PurpleRequestField)

/**
 * purple_request_field_image_new:
 * @id:   The field ID.
 * @text: The label of the field.
 * @buf:  The image data.
 * @size: The size of the data in @buf.
 *
 * Creates an image field.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_image_new(const char *id, const char *text, const char *buf, gsize size);

/**
 * purple_request_field_image_set_scale:
 * @field: The image field.
 * @x:     The x scale factor.
 * @y:     The y scale factor.
 *
 * Sets the scale factors of an image field.
 */
void purple_request_field_image_set_scale(PurpleRequestFieldImage *field, unsigned int x, unsigned int y);

/**
 * purple_request_field_image_get_buffer:
 * @field: The image field.
 *
 * Returns pointer to the image.
 *
 * Returns: Pointer to the image.
 */
const char *purple_request_field_image_get_buffer(PurpleRequestFieldImage *field);

/**
 * purple_request_field_image_get_size:
 * @field: The image field.
 *
 * Returns size (in bytes) of the image.
 *
 * Returns: Size of the image.
 */
gsize purple_request_field_image_get_size(PurpleRequestFieldImage *field);

/**
 * purple_request_field_image_get_scale_x:
 * @field: The image field.
 *
 * Returns X scale coefficient of the image.
 *
 * Returns: X scale coefficient of the image.
 */
unsigned int purple_request_field_image_get_scale_x(PurpleRequestFieldImage *field);

/**
 * purple_request_field_image_get_scale_y:
 * @field: The image field.
 *
 * Returns Y scale coefficient of the image.
 *
 * Returns: Y scale coefficient of the image.
 */
unsigned int purple_request_field_image_get_scale_y(PurpleRequestFieldImage *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_IMAGE_H */
