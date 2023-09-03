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

#ifndef PURPLE_REQUEST_H
#define PURPLE_REQUEST_H

#include <stdlib.h>

#include <glib.h>
#include <glib-object.h>

#define PURPLE_TYPE_REQUEST_UI_OPS (purple_request_ui_ops_get_type())

/**
 * PurpleRequestCommonParameters:
 *
 * Common parameters for UI operations.
 */
typedef struct _PurpleRequestCommonParameters PurpleRequestCommonParameters;

typedef struct _PurpleRequestUiOps PurpleRequestUiOps;

#include "account.h"
#include "purpleconversation.h"
#include "purplerequestpage.h"
#include "purplerequestgroup.h"
#include "purplerequestfield.h"
#include "request-datasheet.h"

#define PURPLE_DEFAULT_ACTION_NONE	-1

/**
 * PurpleRequestType:
 * @PURPLE_REQUEST_INPUT:  Text input request.
 * @PURPLE_REQUEST_CHOICE: Multiple-choice request.
 * @PURPLE_REQUEST_ACTION: Action request.
 * @PURPLE_REQUEST_WAIT:   Please wait dialog.
 * @PURPLE_REQUEST_FIELDS: Multiple fields request.
 * @PURPLE_REQUEST_FILE:   File open or save request.
 * @PURPLE_REQUEST_FOLDER: Folder selection request.
 *
 * Request types.
 */
typedef enum
{
	PURPLE_REQUEST_INPUT = 0,
	PURPLE_REQUEST_CHOICE,
	PURPLE_REQUEST_ACTION,
	PURPLE_REQUEST_WAIT,
	PURPLE_REQUEST_FIELDS,
	PURPLE_REQUEST_FILE,
	PURPLE_REQUEST_FOLDER

} PurpleRequestType;

/**
 * PurpleRequestFeature:
 * @PURPLE_REQUEST_FEATURE_HTML: Specifies that HTML should be supported.
 *
 * Feature flags for the request api.
 */
typedef enum
{
	PURPLE_REQUEST_FEATURE_HTML = 0x00000001
} PurpleRequestFeature;

/**
 * PurpleRequestIconType:
 * @PURPLE_REQUEST_ICON_DEFAULT: The default icon.
 * @PURPLE_REQUEST_ICON_REQUEST: Use a question icon.
 * @PURPLE_REQUEST_ICON_DIALOG: Use a dialog icon.
 * @PURPLE_REQUEST_ICON_WAIT: Use a wait icon.
 * @PURPLE_REQUEST_ICON_INFO: Use an info icon.
 * @PURPLE_REQUEST_ICON_WARNING: Use a warning icon.
 * @PURPLE_REQUEST_ICON_ERROR: Use an error icon.
 *
 * Constants to define which kind of icon should be displayed.
 */
typedef enum
{
	PURPLE_REQUEST_ICON_DEFAULT = 0,
	PURPLE_REQUEST_ICON_REQUEST,
	PURPLE_REQUEST_ICON_DIALOG,
	PURPLE_REQUEST_ICON_WAIT,
	PURPLE_REQUEST_ICON_INFO,
	PURPLE_REQUEST_ICON_WARNING,
	PURPLE_REQUEST_ICON_ERROR
} PurpleRequestIconType;

/**
 * PurpleRequestCancelCb:
 * @data: user-data.
 *
 * A callback that's used to handle cancel actions.
 */
typedef void (*PurpleRequestCancelCb)(gpointer data);

/**
 * PurpleRequestUiOps:
 * @features:            A bitwise or of #PurpleRequestFeature's.
 * @request_input:       See purple_request_input().
 * @request_choice:      See purple_request_choice_varg().
 * @request_action:      See purple_request_action_varg().
 * @request_wait:        See purple_request_wait().
 * @request_wait_update: See purple_request_wait_pulse(),
 *                       purple_request_wait_progress().
 * @request_fields:      See purple_request_fields().
 * @request_file:        See purple_request_file().
 * @request_folder:      See purple_request_folder().
 * @close_request:       See purple_request_close().
 *
 * Request UI operations.
 */
struct _PurpleRequestUiOps
{
	PurpleRequestFeature features;

	void *(*request_input)(const char *title, const char *primary,
		const char *secondary, const char *default_value,
		gboolean multiline, gboolean masked, gchar *hint,
		const char *ok_text, GCallback ok_cb,
		const char *cancel_text, GCallback cancel_cb,
		PurpleRequestCommonParameters *cpar, void *user_data);

