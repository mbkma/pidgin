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

#include <glib/gi18n.h>

#include <purple.h>

#include "pidginaccountmanager.h"

#include "pidgincore.h"
#include "pidginaccounteditor.h"
#include "pidginaccountrow.h"

struct _PidginAccountManager {
	GtkDialog parent;

	GtkListBox *list_box;
	GtkWidget *add;

	GtkWidget *stack;
	GtkWidget *editor;

	/* This is used to not go back to the manager when an account was edited
	 * directly, via the `accounts->(account)->edit` menu or the
	 * `connection error` notification.
	 */
	gboolean edit_only;
};

enum {
	RESPONSE_ADD,
};

G_DEFINE_TYPE(PidginAccountManager, pidgin_account_manager, GTK_TYPE_DIALOG)

/******************************************************************************
 * Helpers
 *****************************************************************************/
static GtkWidget *
pidgin_account_manager_create_widget(gpointer item,
                                     G_GNUC_UNUSED gpointer data)
{
	if(!PURPLE_IS_ACCOUNT(item)) {
		return NULL;
	}

	return pidgin_account_row_new(PURPLE_ACCOUNT(item));
}

static void
pidgin_account_manager_real_edit_account(PidginAccountManager *manager,
                                         PurpleAccount *account,
                                         gboolean edit_only)
{
	g_return_if_fail(PIDGIN_IS_ACCOUNT_MANAGER(manager));

	manager->edit_only = edit_only;

	pidgin_account_editor_set_account(PIDGIN_ACCOUNT_EDITOR(manager->editor),
	                                  account);

	gtk_stack_set_visible_child_name(GTK_STACK(manager->stack), "editor-page");
}

/******************************************************************************
 * Callbacks
 *****************************************************************************/
/* This is used by the add button on the placeholder page. */
static void
pidgin_account_manager_create_account(G_GNUC_UNUSED GtkButton *self,
                                      gpointer data)
{
	PidginAccountManager *manager = data;

	pidgin_account_manager_real_edit_account(manager, NULL, FALSE);
}

static void
pidgin_account_manager_refresh_add_cb(GListModel *list,
                                      G_GNUC_UNUSED guint position,
                                      G_GNUC_UNUSED guint removed,
                                      G_GNUC_UNUSED guint added,
                                      gpointer data)
{
	PidginAccountManager *manager = data;

	/* If there are no accounts, the placeholder is shown, which includes an
	 * Add button. So hide the one in the button box if that's the case. */
	gtk_widget_set_visible(manager->add, g_list_model_get_n_items(list) != 0);
}

static void
pidgin_account_manager_response_cb(GtkDialog *dialog, gint response_id,
                                   G_GNUC_UNUSED gpointer data)
{
	PidginAccountManager *manager = PIDGIN_ACCOUNT_MANAGER(dialog);

	switch(response_id) {
		case RESPONSE_ADD:
			pidgin_account_manager_real_edit_account(manager, NULL, FALSE);
			break;
		case GTK_RESPONSE_CLOSE:
		case GTK_RESPONSE_DELETE_EVENT:
			gtk_window_destroy(GTK_WINDOW(dialog));
			break;
		default:
			g_warning("not sure how you got here...");
	}
}

static void
pidgin_account_manager_row_activated_cb(G_GNUC_UNUSED GtkListBox *box,
                                        GtkListBoxRow *row,
                                        gpointer data)
{
	PurpleAccount *account = NULL;
	PidginAccountManager *manager = data;

	account = pidgin_account_row_get_account(PIDGIN_ACCOUNT_ROW(row));

	pidgin_account_manager_real_edit_account(manager, account, FALSE);
}

static void
pidgin_account_manager_back_clicked_cb(G_GNUC_UNUSED GtkButton *self,
                                       gpointer data)
{
	PidginAccountManager *manager = data;

#if ADW_CHECK_VERSION(1, 3, 0)
	/* Scroll the editor back to the top of the scrolled window. */
	adw_preferences_page_scroll_to_top(ADW_PREFERENCES_PAGE(manager->editor));
#endif

	pidgin_account_manager_show_overview(manager);
}

static void
pidgin_account_manager_save_clicked_cb(G_GNUC_UNUSED GtkButton *self,
                                       gpointer data)
{
	PidginAccountManager *manager = data;

	pidgin_account_editor_save(PIDGIN_ACCOUNT_EDITOR(manager->editor));

	if(manager->edit_only) {
		gtk_window_destroy(GTK_WINDOW(manager));
	} else {
		pidgin_account_manager_show_overview(manager);
	}
}

/******************************************************************************
 * GObject Implementation
 *****************************************************************************/
static void
pidgin_account_manager_init(PidginAccountManager *manager) {
	GListModel *purple_manager = NULL;

	gtk_widget_init_template(GTK_WIDGET(manager));

	purple_manager = purple_account_manager_get_default_as_model();
	gtk_list_box_bind_model(manager->list_box, purple_manager,
	                        pidgin_account_manager_create_widget, NULL, NULL);
	g_signal_connect_object(purple_manager, "items-changed",
	                        G_CALLBACK(pidgin_account_manager_refresh_add_cb),
	                        manager, 0);
	pidgin_account_manager_refresh_add_cb(purple_manager, 0, 0, 0, manager);
}

static void
pidgin_account_manager_class_init(PidginAccountManagerClass *klass) {
	GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

	gtk_widget_class_set_template_from_resource(
		widget_class,
		"/im/pidgin/Pidgin3/Accounts/manager.ui"
	);

	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     list_box);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     add);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     stack);
	gtk_widget_class_bind_template_child(widget_class, PidginAccountManager,
	                                     editor);

	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_response_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_row_activated_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_create_account);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_back_clicked_cb);
	gtk_widget_class_bind_template_callback(widget_class,
	                                        pidgin_account_manager_save_clicked_cb);
}

/******************************************************************************
 * Public API
 *****************************************************************************/
GtkWidget *
pidgin_account_manager_new(void) {
	return g_object_new(PIDGIN_TYPE_ACCOUNT_MANAGER, NULL);
}

void
pidgin_account_manager_show_overview(PidginAccountManager *manager) {
	g_return_if_fail(PIDGIN_IS_ACCOUNT_MANAGER(manager));

	gtk_stack_set_visible_child_name(GTK_STACK(manager->stack), "overview");

	pidgin_account_editor_set_account(PIDGIN_ACCOUNT_EDITOR(manager->editor),
	                                  NULL);
}

void
pidgin_account_manager_edit_account(PidginAccountManager *manager,
                                    PurpleAccount *account)
{
	g_return_if_fail(PIDGIN_IS_ACCOUNT_MANAGER(manager));
	g_return_if_fail(PURPLE_IS_ACCOUNT(account));

	pidgin_account_manager_real_edit_account(manager, account, TRUE);
}
