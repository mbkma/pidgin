/* pidgin
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 *
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <gplugin.h>

#include <gdk/gdkkeysyms.h>

#include <talkatu.h>

#include <purple.h>

#include "gtkconv.h"
#include "pidgincore.h"
#include "pidgindisplaywindow.h"
#include "pidgininfopane.h"
#include "pidginmessage.h"

/* Prototypes. <-- because Paco-Paco hates this comment. */
gboolean pidgin_conv_has_focus(PurpleConversation *conv);

static void pidgin_conv_placement_place(PidginConversationOld *conv);

/**************************************************************************
 * Callbacks
 **************************************************************************/

static gboolean
close_this_sucker(gpointer data)
{
	PidginConversationOld *gtkconv = data;
	GList *list = g_list_copy(gtkconv->convs);
	purple_debug_misc("gtkconv", "closing %s", purple_conversation_get_name(list->data));
	g_list_free_full(list, g_object_unref);
	return FALSE;
}

static gboolean
close_conv_cb(G_GNUC_UNUSED GtkButton *button, PidginConversationOld *gtkconv)
{
	/* We are going to destroy the conversations immediately only if the 'close immediately'
	 * preference is selected. Otherwise, close the conversation after a reasonable timeout
	 * (I am going to consider 10 minutes as a 'reasonable timeout' here.
	 * For chats, close immediately if the chat is not in the buddylist, or if the chat is
	 * not marked 'Persistent' */
	PurpleConversation *conv = gtkconv->active_conv;

	if(PURPLE_IS_IM_CONVERSATION(conv) || PURPLE_IS_CHAT_CONVERSATION(conv)) {
		close_this_sucker(gtkconv);
	}

	return TRUE;
}

static gboolean
check_for_and_do_command(PurpleConversation *conv)
{
	PidginConversationOld *gtkconv;
	GtkWidget *input = NULL;
	GtkTextBuffer *buffer = NULL;
	gchar *cmd;
	const gchar *prefix;
	gboolean retval = FALSE;

	gtkconv = PIDGIN_CONVERSATION_OLD(conv);
	prefix = "/";

	input = talkatu_editor_get_input(TALKATU_EDITOR(gtkconv->editor));
	buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(input));

	cmd = talkatu_buffer_get_plain_text(TALKATU_BUFFER(buffer));

	if (cmd && g_str_has_prefix(cmd, prefix)) {
		PurpleCmdStatus status;
		char *error, *cmdline, *markup;

		cmdline = cmd + strlen(prefix);

		if (purple_strequal(cmdline, "xyzzy")) {
			purple_conversation_write_system_message(conv,
				"Nothing happens", PURPLE_MESSAGE_NO_LOG);
			g_free(cmd);
			return TRUE;
		}

		/* Docs are unclear on whether or not prefix should be removed from
		 * the markup so, ignoring for now.  Notably if the markup is
		 * `<b>/foo arg1</b>` we now have to move the bold tag around?
		 * - gk 20190709 */
		markup = talkatu_markup_get_html(buffer, NULL);
		status = purple_cmd_do_command(conv, cmdline, markup, &error);
		g_free(markup);

		switch (status) {
			case PURPLE_CMD_STATUS_OK:
				retval = TRUE;
				break;
			case PURPLE_CMD_STATUS_NOT_FOUND:
				{
					PurpleProtocol *protocol = NULL;
					PurpleConnection *gc;

					if ((gc = purple_conversation_get_connection(conv)))
						protocol = purple_connection_get_protocol(gc);

					if ((protocol != NULL) && (purple_protocol_get_options(protocol) & OPT_PROTO_SLASH_COMMANDS_NATIVE)) {
						char *spaceslash;

						/* If the first word in the entered text has a '/' in it, then the user
						 * probably didn't mean it as a command. So send the text as message. */
						spaceslash = cmdline;
						while (*spaceslash && *spaceslash != ' ' && *spaceslash != '/')
							spaceslash++;

						if (*spaceslash != '/') {
							purple_conversation_write_system_message(conv,
								_("Unknown command."), PURPLE_MESSAGE_NO_LOG);
							retval = TRUE;
						}
					}
					break;
				}
			case PURPLE_CMD_STATUS_WRONG_ARGS:
				purple_conversation_write_system_message(conv,
					_("Syntax Error:  You typed the wrong "
					"number of arguments to that command."),
					PURPLE_MESSAGE_NO_LOG);
				retval = TRUE;
				break;
			case PURPLE_CMD_STATUS_FAILED:
				purple_conversation_write_system_message(conv,
					error ? error : _("Your command failed for an unknown reason."),
					PURPLE_MESSAGE_NO_LOG);
				g_free(error);
				retval = TRUE;
				break;
			case PURPLE_CMD_STATUS_WRONG_TYPE:
				if(PURPLE_IS_IM_CONVERSATION(conv))
					purple_conversation_write_system_message(conv,
						_("That command only works in chats, not IMs."),
						PURPLE_MESSAGE_NO_LOG);
				else
					purple_conversation_write_system_message(conv,
						_("That command only works in IMs, not chats."),
						PURPLE_MESSAGE_NO_LOG);
				retval = TRUE;
				break;
			case PURPLE_CMD_STATUS_WRONG_PROTOCOL:
				purple_conversation_write_system_message(conv,
					_("That command doesn't work on this protocol."),
					PURPLE_MESSAGE_NO_LOG);
				retval = TRUE;
				break;
		}
	}

	g_free(cmd);

	return retval;
}

