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
#include "purplerequestfielddatasheet.h"

struct _PurpleRequestFieldDatasheet {
	PurpleRequestField parent;

	PurpleRequestDatasheet *sheet;
};

enum {
	PROP_0,
	PROP_SHEET,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_request_field_datasheet_set_sheet(PurpleRequestFieldDatasheet *field,
                                         PurpleRequestDatasheet *sheet)
{
	g_clear_pointer(&field->sheet, purple_request_datasheet_free);
	field->sheet = sheet;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_SHEET]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
G_DEFINE_TYPE(PurpleRequestFieldDatasheet, purple_request_field_datasheet,
              PURPLE_TYPE_REQUEST_FIELD)

static void
purple_request_field_datasheet_get_property(GObject *obj, guint param_id,
                                            GValue *value, GParamSpec *pspec)
{
	PurpleRequestFieldDatasheet *field = PURPLE_REQUEST_FIELD_DATASHEET(obj);

	switch(param_id) {
		case PROP_SHEET:
			g_value_set_pointer(value,
			                    purple_request_field_datasheet_get_sheet(field));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_datasheet_set_property(GObject *obj, guint param_id,
                                            const GValue *value,
                                            GParamSpec *pspec)
{
	PurpleRequestFieldDatasheet *field = PURPLE_REQUEST_FIELD_DATASHEET(obj);

	switch(param_id) {
		case PROP_SHEET:
			purple_request_field_datasheet_set_sheet(field,
			                                         g_value_get_pointer(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_datasheet_finalize(GObject *obj) {
	PurpleRequestFieldDatasheet *field = PURPLE_REQUEST_FIELD_DATASHEET(obj);

	g_clear_pointer(&field->sheet, purple_request_datasheet_free);

	G_OBJECT_CLASS(purple_request_field_datasheet_parent_class)->finalize(obj);
}

static void
purple_request_field_datasheet_init(G_GNUC_UNUSED PurpleRequestFieldDatasheet *field) {
}

static void
purple_request_field_datasheet_class_init(PurpleRequestFieldDatasheetClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_datasheet_finalize;
	obj_class->get_property = purple_request_field_datasheet_get_property;
	obj_class->set_property = purple_request_field_datasheet_set_property;

	/**
	 * PurpleRequestFieldDatasheet:sheet:
	 *
	 * The datasheet of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SHEET] = g_param_spec_pointer(
		"sheet", "sheet",
		"The datasheet of the field.",
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
PurpleRequestField *
purple_request_field_datasheet_new(const char *id, const char *text,
                                   PurpleRequestDatasheet *sheet)
{
	g_return_val_if_fail(id != NULL, NULL);
	g_return_val_if_fail(sheet != NULL, NULL);

	return g_object_new(PURPLE_TYPE_REQUEST_FIELD_DATASHEET,
	                    "id", id,
	                    "label", text,
	                    "sheet", sheet,
	                    NULL);
}

PurpleRequestDatasheet *
purple_request_field_datasheet_get_sheet(PurpleRequestFieldDatasheet *field) {
	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD_DATASHEET(field), NULL);

	return field->sheet;
}
