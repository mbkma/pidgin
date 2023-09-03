/* purple
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

#include "request.h"
#include "purpleprivate.h"

typedef struct {
	char *id;
	char *label;
	char *type_hint;

	gboolean visible;
	gboolean required;
	gboolean sensitive;

	char *tooltip;

	GClosure *validator;
} PurpleRequestFieldPrivate;

enum {
	PROP_0,
	PROP_ID,
	PROP_LABEL,
	PROP_VISIBLE,
	PROP_SENSITIVE,
	PROP_TYPE_HINT,
	PROP_TOOLTIP,
	PROP_REQUIRED,
	PROP_FILLED,
	PROP_VALID,
	N_PROPERTIES,
};
static GParamSpec *properties[N_PROPERTIES] = {NULL, };

G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE(PurpleRequestField, purple_request_field,
                                    G_TYPE_OBJECT)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static void
purple_request_field_set_id(PurpleRequestField *field, const char *id) {
	PurpleRequestFieldPrivate *priv = NULL;

	priv = purple_request_field_get_instance_private(field);

	g_free(priv->id);
	priv->id = g_strdup(id);

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_ID]);
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
purple_request_field_get_property(GObject *obj, guint param_id, GValue *value,
                                  GParamSpec *pspec)
{
	PurpleRequestField *field = PURPLE_REQUEST_FIELD(obj);

	switch(param_id) {
		case PROP_ID:
			g_value_set_string(value, purple_request_field_get_id(field));
			break;
		case PROP_LABEL:
			g_value_set_string(value, purple_request_field_get_label(field));
			break;
		case PROP_VISIBLE:
			g_value_set_boolean(value, purple_request_field_is_visible(field));
			break;
		case PROP_SENSITIVE:
			g_value_set_boolean(value,
			                    purple_request_field_is_sensitive(field));
			break;
		case PROP_TYPE_HINT:
			g_value_set_string(value,
			                   purple_request_field_get_type_hint(field));
			break;
		case PROP_TOOLTIP:
			g_value_set_string(value, purple_request_field_get_tooltip(field));
			break;
		case PROP_REQUIRED:
			g_value_set_boolean(value,
			                    purple_request_field_is_required(field));
			break;
		case PROP_FILLED:
			g_value_set_boolean(value,
			                    purple_request_field_is_filled(field));
			break;
		case PROP_VALID:
			g_value_set_boolean(value,
			                    purple_request_field_is_valid(field, NULL));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_set_property(GObject *obj, guint param_id,
                                  const GValue *value, GParamSpec *pspec)
{
	PurpleRequestField *field = PURPLE_REQUEST_FIELD(obj);

	switch(param_id) {
		case PROP_ID:
			purple_request_field_set_id(field, g_value_get_string(value));
			break;
		case PROP_LABEL:
			purple_request_field_set_label(field, g_value_get_string(value));
			break;
		case PROP_VISIBLE:
			purple_request_field_set_visible(field,
			                                 g_value_get_boolean(value));
			break;
		case PROP_SENSITIVE:
			purple_request_field_set_sensitive(field,
			                                   g_value_get_boolean(value));
			break;
		case PROP_TYPE_HINT:
			purple_request_field_set_type_hint(field,
			                                   g_value_get_string(value));
			break;
		case PROP_TOOLTIP:
			purple_request_field_set_tooltip(field, g_value_get_string(value));
			break;
		case PROP_REQUIRED:
			purple_request_field_set_required(field,
			                                  g_value_get_boolean(value));
			break;
		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, param_id, pspec);
			break;
	}
}

static void
purple_request_field_finalize(GObject *obj) {
	PurpleRequestField *field = PURPLE_REQUEST_FIELD(obj);
	PurpleRequestFieldPrivate *priv = NULL;

	priv = purple_request_field_get_instance_private(field);

	g_free(priv->id);
	g_free(priv->label);
	g_free(priv->type_hint);
	g_free(priv->tooltip);
	g_clear_pointer(&priv->validator, g_closure_unref);

	G_OBJECT_CLASS(purple_request_field_parent_class)->finalize(obj);
}

static void
purple_request_field_init(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	priv = purple_request_field_get_instance_private(field);

	priv->visible = TRUE;
	priv->sensitive = TRUE;
}

static void
purple_request_field_class_init(PurpleRequestFieldClass *klass) {
	GObjectClass *obj_class = G_OBJECT_CLASS(klass);

	obj_class->finalize = purple_request_field_finalize;
	obj_class->get_property = purple_request_field_get_property;
	obj_class->set_property = purple_request_field_set_property;

	/**
	 * PurpleRequestField:id:
	 *
	 * The ID of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_ID] = g_param_spec_string(
		"id", "id",
		"The ID of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:label:
	 *
	 * The label of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_LABEL] = g_param_spec_string(
		"label", "label",
		"The label of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:visible:
	 *
	 * Whether the field should be visible.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VISIBLE] = g_param_spec_boolean(
		"visible", "visible",
		"Whether the field should be visible.",
		TRUE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:sensitive:
	 *
	 * Whether the field should be sensitive.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_SENSITIVE] = g_param_spec_boolean(
		"sensitive", "sensitive",
		"Whether the field should be sensitive.",
		TRUE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:type-hint:
	 *
	 * The type hint of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TYPE_HINT] = g_param_spec_string(
		"type-hint", "type-hint",
		"The type hint of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:tooltip:
	 *
	 * The tooltip of the field.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_TOOLTIP] = g_param_spec_string(
		"tooltip", "tooltip",
		"The tooltip of the field.",
		NULL,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:required:
	 *
	 * Whether the field is required to complete the request.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_REQUIRED] = g_param_spec_boolean(
		"required", "required",
		"Whether the field is required to complete the request.",
		FALSE,
		G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:filled:
	 *
	 * Whether the field has been filled.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_FILLED] = g_param_spec_boolean(
		"filled", "filled",
		"Whether the field has been filled.",
		TRUE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	/**
	 * PurpleRequestField:valid:
	 *
	 * Whether the field has a valid value.
	 *
	 * Since: 3.0.0
	 */
	properties[PROP_VALID] = g_param_spec_boolean(
		"valid", "valid",
		"Whether the field has a valid value.",
		TRUE,
		G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(obj_class, N_PROPERTIES, properties);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
void
purple_request_field_set_label(PurpleRequestField *field, const char *label)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	g_free(priv->label);
	priv->label = g_strdup(label);

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_LABEL]);
}