static void
send_cb(G_GNUC_UNUSED GtkWidget *widget, PidginConversationOld *gtkconv)
{
	PurpleConversation *conv = gtkconv->active_conv;
	PurpleAccount *account;
	PurpleMessageFlags flags = 0;
	GtkTextBuffer *buffer = NULL;
	gchar *content;

	account = purple_conversation_get_account(conv);

	buffer = talkatu_editor_get_buffer(TALKATU_EDITOR(gtkconv->editor));

	if (check_for_and_do_command(conv)) {
		talkatu_buffer_clear(TALKATU_BUFFER(buffer));
		return;
	}

	if (PURPLE_IS_CHAT_CONVERSATION(conv) &&
		purple_chat_conversation_has_left(PURPLE_CHAT_CONVERSATION(conv))) {
		return;
	}

	if (!purple_account_is_connected(account)) {
		return;
	}

	content = talkatu_markup_get_html(buffer, NULL);
	if (purple_strequal(content, "")) {
		g_free(content);
		return;
	}

	purple_idle_touch();

	/* XXX: is there a better way to tell if the message has images? */
	// if (strstr(buf, "<img ") != NULL)
	// 	flags |= PURPLE_MESSAGE_IMAGES;

	purple_conversation_send_with_flags(conv, content, flags);

	g_free(content);

	talkatu_buffer_clear(TALKATU_BUFFER(buffer));
}

/**************************************************************************
 * Utility functions
 **************************************************************************/
