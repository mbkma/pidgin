/* Copyright (C) 2003-2004 Timothy Ringenbach <omarvo@hotmail.com
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

#include "account.h"
#include "purplemarkup.h"
#include "cmds.h"

static GList *cmds = NULL;
static guint next_id = 1;

typedef struct {
	PurpleCmdId id;
	gchar *cmd;
	gchar *args;
	PurpleCmdPriority priority;
	PurpleCmdFlag flags;
	gchar *protocol_id;
	PurpleCmdFunc func;
	gchar *help;
	void *data;
} PurpleCmd;


static gint cmds_compare_func(const PurpleCmd *a, const PurpleCmd *b)
{
	return b->priority - a->priority;
}

PurpleCmdId purple_cmd_register(const gchar *cmd, const gchar *args,
                            PurpleCmdPriority p, PurpleCmdFlag f,
                            const gchar *protocol_id, PurpleCmdFunc func,
                            const gchar *helpstr, void *data)
{
	PurpleCmdId id;
	PurpleCmd *c;

	g_return_val_if_fail(cmd != NULL && *cmd != '\0', 0);
	g_return_val_if_fail(args != NULL, 0);
	g_return_val_if_fail(func != NULL, 0);

	id = next_id++;

	c = g_new0(PurpleCmd, 1);
	c->id = id;
	c->cmd = g_strdup(cmd);
	c->args = g_strdup(args);
	c->priority = p;
	c->flags = f;
	c->protocol_id = g_strdup(protocol_id);
	c->func = func;
	c->help = g_strdup(helpstr);
	c->data = data;

	cmds = g_list_insert_sorted(cmds, c, (GCompareFunc)cmds_compare_func);

	purple_signal_emit(purple_cmds_get_handle(), "cmd-added", cmd, p, f);

	return id;
}

static void purple_cmd_free(PurpleCmd *c)
{
	g_free(c->cmd);
	g_free(c->args);
	g_free(c->protocol_id);
	g_free(c->help);
	g_free(c);
}

static gint
purple_cmd_cmp_id(gconstpointer cmd, gconstpointer id)
{
	return ((PurpleCmd *)cmd)->id - GPOINTER_TO_UINT(id);
}

void purple_cmd_unregister(PurpleCmdId id)
{
	PurpleCmd *c;
	GList *l;

	l = g_list_find_custom(cmds, GUINT_TO_POINTER(id), purple_cmd_cmp_id);
	if (!l) {
		return;
	}

	c = l->data;

	cmds = g_list_delete_link(cmds, l);
	purple_signal_emit(purple_cmds_get_handle(), "cmd-removed", c->cmd);
	purple_cmd_free(c);
}

/*
 * This sets args to a NULL-terminated array of strings.  It should
 * be freed using g_strfreev().
 */
static gboolean purple_cmd_parse_args(PurpleCmd *cmd, const gchar *s, const gchar *m, gchar ***args)
{
	int i;
	const char *end, *cur;

	*args = g_new0(char *, strlen(cmd->args) + 1);

	cur = s;

	for (i = 0; cmd->args[i]; i++) {
		if (!*cur)
			return (cmd->flags & PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS);

		switch (cmd->args[i]) {
		case 'w':
			if (!(end = strchr(cur, ' '))) {
			  end = cur + strlen(cur);
			  (*args)[i] = g_strndup(cur, end - cur);
			  cur = end;
			} else {
			  (*args)[i] = g_strndup(cur, end - cur);
			  cur = end + 1;
			}
			break;
		case 'W':
		        if (!(end = strchr(cur, ' '))) {
			  end = cur + strlen(cur);
			  (*args)[i] = purple_markup_slice(m, g_utf8_pointer_to_offset(s, cur), g_utf8_pointer_to_offset(s, end));
			  cur = end;
			} else {
			  (*args)[i] = purple_markup_slice(m, g_utf8_pointer_to_offset(s, cur), g_utf8_pointer_to_offset(s, end));
			  cur = end +1;
			}
			break;
		case 's':
			(*args)[i] = g_strdup(cur);
			cur = cur + strlen(cur);
			break;
		case 'S':
			(*args)[i] = purple_markup_slice(m, g_utf8_pointer_to_offset(s, cur), g_utf8_strlen(cur, -1) + 1);
			cur = cur + strlen(cur);
			break;
		}
	}

	if (*cur)
		return (cmd->flags & PURPLE_CMD_FLAG_ALLOW_WRONG_ARGS);

	return TRUE;
}

static void purple_cmd_strip_current_char(gunichar c, char *s, guint len)
{
	int bytes;

	bytes = g_unichar_to_utf8(c, NULL);
	memmove(s, s + bytes, len + 1 - bytes);
}

static void purple_cmd_strip_cmd_from_markup(char *markup)
{
	guint len = strlen(markup);
	char *s = markup;

	while (*s) {
		gunichar c = g_utf8_get_char(s);

		if (c == '<') {
			s = strchr(s, '>');
			if (!s)
				return;
		} else if (g_unichar_isspace(c)) {
			purple_cmd_strip_current_char(c, s, len - (s - markup));
			return;
		} else {
			purple_cmd_strip_current_char(c, s, len - (s - markup));
			continue;
		}
		s = g_utf8_next_char(s);
	}
}

static gboolean
is_right_type(PurpleCmd *cmd, PurpleConversation *conv)
{
	return (PURPLE_IS_IM_CONVERSATION(conv) && (cmd->flags & PURPLE_CMD_FLAG_IM))
	    || (PURPLE_IS_CHAT_CONVERSATION(conv) && (cmd->flags & PURPLE_CMD_FLAG_CHAT));
}