void
purple_request_field_set_visible(PurpleRequestField *field, gboolean visible)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	priv->visible = visible;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VISIBLE]);
}

void
purple_request_field_set_type_hint(PurpleRequestField *field,
								 const char *type_hint)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	g_free(priv->type_hint);
	priv->type_hint = g_strdup(type_hint);

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_TYPE_HINT]);
}

void
purple_request_field_set_tooltip(PurpleRequestField *field, const char *tooltip)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	g_free(priv->tooltip);
	priv->tooltip = g_strdup(tooltip);

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_TOOLTIP]);
}

void
purple_request_field_set_required(PurpleRequestField *field, gboolean required)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	if(priv->required == required) {
		return;
	}

	priv->required = required;

	g_object_freeze_notify(G_OBJECT(field));
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_REQUIRED]);
	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALID]);
	g_object_thaw_notify(G_OBJECT(field));
}

const char *
purple_request_field_get_id(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), NULL);

	priv = purple_request_field_get_instance_private(field);

	return priv->id;
}

const char *
purple_request_field_get_label(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), NULL);

	priv = purple_request_field_get_instance_private(field);

	return priv->label;
}

gboolean
purple_request_field_is_visible(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), FALSE);

	priv = purple_request_field_get_instance_private(field);

	return priv->visible;
}

const char *
purple_request_field_get_type_hint(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), NULL);

	priv = purple_request_field_get_instance_private(field);

	return priv->type_hint;
}