static GtkWidget *
setup_common_pane(PidginConversationOld *gtkconv)
{
	GSimpleActionGroup *ag = NULL;
	GtkTextBuffer *buffer = NULL;
	GtkWidget *vbox, *input, *hpaned, *sw;
	PurpleConversation *conv = gtkconv->active_conv;

	/* Setup the top part of the pane */
	vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);

	/* Setup the info pane */
	gtkconv->infopane = pidgin_info_pane_new(conv);
	gtk_widget_set_vexpand(gtkconv->infopane, FALSE);
	gtk_box_append(GTK_BOX(vbox), gtkconv->infopane);

	/* Setup the history widget */
	sw = gtk_scrolled_window_new();
	gtkconv->vadjustment = talkatu_auto_scroller_new();
	gtk_scrolled_window_set_vadjustment(GTK_SCROLLED_WINDOW(sw),
	                                    gtkconv->vadjustment);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
	                               GTK_POLICY_ALWAYS);

	gtkconv->history = talkatu_history_new();
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(sw), gtkconv->history);

	/* Add the talkatu history */
	hpaned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_widget_set_vexpand(hpaned, TRUE);
	gtk_box_append(GTK_BOX(vbox), hpaned);
	gtk_paned_set_start_child(GTK_PANED(hpaned), sw);
	gtk_paned_set_resize_start_child(GTK_PANED(hpaned), TRUE);
	gtk_paned_set_shrink_start_child(GTK_PANED(hpaned), TRUE);

	g_object_set_data(G_OBJECT(gtkconv->history), "gtkconv", gtkconv);

	/* Setup the entry widget and all signals */
	gtkconv->editor = talkatu_editor_new();
	ag = talkatu_action_group_new(TALKATU_FORMAT_HTML);
	buffer = talkatu_buffer_new(ag);
	talkatu_action_group_set_buffer(TALKATU_ACTION_GROUP(ag), buffer);
	g_clear_object(&ag);

	talkatu_editor_set_buffer(TALKATU_EDITOR(gtkconv->editor), buffer);
	g_clear_object(&buffer);
	gtk_box_append(GTK_BOX(vbox), gtkconv->editor);

	input = talkatu_editor_get_input(TALKATU_EDITOR(gtkconv->editor));
	gtk_widget_set_name(input, "pidgin_conv_entry");
	talkatu_input_set_send_binding(TALKATU_INPUT(input), TALKATU_INPUT_SEND_BINDING_RETURN | TALKATU_INPUT_SEND_BINDING_KP_ENTER);
	g_signal_connect(
		G_OBJECT(input),
		"send-message",
		G_CALLBACK(send_cb),
		gtkconv
	);

	return vbox;
}

static PidginConversationOld *
pidgin_conv_find_gtkconv(PurpleConversation * conv)
{
	PurpleBuddy *bud = purple_blist_find_buddy(purple_conversation_get_account(conv), purple_conversation_get_name(conv));
	PurpleMetaContact *c;
	PurpleBlistNode *cn, *bn;

	if (!bud)
		return NULL;

	if (!(c = purple_buddy_get_contact(bud)))
		return NULL;

	cn = PURPLE_BLIST_NODE(c);
	for (bn = purple_blist_node_get_first_child(cn); bn; bn = purple_blist_node_get_sibling_next(bn)) {
		PurpleBuddy *b = PURPLE_BUDDY(bn);
		PurpleConversation *im;
		PurpleConversationManager *manager;

		manager = purple_conversation_manager_get_default();
		im = purple_conversation_manager_find_im(manager,
		                                         purple_buddy_get_account(b),
		                                         purple_buddy_get_name(b));

		if(PIDGIN_CONVERSATION_OLD(im)) {
			return PIDGIN_CONVERSATION_OLD(im);
		}
	}

	return NULL;
}

static gboolean
ignore_middle_click(G_GNUC_UNUSED GtkGestureClick *click,
                    gint n_press, G_GNUC_UNUSED gdouble x,
                    G_GNUC_UNUSED gdouble y, G_GNUC_UNUSED gpointer data)
{
	/* A click on the pane is propagated to the notebook containing the pane.
	 * So if Stu accidentally aims high and middle clicks on the pane-handle,
	 * it causes a conversation tab to close. Let's stop that from happening.
	 */
	if (n_press == 1) {
		return TRUE;
	}
	return FALSE;
}

/**************************************************************************
 * Conversation UI operations
 **************************************************************************/