static gboolean
is_right_protocol(PurpleCmd *cmd, PurpleConversation *conv)
{
	const gchar *protocol_id = purple_account_get_protocol_id(purple_conversation_get_account(conv));

	return !(cmd->flags & PURPLE_CMD_FLAG_PROTOCOL_ONLY)
	    || purple_strequal(cmd->protocol_id, protocol_id);
}

PurpleCmdStatus purple_cmd_do_command(PurpleConversation *conv, const gchar *cmdline,
                                  const gchar *markup, gchar **error)
{
	gchar *err = NULL;
	gboolean found = FALSE, tried_cmd = FALSE, right_type = FALSE, right_protocol = FALSE;
	gchar *cmd, *rest, *mrest;
	PurpleCmdRet ret = PURPLE_CMD_RET_CONTINUE;

	*error = NULL;

	rest = strchr(cmdline, ' ');
	if (rest) {
		cmd = g_strndup(cmdline, rest - cmdline);
		rest++;
	} else {
		cmd = g_strdup(cmdline);
		rest = "";
	}

	mrest = g_strdup(markup);
	purple_cmd_strip_cmd_from_markup(mrest);

	for (GList *l = cmds; l; l = l->next) {
		PurpleCmd *c = l->data;
		gchar **args = NULL;

		if (!purple_strequal(c->cmd, cmd))
			continue;

		found = TRUE;

		if (!is_right_type(c, conv)) {
			continue;
		}

		right_type = TRUE;

		if (!is_right_protocol(c, conv)) {
			continue;
		}

		right_protocol = TRUE;

		/* this checks the allow bad args flag for us */
		if (!purple_cmd_parse_args(c, rest, mrest, &args)) {
			g_strfreev(args);
			continue;
		}

		tried_cmd = TRUE;
		ret = c->func(conv, cmd, args, &err, c->data);
		g_strfreev(args);
		if (ret == PURPLE_CMD_RET_CONTINUE) {
			g_free(err);
			err = NULL;
			continue;
		}

		break;
	}

	g_free(cmd);
	g_free(mrest);

	if (!found)
		return PURPLE_CMD_STATUS_NOT_FOUND;

	if (!right_type)
		return PURPLE_CMD_STATUS_WRONG_TYPE;
	if (!right_protocol)
		return PURPLE_CMD_STATUS_WRONG_PROTOCOL;
	if (!tried_cmd)
		return PURPLE_CMD_STATUS_WRONG_ARGS;

	if (ret != PURPLE_CMD_RET_OK) {
		*error = err;
		if (ret == PURPLE_CMD_RET_CONTINUE)
			return PURPLE_CMD_STATUS_NOT_FOUND;
		else
			return PURPLE_CMD_STATUS_FAILED;
	}

	return PURPLE_CMD_STATUS_OK;
}

gboolean purple_cmd_execute(PurpleCmdId id, PurpleConversation *conv,
			    const gchar *cmdline)
{
	PurpleCmd *cmd = NULL;
	PurpleCmdRet ret = PURPLE_CMD_RET_CONTINUE;
	GList *l = NULL;
	gchar *err = NULL;
	gchar **args = NULL;

	l = g_list_find_custom(cmds, GUINT_TO_POINTER(id), purple_cmd_cmp_id);
	if (!l) {
		return FALSE;
	}

	cmd = l->data;

	if (!is_right_type(cmd, conv)) {
		return FALSE;
	}

	/* XXX: Don't worry much about the markup version of the command
	   line, there's not a single use case... */
	/* this checks the allow bad args flag for us */
	if (!purple_cmd_parse_args(cmd, cmdline, cmdline, &args)) {
		g_strfreev(args);
		return FALSE;
	}

	ret = cmd->func(conv, cmd->cmd, args, &err, cmd->data);

	g_free(err);
	g_strfreev(args);

	return ret == PURPLE_CMD_RET_OK;
}

GList *purple_cmd_list(PurpleConversation *conv)
{
	GList *ret = NULL;

	for (GList *l = cmds; l; l = l->next) {
		PurpleCmd *c = l->data;

		if (conv && (!is_right_type(c, conv) || !is_right_protocol(c, conv))) {
			continue;
		}

		ret = g_list_append(ret, c->cmd);
	}

	ret = g_list_sort(ret, (GCompareFunc)strcmp);

	return ret;
}


GList *purple_cmd_help(PurpleConversation *conv, const gchar *cmd)
{
	GList *ret = NULL;

	for (GList *l = cmds; l; l = l->next) {
		PurpleCmd *c = l->data;

		if (cmd && !purple_strequal(cmd, c->cmd))
			continue;

		if (conv && (!is_right_type(c, conv) || !is_right_protocol(c, conv))) {
			continue;
		}

		ret = g_list_append(ret, c->help);
	}

	ret = g_list_sort(ret, (GCompareFunc)strcmp);

	return ret;
}

gpointer purple_cmds_get_handle(void)
{
	static int handle;
	return &handle;
}

void purple_cmds_init(void)
{
	gpointer handle = purple_cmds_get_handle();

	purple_signal_register(handle, "cmd-added",
			purple_marshal_VOID__POINTER_INT_INT, G_TYPE_NONE, 3,
			G_TYPE_STRING, G_TYPE_INT, G_TYPE_INT);
	purple_signal_register(handle, "cmd-removed",
			purple_marshal_VOID__POINTER, G_TYPE_NONE, 1,
			G_TYPE_STRING);
}

void purple_cmds_uninit(void)
{
	purple_signals_unregister_by_instance(purple_cmds_get_handle());

	g_clear_list(&cmds, (GDestroyNotify)purple_cmd_free);
}