const char *
purple_request_field_get_tooltip(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), NULL);

	priv = purple_request_field_get_instance_private(field);

	return priv->tooltip;
}

gboolean
purple_request_field_is_required(PurpleRequestField *field) {
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), FALSE);

	priv = purple_request_field_get_instance_private(field);

	return priv->required;
}

gboolean
purple_request_field_is_filled(PurpleRequestField *field) {
	PurpleRequestFieldClass *klass = NULL;
	gboolean filled = TRUE;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), FALSE);

	klass = PURPLE_REQUEST_FIELD_GET_CLASS(field);
	if(klass != NULL && klass->is_filled != NULL) {
		filled = klass->is_filled(field);
	}

	return filled;
}

void
purple_request_field_set_validator(PurpleRequestField *field,
                                   PurpleRequestFieldValidator validator,
                                   gpointer user_data,
                                   GDestroyNotify destroy_data)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	g_clear_pointer(&priv->validator, g_closure_unref);
	if(validator != NULL) {
		priv->validator = g_cclosure_new(G_CALLBACK(validator), user_data,
		                                 (GClosureNotify)G_CALLBACK(destroy_data));
		g_closure_ref(priv->validator);
		g_closure_sink(priv->validator);
		g_closure_set_marshal(priv->validator, g_cclosure_marshal_generic);
	}

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_VALID]);
}

gboolean
purple_request_field_is_valid(PurpleRequestField *field, gchar **errmsg)
{
	PurpleRequestFieldClass *klass = NULL;
	PurpleRequestFieldPrivate *priv = NULL;
	gboolean valid = TRUE;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), FALSE);

	if(errmsg != NULL) {
		*errmsg = NULL;
	}

	klass = PURPLE_REQUEST_FIELD_GET_CLASS(field);
	if(klass != NULL && klass->is_valid != NULL) {
		valid = klass->is_valid(field, errmsg);
	}

	priv = purple_request_field_get_instance_private(field);
	if(valid && priv->validator != NULL) {
		GValue result = G_VALUE_INIT;
		GValue params[] = {G_VALUE_INIT, G_VALUE_INIT};
		g_value_init(&result, G_TYPE_BOOLEAN);
		g_value_set_instance(g_value_init(&params[0],
		                                  PURPLE_TYPE_REQUEST_FIELD),
		                     field);
		g_value_set_pointer(g_value_init(&params[1], G_TYPE_POINTER), errmsg);
		g_closure_invoke(priv->validator, &result,
		                 G_N_ELEMENTS(params), params, NULL);
		valid = g_value_get_boolean(&result);
		g_value_unset(&result);
		for(gsize i = 0; i < G_N_ELEMENTS(params); i++) {
			g_value_unset(&params[i]);
		}
	}

	if(valid && purple_request_field_is_required(field) &&
	   !purple_request_field_is_filled(field))
	{
		if(errmsg != NULL) {
			*errmsg = g_strdup(_("Required field is not filled."));
		}
		valid = FALSE;
	}

	if(!valid && errmsg != NULL && *errmsg == NULL) {
		*errmsg = g_strdup(_("Validation failed without setting an error "
		                     "message."));
	}

	return valid;
}

void
purple_request_field_set_sensitive(PurpleRequestField *field,
	gboolean sensitive)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_if_fail(PURPLE_IS_REQUEST_FIELD(field));

	priv = purple_request_field_get_instance_private(field);

	priv->sensitive = sensitive;

	g_object_notify_by_pspec(G_OBJECT(field), properties[PROP_SENSITIVE]);
}

gboolean
purple_request_field_is_sensitive(PurpleRequestField *field)
{
	PurpleRequestFieldPrivate *priv = NULL;

	g_return_val_if_fail(PURPLE_IS_REQUEST_FIELD(field), FALSE);

	priv = purple_request_field_get_instance_private(field);

	return priv->sensitive;
}