static void
private_gtkconv_new(PurpleConversation *conv, G_GNUC_UNUSED gboolean hidden)
{
	PidginConversationOld *gtkconv;
	GtkWidget *tab_cont, *pane;
	GtkGesture *click = NULL;

	if (PURPLE_IS_IM_CONVERSATION(conv) && (gtkconv = pidgin_conv_find_gtkconv(conv))) {
		purple_debug_misc("gtkconv", "found existing gtkconv %p", gtkconv);
		g_object_set_data(G_OBJECT(conv), "pidgin", gtkconv);
		if (!g_list_find(gtkconv->convs, conv))
			gtkconv->convs = g_list_prepend(gtkconv->convs, conv);
		return;
	}

	purple_debug_misc("gtkconv", "creating new gtkconv for %p", conv);
	gtkconv = g_new0(PidginConversationOld, 1);
	g_object_set_data(G_OBJECT(conv), "pidgin", gtkconv);
	gtkconv->active_conv = conv;
	gtkconv->convs = g_list_prepend(gtkconv->convs, conv);

	pane = setup_common_pane(gtkconv);

	if (pane == NULL) {
		g_free(gtkconv);
		g_object_set_data(G_OBJECT(conv), "pidgin", NULL);
		return;
	}

	click = gtk_gesture_click_new();
	gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(click), GDK_BUTTON_MIDDLE);
	g_signal_connect(click, "pressed", G_CALLBACK(ignore_middle_click), NULL);
	gtk_widget_add_controller(pane, GTK_EVENT_CONTROLLER(click));

	/* Setup the container for the tab. */
	gtkconv->tab_cont = tab_cont = gtk_box_new(GTK_ORIENTATION_VERTICAL, 6);
	g_object_set_data(G_OBJECT(tab_cont), "PidginConversation", gtkconv);
	gtk_widget_set_vexpand(pane, TRUE);
	gtk_box_append(GTK_BOX(tab_cont), pane);

	talkatu_editor_set_toolbar_visible(
		TALKATU_EDITOR(gtkconv->editor),
		purple_prefs_get_bool(PIDGIN_PREFS_ROOT "/conversations/show_formatting_toolbar")
	);

	pidgin_conv_placement_place(gtkconv);
}

static void
pidgin_conv_new(PurpleConversation *conv)
{
	private_gtkconv_new(conv, FALSE);
	if (PIDGIN_IS_PIDGIN_CONVERSATION_OLD(conv))
		purple_signal_emit(pidgin_conversations_get_handle(),
				"conversation-displayed", PIDGIN_CONVERSATION_OLD(conv));
}

void
pidgin_conversation_detach(PurpleConversation *conv) {
	if(PIDGIN_IS_PIDGIN_CONVERSATION_OLD(conv)) {
		PidginConversationOld *gtkconv = PIDGIN_CONVERSATION_OLD(conv);

		close_conv_cb(NULL, gtkconv);

		g_free(gtkconv);

		g_object_set_data(G_OBJECT(conv), "pidgin", NULL);
	}
}

static void
received_im_msg_cb(G_GNUC_UNUSED PurpleAccount *account,
                   G_GNUC_UNUSED char *sender, G_GNUC_UNUSED char *message,
                   PurpleConversation *conv,
                   G_GNUC_UNUSED PurpleMessageFlags flags)
{
	guint timer;

	/* Somebody wants to keep this conversation around, so don't time it out */
	if (conv) {
		timer = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "close-timer"));
		if (timer) {
			g_source_remove(timer);
			g_object_set_data(G_OBJECT(conv), "close-timer", GINT_TO_POINTER(0));
		}
	}
}

static void
pidgin_conv_destroy(PurpleConversation *conv)
{
	PidginConversationOld *gtkconv = PIDGIN_CONVERSATION_OLD(conv);
	GtkRoot *win = NULL;

	gtkconv->convs = g_list_remove(gtkconv->convs, conv);
	/* Don't destroy ourselves until all our convos are gone */
	if (gtkconv->convs) {
		/* Make sure the destroyed conversation is not the active one */
		if (gtkconv->active_conv == conv) {
			gtkconv->active_conv = gtkconv->convs->data;
			purple_conversation_update(gtkconv->active_conv, PURPLE_CONVERSATION_UPDATE_FEATURES);
		}
		return;
	}

	win = gtk_widget_get_root(gtkconv->tab_cont);
	pidgin_display_window_remove(PIDGIN_DISPLAY_WINDOW(win), conv);

	/* If the "Save Conversation" or "Save Icon" dialogs are open then close them */
	purple_request_close_with_handle(gtkconv);
	purple_notify_close_with_handle(gtkconv);

	gtk_widget_unparent(gtkconv->tab_cont);

	purple_signals_disconnect_by_handle(gtkconv);

	g_clear_object(&gtkconv->vadjustment);

	g_free(gtkconv);
}