	void *(*request_choice)(const char *title, const char *primary,
		const char *secondary, gpointer default_value,
		const char *ok_text, GCallback ok_cb, const char *cancel_text,
		GCallback cancel_cb, PurpleRequestCommonParameters *cpar,
		void *user_data, va_list choices);

	void *(*request_action)(const char *title, const char *primary,
		const char *secondary, int default_action,
		PurpleRequestCommonParameters *cpar, void *user_data,
		size_t action_count, va_list actions);

	void *(*request_wait)(const char *title, const char *primary,
		const char *secondary, gboolean with_progress,
		PurpleRequestCancelCb cancel_cb,
		PurpleRequestCommonParameters *cpar, void *user_data);

	void (*request_wait_update)(void *ui_handle, gboolean animate,
		gfloat fraction);

	void *(*request_fields)(const char *title, const char *primary,
		const char *secondary, PurpleRequestPage *page,
		const char *ok_text, GCallback ok_cb,
		const char *cancel_text, GCallback cancel_cb,
		PurpleRequestCommonParameters *cpar, void *user_data);

	void *(*request_file)(const char *title, const char *filename,
		gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
		PurpleRequestCommonParameters *cpar, void *user_data);

	void *(*request_folder)(const char *title, const char *dirname,
		GCallback ok_cb, GCallback cancel_cb,
		PurpleRequestCommonParameters *cpar, void *user_data);

	void (*close_request)(PurpleRequestType type, void *ui_handle);

	/*< private >*/
	void (*_purple_reserved1)(void);
	void (*_purple_reserved2)(void);
	void (*_purple_reserved3)(void);
	void (*_purple_reserved4)(void);
};

typedef void (*PurpleRequestInputCb)(void *data, const char *value);

/**
 * PurpleRequestActionCb:
 * @data:   user-data
 * @action: The action that was chosen.
 *
 * The type of callbacks passed to purple_request_action().  The first
 * argument is the <literal>user_data</literal> parameter; the second is the
 * index in the list of actions of the one chosen.
 */
typedef void (*PurpleRequestActionCb)(void *data, int action);

/**
 * PurpleRequestChoiceCb:
 * @data:  user-data
 * @value: The choice that was made.
 *
 * The type of callbacks passed to purple_request_choice().  The first
 * argument is the <literal>user_data</literal> parameter; the second is the
 * values of those choice.
 */
typedef void (*PurpleRequestChoiceCb)(void *data, gpointer value);
typedef void (*PurpleRequestFieldsCb)(void *data, PurpleRequestPage *page);
typedef void (*PurpleRequestFileCb)(void *data, const char *filename);
typedef void (*PurpleRequestHelpCb)(gpointer data);

G_BEGIN_DECLS

/**************************************************************************/
/* Common parameters API                                                  */
/**************************************************************************/

/**
 * purple_request_cpar_new:
 *
 * Creates new parameters set for the request, which may or may not be used by
 * the UI to display the request.
 *
 * Returns: (transfer full): The new parameters set.
 */
PurpleRequestCommonParameters *
purple_request_cpar_new(void);

/**
 * purple_request_cpar_from_connection:
 * @gc: The #PurpleConnection.
 *
 * Creates new parameters set initially bound with the #PurpleConnection.
 *
 * Returns: (transfer full): The new parameters set.
 */
PurpleRequestCommonParameters *
purple_request_cpar_from_connection(PurpleConnection *gc);

/**
 * purple_request_cpar_from_account:
 * @account: The #PurpleAccount.
 *
 * Creates new parameters set initially bound with the #PurpleAccount.
 *
 * Returns: (transfer full): The new parameters set.
 */
PurpleRequestCommonParameters *
purple_request_cpar_from_account(PurpleAccount *account);

/**
 * purple_request_cpar_from_conversation:
 * @conv: The #PurpleConversation.
 *
 * Creates new parameters set initially bound with the #PurpleConversation.
 *
 * Returns: (transfer full): The new parameters set.
 */
PurpleRequestCommonParameters *
purple_request_cpar_from_conversation(PurpleConversation *conv);

/**
 * purple_request_cpar_ref:
 * @cpar: The object to ref.
 *
 * Increases the reference count on the parameters set.
 */
