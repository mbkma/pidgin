/*
 * finch
 *
 * Finch is the legal property of its developers, whose names are too numerous
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include <glib/gi18n-lib.h>

#include <purple.h>

#include <gnt.h>

#include "gntprefs.h"
#include "gntrequest.h"


#include <string.h>

static struct {
	GList *list_data;  /* Data to be freed when the pref-window is closed */
	gboolean showing;
	GntWidget *window;
} pref_request;

void
finch_prefs_init(void)
{
	purple_prefs_add_none("/finch");

	purple_prefs_add_none("/finch/plugins");
	purple_prefs_add_path_list("/finch/plugins/loaded", NULL);
	purple_prefs_add_path_list("/finch/plugins/seen", NULL);

	purple_prefs_add_none("/finch/conversations");
	purple_prefs_add_bool("/finch/conversations/timestamps", TRUE);
	purple_prefs_add_bool("/finch/conversations/notify_typing", FALSE);

	purple_prefs_add_none("/finch/filelocations");
	purple_prefs_add_path("/finch/filelocations/last_save_folder", "");
	purple_prefs_add_path("/finch/filelocations/last_save_folder", "");
}

void
finch_prefs_update_old(void)
{
}

typedef struct {
	PurplePrefType type;
	const char *pref;
	const char *label;
	GList *(*choices)(void);   /* If the value is to be selected from a number of choices */
} Prefs;

static GList *
get_idle_options(void) {
	GList *list = NULL;
	PurpleKeyValuePair *pair = NULL;

	pair = purple_key_value_pair_new(_("Based on keyboard use"), "system");
	list = g_list_append(list, pair);

	pair = purple_key_value_pair_new(_("From last sent message"), "purple");
	list = g_list_append(list, pair);

	pair = purple_key_value_pair_new(_("Never"), "never");
	list = g_list_append(list, pair);

	return list;
}

static GList *
get_status_titles(void) {
	GList *list = NULL;

	for(GList *iter = purple_savedstatuses_get_all(); iter; iter = iter->next) {
		PurpleKeyValuePair *pair = NULL;
		const char *title = NULL;
		char *str = NULL;

		if(purple_savedstatus_is_transient(iter->data)) {
			continue;
		}

		str = g_strdup_printf("%ld", purple_savedstatus_get_creation_time(iter->data));
		title = purple_savedstatus_get_title(iter->data);
		pair = purple_key_value_pair_new_full(title, str, g_free);
		list = g_list_append(list, pair);
	}
	return list;
}

static void
get_credential_provider_options_helper(PurpleCredentialProvider *provider,
                                       gpointer data)
{
	GList **list = data;
	PurpleKeyValuePair *pair = NULL;
	const char *name, *id;

	name = purple_credential_provider_get_name(provider);
	id = purple_credential_provider_get_id(provider);
	pair = purple_key_value_pair_new(name, (gpointer)id);
	*list = g_list_append(*list, pair);
}

static GList *
get_credential_provider_options(void) {
	PurpleCredentialManager *manager = NULL;
	GList *list = NULL;

	manager = purple_credential_manager_get_default();
	purple_credential_manager_foreach(manager,
	                                  get_credential_provider_options_helper,
	                                  &list);

	return list;
}

static PurpleRequestField *
get_pref_field(Prefs *prefs) {
	PurpleRequestField *field = NULL;

	if(prefs->choices == NULL) {
		switch(prefs->type) {
			case PURPLE_PREF_BOOLEAN:
				field = purple_request_field_bool_new(prefs->pref,
				                                      _(prefs->label),
				                                      purple_prefs_get_bool(prefs->pref));
				break;
			case PURPLE_PREF_INT:
				field = purple_request_field_int_new(prefs->pref,
				                                     _(prefs->label),
				                                     purple_prefs_get_int(prefs->pref),
				                                     INT_MIN, INT_MAX);
				break;
			case PURPLE_PREF_STRING:
				field = purple_request_field_string_new(prefs->pref,
				                                        _(prefs->label),
				                                        purple_prefs_get_string(prefs->pref),
				                                        FALSE);
				break;
			default:
				break;
		}
	} else {
		PurpleRequestFieldChoice *choice = NULL;
		GList *list = NULL;

		list = prefs->choices();
		if(list != NULL) {
			field = purple_request_field_choice_new(prefs->pref,
			                                        _(prefs->label), NULL);
			choice = PURPLE_REQUEST_FIELD_CHOICE(field);
		}

		for(GList *iter = list; iter; iter = iter->next) {
			PurpleKeyValuePair *pair = iter->data;
			gboolean select = FALSE;
			int idata;

			switch(prefs->type) {
				case PURPLE_PREF_BOOLEAN:
					if(sscanf(pair->value, "%d", &idata) != 1) {
						idata = FALSE;
					}
					if(purple_prefs_get_bool(prefs->pref) == idata) {
						select = TRUE;
					}
					break;
				case PURPLE_PREF_INT:
					if(sscanf(pair->value, "%d", &idata) != 1) {
						idata = 0;
					}
					if(purple_prefs_get_int(prefs->pref) == idata) {
						select = TRUE;
					}
					break;
				case PURPLE_PREF_STRING:
					if(purple_strequal(purple_prefs_get_string(prefs->pref),
					                   pair->value))
					{
						select = TRUE;
					}
					break;
				default:
					break;
			}

			purple_request_field_choice_add(choice, pair->key, pair->value);
			if(select) {
				purple_request_field_choice_set_default_value(choice,
				                                              pair->value);
				purple_request_field_choice_set_value(choice, pair->value);
			}
		}

		pref_request.list_data = g_list_concat(pref_request.list_data, list);
	}
	return field;
}