static gboolean
writing_msg(PurpleConversation *conv, PurpleMessage *msg,
            G_GNUC_UNUSED gpointer _unused)
{
	PidginConversationOld *gtkconv;

	g_return_val_if_fail(msg != NULL, FALSE);

	if (!(purple_message_get_flags(msg) & PURPLE_MESSAGE_ACTIVE_ONLY))
		return FALSE;

	g_return_val_if_fail(conv != NULL, FALSE);
	gtkconv = PIDGIN_CONVERSATION_OLD(conv);
	g_return_val_if_fail(gtkconv != NULL, FALSE);

	if (conv == gtkconv->active_conv)
		return FALSE;

	purple_debug_info("gtkconv",
		"Suppressing message for an inactive conversation");

	return TRUE;
}

static void
pidgin_conv_write_conv(PurpleConversation *conv, PurpleMessage *pmsg)
{
	PidginMessage *pidgin_msg = NULL;
	PurpleMessageFlags flags;
	PidginConversationOld *gtkconv;
	PurpleConnection *gc;
	PurpleAccount *account;
	gboolean plugin_return;

	g_return_if_fail(conv != NULL);
	gtkconv = PIDGIN_CONVERSATION_OLD(conv);
	g_return_if_fail(gtkconv != NULL);
	flags = purple_message_get_flags(pmsg);

	account = purple_conversation_get_account(conv);
	g_return_if_fail(account != NULL);
	gc = purple_account_get_connection(account);
	g_return_if_fail(gc != NULL || !(flags & (PURPLE_MESSAGE_SEND | PURPLE_MESSAGE_RECV)));

	plugin_return = GPOINTER_TO_INT(purple_signal_emit_return_1(
		pidgin_conversations_get_handle(),
		(PURPLE_IS_IM_CONVERSATION(conv) ? "displaying-im-msg" : "displaying-chat-msg"),
		conv, pmsg));
	if (plugin_return)
	{
		return;
	}

	pidgin_msg = pidgin_message_new(pmsg);
	talkatu_history_write_message(
		TALKATU_HISTORY(gtkconv->history),
		TALKATU_MESSAGE(pidgin_msg)
	);

	purple_signal_emit(pidgin_conversations_get_handle(),
		(PURPLE_IS_IM_CONVERSATION(conv) ? "displayed-im-msg" : "displayed-chat-msg"),
		conv, pmsg);
}

gboolean
pidgin_conv_has_focus(PurpleConversation *conv)
{
	PidginConversationOld *gtkconv = PIDGIN_CONVERSATION_OLD(conv);
	GtkRoot *win;

	win = gtk_widget_get_root(gtkconv->tab_cont);
	if(gtk_window_is_active(GTK_WINDOW(win))) {
		PidginDisplayWindow *displaywin = PIDGIN_DISPLAY_WINDOW(win);

		if(pidgin_display_window_conversation_is_selected(displaywin, conv)) {
			return TRUE;
		}
	}

	return FALSE;
}

static PurpleConversationUiOps conversation_ui_ops =
{
	.create_conversation = pidgin_conv_new,
	.destroy_conversation = pidgin_conv_destroy,
	.write_conv = pidgin_conv_write_conv,
	.has_focus = pidgin_conv_has_focus,
};

PurpleConversationUiOps *
pidgin_conversations_get_conv_ui_ops(void)
{
	return &conversation_ui_ops;
}

/**************************************************************************
 * Public conversation utility functions
 **************************************************************************/
static void
show_formatting_toolbar_pref_cb(G_GNUC_UNUSED const char *name,
                                G_GNUC_UNUSED PurplePrefType type,
                                gconstpointer value,
                                G_GNUC_UNUSED gpointer data)
{
	GList *list;
	PurpleConversation *conv;
	PurpleConversationManager *manager;
	PidginConversationOld *gtkconv;
	gboolean visible = (gboolean)GPOINTER_TO_INT(value);

	manager = purple_conversation_manager_get_default();
	list = purple_conversation_manager_get_all(manager);
	while(list != NULL) {
		conv = PURPLE_CONVERSATION(list->data);

		if (!PIDGIN_IS_PIDGIN_CONVERSATION_OLD(conv)) {
			list = g_list_delete_link(list, list);

			continue;
		}

		gtkconv = PIDGIN_CONVERSATION_OLD(conv);

		talkatu_editor_set_toolbar_visible(TALKATU_EDITOR(gtkconv->editor), visible);

		list = g_list_delete_link(list, list);
	}
}

