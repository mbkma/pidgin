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

#ifndef PURPLE_REQUEST_FIELD_DATASHEET_H
#define PURPLE_REQUEST_FIELD_DATASHEET_H

#include <glib.h>
#include <glib-object.h>

#include "request.h"
#include "request-datasheet.h"

G_BEGIN_DECLS

/**
 * PurpleRequestFieldDatasheet:
 *
 * A datasheet request field.
 *
 * Since: 3.0.0
 */
#define PURPLE_TYPE_REQUEST_FIELD_DATASHEET (purple_request_field_datasheet_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestFieldDatasheet, purple_request_field_datasheet,
                     PURPLE, REQUEST_FIELD_DATASHEET, PurpleRequestField)

/**
 * purple_request_field_datasheet_new:
 * @id:    The field ID.
 * @text:  The label of the field, may be %NULL.
 * @sheet: The datasheet.
 *
 * Creates a datasheet item field.
 *
 * Returns: (transfer full): The new field.
 */
PurpleRequestField *purple_request_field_datasheet_new(const char *id, const gchar *text, PurpleRequestDatasheet *sheet);

/**
 * purple_request_field_datasheet_get_sheet:
 * @field: The field.
 *
 * Returns a datasheet for a field.
 *
 * Returns: (transfer none): The datasheet object.
 */
PurpleRequestDatasheet *purple_request_field_datasheet_get_sheet(PurpleRequestFieldDatasheet *field);

G_END_DECLS

#endif /* PURPLE_REQUEST_FIELD_DATASHEET_H */