void
purple_request_cpar_ref(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_unref:
 * @cpar: The parameters set object to unref and possibly destroy.
 *
 * Decreases the reference count on the parameters set.
 *
 * The object will be destroyed when this reaches 0.
 */
void
purple_request_cpar_unref(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_account:
 * @cpar:    The parameters set.
 * @account: The #PurpleAccount to associate.
 *
 * Sets the #PurpleAccount associated with the request, or %NULL, if none is.
 */
void
purple_request_cpar_set_account(PurpleRequestCommonParameters *cpar,
	PurpleAccount *account);

/**
 * purple_request_cpar_get_account:
 * @cpar: The parameters set (may be %NULL).
 *
 * Gets the #PurpleAccount associated with the request.
 *
 * Returns: (transfer none): The associated #PurpleAccount, or %NULL if none is
 *          set.
 */
PurpleAccount *
purple_request_cpar_get_account(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_conversation:
 * @cpar: The parameters set.
 * @conv: The #PurpleConversation to associate.
 *
 * Sets the #PurpleConversation associated with the request, or %NULL, if
 * none is.
 */
void
purple_request_cpar_set_conversation(PurpleRequestCommonParameters *cpar,
	PurpleConversation *conv);

/**
 * purple_request_cpar_get_conversation:
 * @cpar: The parameters set (may be %NULL).
 *
 * Gets the #PurpleConversation associated with the request.
 *
 * Returns: (transfer none): The associated #PurpleConversation, or %NULL if
 *          none is set.
 */
PurpleConversation *
purple_request_cpar_get_conversation(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_icon:
 * @cpar:      The parameters set.
 * @icon_type: The icon type.
 *
 * Sets the icon associated with the request.
 */
void
purple_request_cpar_set_icon(PurpleRequestCommonParameters *cpar,
	PurpleRequestIconType icon_type);

/**
 * purple_request_cpar_get_icon:
 * @cpar: The parameters set.
 *
 * Gets the icon associated with the request.
 *
 * Returns: icon_type The icon type.
 */
PurpleRequestIconType
purple_request_cpar_get_icon(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_custom_icon:
 * @cpar:      The parameters set.
 * @icon_data: The icon image contents (%NULL to reset).
 * @icon_size: The icon image size.
 *
 * Sets the custom icon associated with the request.
 */
void
purple_request_cpar_set_custom_icon(PurpleRequestCommonParameters *cpar,
	gconstpointer icon_data, gsize icon_size);

/**
 * purple_request_cpar_get_custom_icon:
 * @cpar:      The parameters set (may be %NULL).
 * @icon_size: The pointer to variable, where icon size should be stored
 *                  (may be %NULL).
 *
 * Gets the custom icon associated with the request.
 *
 * Returns: The icon image contents.
 */
gconstpointer
purple_request_cpar_get_custom_icon(PurpleRequestCommonParameters *cpar,
	gsize *icon_size);

/**
 * purple_request_cpar_set_html:
 * @cpar:    The parameters set.
 * @enabled: 1, if the text passed with the request contains HTML,
 *                0 otherwise. Don't use any other values, as they may be
 *                redefined in the future.
 *
 * Switches the request text to be HTML or not.
 */
void
purple_request_cpar_set_html(PurpleRequestCommonParameters *cpar,
	gboolean enabled);

/**
 * purple_request_cpar_is_html:
 * @cpar: The parameters set (may be %NULL).
 *
 * Checks, if the text passed to the request is HTML.
 *
 * Returns: %TRUE, if the text is HTML, %FALSE otherwise.
 */
gboolean
purple_request_cpar_is_html(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_compact:
 * @cpar:    The parameters set.
 * @compact: TRUE for compact, FALSE otherwise.
 *
 * Sets dialog display mode to compact or default.
 */
void
purple_request_cpar_set_compact(PurpleRequestCommonParameters *cpar,
	gboolean compact);

/**
 * purple_request_cpar_is_compact:
 * @cpar: The parameters set (may be %NULL).
 *
 * Gets dialog display mode.
 *
 * Returns: TRUE for compact, FALSE for default.
 */
gboolean
purple_request_cpar_is_compact(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_help_cb:
 * @cpar:      The parameters set.
 * @cb: (scope notified):       The callback.
 * @user_data: The data to be passed to the callback.
 *
 * Sets the callback for the Help button.
 */
void
purple_request_cpar_set_help_cb(PurpleRequestCommonParameters *cpar,
	PurpleRequestHelpCb cb, gpointer user_data);

/**
 * purple_request_cpar_get_help_cb:
 * @cpar:      The parameters set (may be %NULL).
 * @user_data: The pointer to the variable, where user data (to be passed to
 *             callback function) should be stored.
 *
 * Gets the callback for the Help button.
 *
 * Returns: (transfer none): The callback.
 */
PurpleRequestHelpCb
purple_request_cpar_get_help_cb(PurpleRequestCommonParameters *cpar,
	gpointer *user_data);

/**
 * purple_request_cpar_set_extra_actions:
 * @cpar: The parameters set.
 * @...:  A list of actions. These are pairs of arguments. The first of each
 *        pair is the <type>char *</type> label that appears on the button.  It
 *        should have an underscore before the letter you want to use as the
 *        accelerator key for the button. The second of each pair is the
 *        #PurpleRequestFieldsCb function to use when the button is clicked.
 *        Should be terminated with the NULL label.
 *
 * Sets extra actions for the PurpleRequestPage dialog.
 */
void
purple_request_cpar_set_extra_actions(PurpleRequestCommonParameters *cpar, ...);

/**
 * purple_request_cpar_get_extra_actions:
 * @cpar: The parameters set (may be %NULL).
 *
 * Gets extra actions for the PurpleRequestPage dialog.
 *
 * Returns: (element-type PurpleKeyValuePair) (transfer none): A list of actions (pairs of arguments, as in
 *          setter).
 */
GSList *
purple_request_cpar_get_extra_actions(PurpleRequestCommonParameters *cpar);

/**
 * purple_request_cpar_set_parent_from:
 * @cpar:      The parameters set.
 * @ui_handle: The UI handle.
 *
 * Sets the same parent window for this dialog, as the parent of specified
 * Notify API or Request API dialog UI handle.
 */
void
purple_request_cpar_set_parent_from(PurpleRequestCommonParameters *cpar,
	gpointer ui_handle);

/**
 * purple_request_cpar_get_parent_from:
 * @cpar: The parameters set (may be %NULL).
 *
 * Gets the parent "donor" for this dialog.
 *
 * Returns: The donors UI handle.
 */
gpointer
purple_request_cpar_get_parent_from(PurpleRequestCommonParameters *cpar);

/**************************************************************************/
/* Request API                                                            */
/**************************************************************************/

/**
 * purple_request_input:
 * @handle:        The plugin or connection handle.  For some
 *                 things this is <emphasis>extremely</emphasis> important.  The
 *                 handle is used to programmatically close the request
 *                 dialog when it is no longer needed.  For protocols this
 *                 is often a pointer to the #PurpleConnection
 *                 instance.  For plugins this should be a similar,
 *                 unique memory location.  This value is important
 *                 because it allows a request to be closed with
 *                 purple_request_close_with_handle() when, for
 *                 example, you sign offline.  If the request is
 *                 <emphasis>not</emphasis> closed it is
 *                 <emphasis>very</emphasis> likely to cause a crash whenever
 *                 the callback handler functions are triggered.
 * @title:         The title of the message, or %NULL if it should have
 *                 no title.
 * @primary:       The main point of the message, or %NULL if you're
 *                 feeling enigmatic.
 * @secondary:     Secondary information, or %NULL if there is none.
 * @default_value: The default value.
 * @multiline:     %TRUE if the inputted text can span multiple lines.
 * @masked:        %TRUE if the inputted text should be masked in some
 *                 way (such as by displaying characters as stars).  This
 *                 might be because the input is some kind of password.
 * @hint:          Optionally suggest how the input box should appear.
 *                 Use "html", for example, to allow the user to enter HTML.
 * @ok_text:       The text for the <literal>OK</literal> button, which may not
 *                 be %NULL.
 * @ok_cb: (scope notified):         The callback for the <literal>OK</literal> button, which may
 *                 not be %NULL.
 * @cancel_text:   The text for the <literal>Cancel</literal> button, which may
 *                 not be %NULL.
 * @cancel_cb: (scope notified):     The callback for the <literal>Cancel</literal> button, which
 *                 may be %NULL.
 * @cpar:          The #PurpleRequestCommonParameters object, which gets
 *                 unref'ed after this call.
 * @user_data:     The data to pass to the callback.
 *
 * Prompts the user for text input.
 *
 * Returns: A UI-specific handle.
 */
void *purple_request_input(void *handle, const char *title, const char *primary,
	const char *secondary, const char *default_value, gboolean multiline,
	gboolean masked, gchar *hint,
	const char *ok_text, GCallback ok_cb,
	const char *cancel_text, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar,
	void *user_data);

/**
 * purple_request_choice:
 * @handle:        The plugin or connection handle.  For some things this
 *                 is <emphasis>extremely</emphasis> important.  See the comments on
 *                 purple_request_input().
 * @title:         The title of the message, or %NULL if it should have
 *                 no title.
 * @primary:       The main point of the message, or %NULL if you're
 *                 feeling enigmatic.
 * @secondary:     Secondary information, or %NULL if there is none.
 * @default_value: The default choice; this should be one of the values
 *                 listed in the varargs.
 * @ok_text:       The text for the <literal>OK</literal> button, which may not
 *                 be %NULL.
 * @ok_cb: (scope notified):         The callback for the <literal>OK</literal> button, which may
 *                 not be %NULL.
 * @cancel_text:   The text for the <literal>Cancel</literal> button, which may
 *                 not be %NULL.
 * @cancel_cb: (scope notified):     The callback for the <literal>Cancel</literal> button, or
 *                 %NULL to do nothing.
 * @cpar:          The #PurpleRequestCommonParameters object, which gets
 *                 unref'ed after this call.
 * @user_data:     The data to pass to the callback.
 * @...:           The choices, which should be pairs of <type>char *</type>
 *                 descriptions and <type>int</type> values, terminated with a
 *                 %NULL parameter.
 *
 * Prompts the user for multiple-choice input.
 *
 * Returns: A UI-specific handle.
 */
void *purple_request_choice(void *handle, const char *title, const char *primary,
	const char *secondary, gpointer default_value,
	const char *ok_text, GCallback ok_cb,
	const char *cancel_text, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar,
	void *user_data, ...) G_GNUC_NULL_TERMINATED;

/**
 * purple_request_choice_varg:
 * @handle:        The plugin or connection handle.  For some things this
 *                 is <emphasis>extremely</emphasis> important.  See the comments on
 *                 purple_request_input().
 * @title:         The title of the message, or %NULL if it should have
 *                 no title.
 * @primary:       The main point of the message, or %NULL if you're
 *                 feeling enigmatic.
 * @secondary:     Secondary information, or %NULL if there is none.
 * @default_value: The default choice; this should be one of the values
 *                 listed in the varargs.
 * @ok_text:       The text for the <literal>OK</literal> button, which may not
 *                 be %NULL.
 * @ok_cb: (scope notified):         The callback for the <literal>OK</literal> button, which may
 *                 not be %NULL.
 * @cancel_text:   The text for the <literal>Cancel</literal> button, which may
 *                 not be %NULL.
 * @cancel_cb: (scope notified):     The callback for the <literal>Cancel</literal> button, or
 *                 %NULL to do nothing.
 * @cpar:          The #PurpleRequestCommonParameters object, which gets
 *                 unref'ed after this call.
 * @user_data:     The data to pass to the callback.
 * @choices:       The choices, which should be pairs of <type>char *</type>
 *                 descriptions and <type>int</type> values, terminated with a
 *                 %NULL parameter.
 *
 * <literal>va_list</literal> version of purple_request_choice(); see its
 * documentation.
 */
void *purple_request_choice_varg(void *handle, const char *title,
	const char *primary, const char *secondary, gpointer default_value,
	const char *ok_text, GCallback ok_cb,
	const char *cancel_text, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar,
	void *user_data, va_list choices);

/**
 * purple_request_action:
 * @handle:         The plugin or connection handle.  For some things this
 *                  is <emphasis>extremely</emphasis> important.  See the comments on
 *                  purple_request_input().
 * @title:          The title of the message, or %NULL if it should have
 *                  no title.
 * @primary:        The main point of the message, or %NULL if you're
 *                  feeling enigmatic.
 * @secondary:      Secondary information, or %NULL if there is none.
 * @default_action: The default action, zero-indexed; if the third action
 *                  supplied should be the default, supply
 *                  <literal>2</literal>.  This should be the action that
 *                  users are most likely to select.
 * @cpar:           The #PurpleRequestCommonParameters object, which gets
 *                  unref'ed after this call.
 * @user_data:      The data to pass to the callback.
 * @action_count:   The number of actions.
 * @...:            A list of actions.  These are pairs of
 *                  arguments.  The first of each pair is the
 *                  <type>char *</type> label that appears on the button.
 *                  It should have an underscore before the letter you want
 *                  to use as the accelerator key for the button.  The
 *                  second of each pair is the #PurpleRequestActionCb
 *                  function to use when the button is clicked.
 *
 * Prompts the user for an action.
 *
 * This is often represented as a dialog with a button for each action.
 *
 * Returns: A UI-specific handle.
 */
void *
purple_request_action(void *handle, const char *title, const char *primary,
	const char *secondary, int default_action,
	PurpleRequestCommonParameters *cpar, void *user_data,
	size_t action_count, ...);

/**
 * purple_request_action_varg:
 * @handle:         The plugin or connection handle.  For some things this
 *                  is <emphasis>extremely</emphasis> important.  See the comments on
 *                  purple_request_input().
 * @title:          The title of the message, or %NULL if it should have
 *                  no title.
 * @primary:        The main point of the message, or %NULL if you're
 *                  feeling enigmatic.
 * @secondary:      Secondary information, or %NULL if there is none.
 * @default_action: The default action, zero-indexed; if the third action
 *                  supplied should be the default, supply
 *                  <literal>2</literal>.  This should be the action that
 *                  users are most likely to select.
 * @cpar:           The #PurpleRequestCommonParameters object, which gets
 *                  unref'ed after this call.
 * @user_data:      The data to pass to the callback.
 * @action_count:   The number of actions.
 * @actions:        A list of actions.  These are pairs of
 *                  arguments.  The first of each pair is the
 *                  <type>char *</type> label that appears on the button.
 *                  It should have an underscore before the letter you want
 *                  to use as the accelerator key for the button.  The
 *                  second of each pair is the #PurpleRequestActionCb
 *                  function to use when the button is clicked.
 *
 * <literal>va_list</literal> version of purple_request_action(); see its
 * documentation.
 */
void *
purple_request_action_varg(void *handle, const char *title, const char *primary,
	const char *secondary, int default_action,
	PurpleRequestCommonParameters *cpar, void *user_data,
	size_t action_count, va_list actions);

/**
 * purple_request_wait:
 * @handle:        The plugin or connection handle.  For some things this
 *                 is <emphasis>extremely</emphasis> important.  See the comments on
 *                 purple_request_input().
 * @title:         The title of the message, or %NULL if it should have
 *                 default title.
 * @primary:       The main point of the message, or %NULL if you're
 *                 feeling enigmatic.
 * @secondary:     Secondary information, or %NULL if there is none.
 * @with_progress: %TRUE, if we want to display progress bar, %FALSE
 *                 otherwise
 * @cancel_cb: (scope notified): The callback for the <literal>Cancel</literal> button, which
 *                 may be %NULL.
 * @cpar:          The #PurpleRequestCommonParameters object, which gets
 *                 unref'ed after this call.
 * @user_data:     The data to pass to the callback.
 *
 * Displays a "please wait" dialog.
 *
 * Returns: A UI-specific handle.
 */
void *
purple_request_wait(void *handle, const char *title, const char *primary,
	const char *secondary, gboolean with_progress,
	PurpleRequestCancelCb cancel_cb, PurpleRequestCommonParameters *cpar,
	void *user_data);

/**
 * purple_request_wait_pulse:
 * @ui_handle: The request UI handle.
 *
 * Notifies the "please wait" dialog that some progress has been made, but you
 * don't know how much.
 */
void
purple_request_wait_pulse(void *ui_handle);

/**
 * purple_request_wait_progress:
 * @ui_handle: The request UI handle.
 * @fraction:  The part of task that is done (between 0.0 and 1.0,
 *                  inclusive).
 *
 * Notifies the "please wait" dialog about progress has been made.
 */
void
purple_request_wait_progress(void *ui_handle, gfloat fraction);

/**
 * purple_request_fields:
 * @handle:      The plugin or connection handle.  For some things this
 *               is <emphasis>extremely</emphasis> important.  See the comments on
 *               purple_request_input().
 * @title:       The title of the message, or %NULL if it should have
 *               no title.
 * @primary:     The main point of the message, or %NULL if you're
 *               feeling enigmatic.
 * @secondary:   Secondary information, or %NULL if there is none.
 * @page:        The page of fields.
 * @ok_text:     The text for the <literal>OK</literal> button, which may not be
 *               %NULL.
 * @ok_cb: (scope notified):       The callback for the <literal>OK</literal> button, which may
 *               not be
 *               %NULL.
 * @cancel_text: The text for the <literal>Cancel</literal> button, which may
 *               not be %NULL.
 * @cancel_cb: (scope notified):   The callback for the <literal>Cancel</literal> button, which
 *               may be %NULL.
 * @cpar:        The #PurpleRequestCommonParameters object, which gets
 *               unref'ed after this call.
 * @user_data:   The data to pass to the callback.
 *
 * Displays groups of fields for the user to fill in.
 *
 * Returns: A UI-specific handle.
 */
void *
purple_request_fields(void *handle, const char *title, const char *primary,
	const char *secondary, PurpleRequestPage *page,
	const char *ok_text, GCallback ok_cb,
	const char *cancel_text, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar,
	void *user_data);

/**
 * purple_request_is_valid_ui_handle:
 * @ui_handle: The UI handle.
 * @type:      The pointer to variable, where request type may be stored
 *             (may be %NULL).
 *
 * Checks, if passed UI handle is valid.
 *
 * Returns: TRUE, if handle is valid, FALSE otherwise.
 */
gboolean
purple_request_is_valid_ui_handle(void *ui_handle, PurpleRequestType *type);

/**
 * purple_request_add_close_notify:
 * @ui_handle:   The UI handle.
 * @notify:      The function to be called.
 * @notify_data: The data to be passed to the callback function.
 *
 * Adds a function called when notification dialog is closed.
 */
void
purple_request_add_close_notify(void *ui_handle, GDestroyNotify notify,
	gpointer notify_data);

/**
 * purple_request_close:
 * @type:     The request type.
 * @uihandle: The request UI handle.
 *
 * Closes a request.
 */
void purple_request_close(PurpleRequestType type, void *uihandle);

/**
 * purple_request_close_with_handle:
 * @handle: The handle, as supplied as the @handle parameter to one of the
 *          <literal>purple_request_*</literal> functions.
 *
 * Closes all requests registered with the specified handle.
 *
 * See purple_request_input().
 */
void purple_request_close_with_handle(void *handle);

/**
 * purple_request_yes_no:
 * @handle:         The handle, as supplied as the @handle parameter to one of the
 *                  <literal>purple_request_*</literal> functions.
 * @title:          The title of the message, or %NULL if it should have
 *                  no title.
 * @primary:        The main point of the message, or %NULL if you're
 *                  feeling enigmatic.
 * @secondary:      Secondary information, or %NULL if there is none.
 * @default_action: The default action, zero-indexed; if the third action
 *                  supplied should be the default, supply
 *                  <literal>2</literal>.  This should be the action that
 *                  users are most likely to select.
 * @cpar:           The #PurpleRequestCommonParameters object, which gets
 *                  unref'ed after this call.
 * @user_data:      The data to pass to the callback.
 * @yes_cb:         A #PurpleRequestActionCb to call when yes is selected.
 * @no_cb:          A #PurpleRequestActionCb to call when no is selected.
 *
 * A wrapper for purple_request_action() that uses <literal>Yes</literal> and
 * <literal>No</literal> buttons.
 */
#define purple_request_yes_no(handle, title, primary, secondary, default_action, cpar, user_data, yes_cb, no_cb) \
	purple_request_action((handle), (title), (primary), (secondary), \
		(default_action), (cpar), (user_data), 2, _("_Yes"), (yes_cb), \
		_("_No"), (no_cb))

/**
 * purple_request_ok_cancel:
 * @handle:         The handle, as supplied as the @handle parameter to one of
 *                  the <literal>purple_request_*</literal> functions.
 * @title:          The title of the message, or %NULL if it should have
 *                  no title.
 * @primary:        The main point of the message, or %NULL if you're
 *                  feeling enigmatic.
 * @secondary:      Secondary information, or %NULL if there is none.
 * @default_action: The default action, zero-indexed; if the third action
 *                  supplied should be the default, supply
 *                  <literal>2</literal>.  This should be the action that
 *                  users are most likely to select.
 * @cpar:           The #PurpleRequestCommonParameters object, which gets
 *                  unref'ed after this call.
 * @user_data:      The data to pass to the callback.
 * @ok_cb:          A #PurpleRequestActionCb to call when ok is selected.
 * @cancel_cb:      A #PurpleRequestActionCb to call when cancel is selected.
 *
 * A wrapper for purple_request_action() that uses <literal>OK</literal> and
 * <literal>Cancel</literal> buttons.
 */
#define purple_request_ok_cancel(handle, title, primary, secondary, default_action, cpar, user_data, ok_cb, cancel_cb) \
	purple_request_action((handle), (title), (primary), (secondary), \
		(default_action), (cpar), (user_data), 2, _("_OK"), (ok_cb), \
		_("_Cancel"), (cancel_cb))

/**
 * purple_request_accept_cancel:
 * @handle:         The handle, as supplied as the @handle parameter to one of
 *                  the <literal>purple_request_*</literal> functions.
 * @title:          The title of the message, or %NULL if it should have
 *                  no title.
 * @primary:        The main point of the message, or %NULL if you're
 *                  feeling enigmatic.
 * @secondary:      Secondary information, or %NULL if there is none.
 * @default_action: The default action, zero-indexed; if the third action
 *                  supplied should be the default, supply
 *                  <literal>2</literal>.  This should be the action that
 *                  users are most likely to select.
 * @cpar:           The #PurpleRequestCommonParameters object, which gets
 *                  unref'ed after this call.
 * @user_data:      The data to pass to the callback.
 * @accept_cb:      A #PurpleRequestActionCb to call when accepted is selected.
 * @cancel_cb:      A #PurpleRequestActionCb to call when cancel is selected.
 *
 * A wrapper for purple_request_action() that uses Accept and Cancel buttons.
 */
#define purple_request_accept_cancel(handle, title, primary, secondary, default_action, cpar, user_data, accept_cb, cancel_cb) \
	purple_request_action((handle), (title), (primary), (secondary), \
		(default_action), (cpar), (user_data), 2, _("_Accept"), \
		(accept_cb), _("_Cancel"), (cancel_cb))

/**
 * purple_request_file:
 * @handle:     The plugin or connection handle.  For some things this
 *              is <emphasis>extremely</emphasis> important.  See the comments
 *              on purple_request_input().
 * @title:      The title of the message, or %NULL if it should have no title.
 * @filename:   The default filename (may be %NULL)
 * @savedialog: True if this dialog is being used to save a file.  False if
 *              it is being used to open a file.
 * @ok_cb: (scope notified):      The callback for the <literal>OK</literal> button.
 * @cancel_cb: (scope notified):  The callback for the <literal>Cancel</literal> button, which
 *              may be %NULL.
 * @cpar:       The #PurpleRequestCommonParameters object, which gets unref'ed
 *              after this call.
 * @user_data:  The data to pass to the callback.
 *
 * Displays a file selector request dialog.  Returns the selected filename to
 * the callback.  Can be used for either opening a file or saving a file.
 *
 * Returns: A UI-specific handle.
 */
void *
purple_request_file(void *handle, const char *title, const char *filename,
	gboolean savedialog, GCallback ok_cb, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar, void *user_data);

/**
 * purple_request_folder:
 * @handle:    The plugin or connection handle.  For some things this is
 *             <emphasis>extremely</emphasis> important.  See the comments on
 *             purple_request_input().
 * @title:     The title of the message, or %NULL if it should have no title.
 * @dirname:   The default directory name (may be %NULL)
 * @ok_cb: (scope notified):     The callback for the <literal>OK</literal> button.
 * @cancel_cb: (scope notified): The callback for the <literal>Cancel</literal> button, which
 *             may be %NULL.
 * @cpar:      The #PurpleRequestCommonParameters object, which gets unref'ed
 *             after this call.
 * @user_data: The data to pass to the callback.
 *
 * Displays a folder select dialog. Returns the selected filename to
 * the callback.
 *
 * Returns: A UI-specific handle.
 */
void *
purple_request_folder(void *handle, const char *title, const char *dirname,
	GCallback ok_cb, GCallback cancel_cb,
	PurpleRequestCommonParameters *cpar, void *user_data);

/**************************************************************************/
/* UI Registration Functions                                              */
/**************************************************************************/

/**
 * purple_request_ui_ops_get_type:
 *
 * The standard _get_type function for #PurpleRequestUiOps.
 *
 * Returns: The #GType for the #PurpleRequestUiOps boxed structure.
 */
GType purple_request_ui_ops_get_type(void);

/**
 * purple_request_set_ui_ops:
 * @ops: The UI operations structure.
 *
 * Sets the UI operations structure to be used when displaying a
 * request.
 */
void purple_request_set_ui_ops(PurpleRequestUiOps *ops);

/**
 * purple_request_get_ui_ops:
 *
 * Returns the UI operations structure to be used when displaying a
 * request.
 *
 * Returns: The UI operations structure.
 */
PurpleRequestUiOps *purple_request_get_ui_ops(void);

G_END_DECLS

#endif /* PURPLE_REQUEST_H */