static void
pidgin_conv_attach(PurpleConversation *conv)
{
	int timer;
	purple_conversation_set_ui_ops(conv, pidgin_conversations_get_conv_ui_ops());
	if (!PIDGIN_CONVERSATION_OLD(conv))
		private_gtkconv_new(conv, FALSE);
	timer = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(conv), "close-timer"));
	if (timer) {
		g_source_remove(timer);
		g_object_set_data(G_OBJECT(conv), "close-timer", NULL);
	}
}

gboolean pidgin_conv_attach_to_conversation(PurpleConversation *conv)
{
	PidginConversationOld *gtkconv;

	pidgin_conv_attach(conv);
	gtkconv = PIDGIN_CONVERSATION_OLD(conv);

	purple_signal_emit(pidgin_conversations_get_handle(),
	                   "conversation-displayed", gtkconv);

	return TRUE;
}

void *
pidgin_conversations_get_handle(void)
{
	static int handle;

	return &handle;
}

void
pidgin_conversations_init(void)
{
	void *handle = pidgin_conversations_get_handle();

	/* Conversations */
	purple_prefs_add_none(PIDGIN_PREFS_ROOT "/conversations");
	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/conversations/show_incoming_formatting", TRUE);
	purple_prefs_add_int(PIDGIN_PREFS_ROOT "/conversations/minimum_entry_lines", 2);

	purple_prefs_add_bool(PIDGIN_PREFS_ROOT "/conversations/show_formatting_toolbar", TRUE);

	/* Conversations -> Chat */
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/chat");

	/* Conversations -> IM */
	purple_prefs_remove(PIDGIN_PREFS_ROOT "/conversations/im");

	/* Connect callbacks. */
	purple_prefs_connect_callback(handle, PIDGIN_PREFS_ROOT "/conversations/show_formatting_toolbar",
								show_formatting_toolbar_pref_cb, NULL);

	/**********************************************************************
	 * Register signals
	 **********************************************************************/
	purple_signal_register(handle, "displaying-im-msg",
		purple_marshal_BOOLEAN__POINTER_POINTER,
		G_TYPE_BOOLEAN, 2, PURPLE_TYPE_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "displayed-im-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
		PURPLE_TYPE_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "displaying-chat-msg",
		purple_marshal_BOOLEAN__POINTER_POINTER,
		G_TYPE_BOOLEAN, 2, PURPLE_TYPE_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "displayed-chat-msg",
		purple_marshal_VOID__POINTER_POINTER, G_TYPE_NONE, 2,
		PURPLE_TYPE_CONVERSATION, PURPLE_TYPE_MESSAGE);

	purple_signal_register(handle, "conversation-displayed",
						 purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
						 G_TYPE_POINTER); /* (PidginConversationOld *) */

	/**********************************************************************
	 * UI operations
	 **********************************************************************/
	purple_signal_connect(purple_conversations_get_handle(), "writing-im-msg",
		handle, G_CALLBACK(writing_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "writing-chat-msg",
		handle, G_CALLBACK(writing_msg), NULL);
	purple_signal_connect(purple_conversations_get_handle(), "received-im-msg",
						handle, G_CALLBACK(received_im_msg_cb), NULL);

	purple_conversations_set_ui_ops(&conversation_ui_ops);
}

void
pidgin_conversations_uninit(void)
{
	purple_prefs_disconnect_by_handle(pidgin_conversations_get_handle());
	purple_signals_disconnect_by_handle(pidgin_conversations_get_handle());
	purple_signals_unregister_by_instance(pidgin_conversations_get_handle());
}

/**************************************************************************
 * GTK window ops
 **************************************************************************/
static void
pidgin_conv_placement_place(PidginConversationOld *conv) {
	GtkWidget *window = NULL;
	PidginDisplayWindow *display_window = NULL;

	window = pidgin_display_window_get_default();
	display_window = PIDGIN_DISPLAY_WINDOW(window);

	pidgin_display_window_add(display_window, conv->active_conv);
}
