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

#ifndef PURPLE_REQUEST_GROUP_H
#define PURPLE_REQUEST_GROUP_H

#include <glib.h>
#include <glib-object.h>

/**
 * PurpleRequestGroup:
 *
 * A group of fields with a title.
 *
 * Since: 3.0.0
 */
typedef struct _PurpleRequestGroup PurpleRequestGroup;

#include "purplerequestfield.h"
#include "purplerequestpage.h"

#define PURPLE_TYPE_REQUEST_GROUP (purple_request_group_get_type())
G_DECLARE_FINAL_TYPE(PurpleRequestGroup, purple_request_group,
                     PURPLE, REQUEST_GROUP, GObject)

G_BEGIN_DECLS

/**
 * purple_request_group_new:
 * @title: (nullable): The title to give the group.
 *
 * Creates a fields group with an optional title.
 *
 * Returns: (transfer full): A new fields group
 *
 * Since: 3.0.0
 */
PurpleRequestGroup *purple_request_group_new(const char *title);

/**
 * purple_request_group_add_field:
 * @group: The group to add the field to.
 * @field: (transfer full): The field to add to the group.
 *
 * Adds a field to the group.
 *
 * Since: 3.0.0
 */
void purple_request_group_add_field(PurpleRequestGroup *group, PurpleRequestField *field);

/**
 * purple_request_group_get_title:
 * @group: The group.
 *
 * Returns the title of a fields group.
 *
 * Returns: (nullable): The title, if set.
 *
 * Since: 3.0.0
 */
const char *purple_request_group_get_title(PurpleRequestGroup *group);

/**
 * purple_request_group_get_page:
 * @group: The group.
 *
 * Returns a list of all fields in a group.
 *
 * Returns: (transfer none): The list of fields in the group.
 *
 * Since: 3.0.0
 */
PurpleRequestPage *purple_request_group_get_page(PurpleRequestGroup *group);

/**
 * purple_request_group_is_valid:
 * @group:  The field.
 *
 * Returns whether or not all fields are valid.
 *
 * Returns: %TRUE if all fields in the group are valid, %FALSE otherwise.
 *
 * Since: 3.0.0
 */
gboolean purple_request_group_is_valid(PurpleRequestGroup *group);

G_END_DECLS

#endif /* PURPLE_REQUEST_GROUP_H */
