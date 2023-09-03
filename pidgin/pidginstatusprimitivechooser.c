/*
 * Pidgin - Internet Messenger
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
 *
 * Pidgin is the legal property of its developers, whose names are too numerous
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

#include <pidgin/pidginstatusprimitivechooser.h>

#include <pidgin/pidginiconname.h>

struct _PidginStatusPrimitiveChooser {
	AdwComboRow parent;
};

G_DEFINE_TYPE(PidginStatusPrimitiveChooser, pidgin_status_primitive_chooser,
              ADW_TYPE_COMBO_ROW)

/******************************************************************************
 * Callbacks
 *****************************************************************************/
static PurpleStatusPrimitive
pidgin_status_primitive_chooser_primitive_cb(G_GNUC_UNUSED GObject *self,
                                             GtkStringObject *object,
                                             G_GNUC_UNUSED gpointer data)
{
	PurpleStatusPrimitive primitive = PURPLE_STATUS_UNSET;

	if(GTK_IS_STRING_OBJECT(object)) {
		const char *value = gtk_string_object_get_string(object);
		primitive = purple_primitive_get_type_from_id(value);
	}

	return primitive;
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_status_primitive_chooser_init(PidginStatusPrimitiveChooser *chooser) {
	gtk_widget_init_template(GTK_WIDGET(chooser));
}

static void
pidgin_status_primitive_chooser_class_init(PidginStatusPrimitiveChooserClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	/* Widget template */
	gtk_widget_class_set_template_from_resource(
	        widget_class, "/im/pidgin/Pidgin3/statusprimitivechooser.ui");

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_status_primitive_chooser_primitive_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_status_primitive_chooser_new(void) {
	return g_object_new(PIDGIN_TYPE_STATUS_PRIMITIVE_CHOOSER, NULL);
}

PurpleStatusPrimitive
pidgin_status_primitive_chooser_get_selected(PidginStatusPrimitiveChooser *chooser) {
	GtkStringObject *selected = NULL;
	const char *value = NULL;

	g_return_val_if_fail(PIDGIN_IS_STATUS_PRIMITIVE_CHOOSER(chooser),
	                     PURPLE_STATUS_UNSET);

	selected = adw_combo_row_get_selected_item(ADW_COMBO_ROW(chooser));
	value = gtk_string_object_get_string(selected);

	return purple_primitive_get_type_from_id(value);
}

void
pidgin_status_primitive_chooser_set_selected(PidginStatusPrimitiveChooser *chooser,
                                             PurpleStatusPrimitive primitive)
{
	GListModel *model = NULL;
	GtkStringList *list = NULL;

	g_return_if_fail(PIDGIN_IS_STATUS_PRIMITIVE_CHOOSER(chooser));

	model = adw_combo_row_get_model(ADW_COMBO_ROW(chooser));
	list = GTK_STRING_LIST(model);
	for(guint i = 0; i < g_list_model_get_n_items(model); i++) {
		PurpleStatusPrimitive candidate = PURPLE_STATUS_UNSET;
		const char *value = NULL;

		value = gtk_string_list_get_string(list, i);
		candidate = purple_primitive_get_type_from_id(value);

		if(primitive == candidate) {
			adw_combo_row_set_selected(ADW_COMBO_ROW(chooser), i);
			break;
		}
	}
}
