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
#include "purplerequestfieldlabel.h"

struct _PurpleRequestFieldLabel {
	PurpleRequestField parent;
};

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldLabel, purple_request_field_label,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_label_init(G_GNUC_UNUSED PurpleRequestFieldLabel *field) {
}

static void
purple_request_field_label_class_init(G_GNUC_UNUSED PurpleRequestFieldLabelClass *klass) {
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_label_new(const char *id, const char *text) {
	g_return_val_if_fail(id   != NULL, NULL);
	g_return_val_if_fail(text != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_LABEL,
	                    "id", id,
	                    "label", text,
	                    NULL);
}