static Prefs blist[] = {
	{
		.type = PURPLE_PREF_BOOLEAN,
		.pref = "/finch/blist/idletime",
		.label = N_("Show Idle Time"),
	}, {
		.type = PURPLE_PREF_BOOLEAN,
		.pref = "/finch/blist/showoffline",
		.label = N_("Show Offline Buddies"),
	},
};

static Prefs convs[] = {
	{
		.type = PURPLE_PREF_BOOLEAN,
		.pref = "/finch/conversations/timestamps",
		.label = N_("Show Timestamps"),
	}, {
		.type = PURPLE_PREF_BOOLEAN,
		.pref = "/finch/conversations/notify_typing",
		.label = N_("Notify buddies when you are typing"),
	},
};

static Prefs idle[] = {
	{
		.type = PURPLE_PREF_STRING,
		.pref = "/purple/away/idle_reporting",
		.label = N_("Report Idle time"),
		.choices = get_idle_options,
	}, {
		.type = PURPLE_PREF_BOOLEAN,
		.pref = "/purple/away/away_when_idle",
		.label = N_("Change status when idle"),
	}, {
		.type = PURPLE_PREF_INT,
		.pref = "/purple/away/mins_before_away",
		.label = N_("Minutes before changing status"),
	}, {
		.type = PURPLE_PREF_INT,
		.pref = "/purple/savedstatus/idleaway",
		.label = N_("Change status to"),
		.choices = get_status_titles,
	},
};

static Prefs credentials[] = {
	{
		.type = PURPLE_PREF_STRING,
		.pref = "/purple/credentials/active-provider",
		.label = N_("Provider"),
		.choices = get_credential_provider_options,
	},
};

static void
free_strings(void)
{
	g_clear_list(&pref_request.list_data,
	             (GDestroyNotify)purple_key_value_pair_free);
	pref_request.showing = FALSE;
}

static void
save_cb(void *data, PurpleRequestPage *page) {
	finch_request_save_in_prefs(data, page);
	free_strings();
}

static void
add_pref_group(PurpleRequestPage *page, const char *title, Prefs *prefs,
               guint n_prefs)
{
	PurpleRequestGroup *group = NULL;

	group = purple_request_group_new(title);
	purple_request_page_add_group(page, group);

	for(guint i = 0; i < n_prefs; i++) {
		PurpleRequestField *field = get_pref_field(&prefs[i]);

		if(PURPLE_IS_REQUEST_FIELD(field)) {
			purple_request_group_add_field(group, field);
		}
	}
}

void
finch_prefs_show_all(void)
{
	PurpleRequestPage *page;

	if (pref_request.showing) {
		gnt_window_present(pref_request.window);
		return;
	}

	page = purple_request_page_new();

	add_pref_group(page, _("Buddy List"), blist, G_N_ELEMENTS(blist));
	add_pref_group(page, _("Conversations"), convs, G_N_ELEMENTS(convs));
	add_pref_group(page, _("Idle"), idle, G_N_ELEMENTS(idle));
	add_pref_group(page, _("Credentials"), credentials,
	               G_N_ELEMENTS(credentials));

	pref_request.showing = TRUE;
	pref_request.window = purple_request_fields(NULL, _("Preferences"), NULL, NULL, page,
			_("Save"), G_CALLBACK(save_cb), _("Cancel"), free_strings,
			NULL, NULL);
}
