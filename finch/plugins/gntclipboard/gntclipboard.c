/**
 * Copyright (C) 2007 Richard Nelson <wabz@whatsbeef.net>
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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <glib/gi18n-lib.h>

#define PLUGIN_ID           "gntclipboard"
#define PLUGIN_DOMAIN       (g_quark_from_static_string(PLUGIN_ID))

#ifdef HAVE_X11
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#endif

#include <sys/types.h>
#include <signal.h>

#include <glib.h>

#include <purple.h>

#include <gnt.h>

#include <finch.h>

#ifdef HAVE_X11
static pid_t child = 0;

static gulong sig_handle;

static void
set_clip(gchar *string)
{
	Window w;
	XEvent e, respond;
	XSelectionRequestEvent *req;
	const char *ids;
	Display *dpy = XOpenDisplay(NULL);

	if (!dpy)
		return;
	ids = getenv("WINDOWID");
	if (ids == NULL)
		return;
	w = atoi(ids);
	XSetSelectionOwner(dpy, XA_PRIMARY, w, CurrentTime);
	XFlush(dpy);
	XSelectInput(dpy, w, StructureNotifyMask);
	while (TRUE) {
		XNextEvent(dpy, &e); /* this blocks. */
		req = &e.xselectionrequest;
		if (e.type == SelectionRequest) {
			XChangeProperty(dpy,
				req->requestor,
				req->property,
				XA_STRING,
				8, PropModeReplace,
				(unsigned char *)string,
				strlen(string));
			respond.xselection.property = req->property;
			respond.xselection.type = SelectionNotify;
			respond.xselection.display = req->display;
			respond.xselection.requestor = req->requestor;
			respond.xselection.selection = req->selection;
			respond.xselection.target= req->target;
			respond.xselection.time = req->time;
			XSendEvent(dpy, req->requestor, 0, 0, &respond);
			XFlush (dpy);
		} else if (e.type == SelectionClear) {
			return;
		}
	}
}

static void
clipboard_changed(G_GNUC_UNUSED GntWM *wm, gchar *string)
{
	if (child) {
		kill(child, SIGTERM);
	}
	if ((child = fork()) == 0) {
		set_clip(string);
		_exit(0);
	}
}
#endif

static GPluginPluginInfo *
gnt_clipboard_query(G_GNUC_UNUSED GError **error) {
	const gchar * const authors[] = {
		"Richard Nelson <wabz@whatsbeef.net>",
		NULL
	};

	return finch_plugin_info_new(
		"id",           PLUGIN_ID,
		"name",         N_("GntClipboard"),
		"version",      DISPLAY_VERSION,
		"category",     N_("Utility"),
		"summary",      N_("Clipboard plugin"),
		"description",  N_("When the gnt clipboard contents change, the "
		                   "contents are made available to X, if possible."),
		"authors",      authors,
		"website",      PURPLE_WEBSITE,
		"abi-version",  PURPLE_ABI_VERSION,
		NULL
	);
}

static gboolean
gnt_clipboard_load(G_GNUC_UNUSED GPluginPlugin *plugin,
                   G_GNUC_UNUSED GError **error)
{
#ifdef HAVE_X11
	if (!XOpenDisplay(NULL)) {
		purple_debug_warning("gntclipboard", "Couldn't find X display\n");
		purple_notify_error(NULL, _("Error"), _("Error loading the plugin."),
				_("Couldn't find X display"), NULL);
		return FALSE;
	}
	if (!getenv("WINDOWID")) {
		purple_debug_warning("gntclipboard", "Couldn't find window\n");
		purple_notify_error(NULL, _("Error"), _("Error loading the plugin."),
				_("Couldn't find window"), NULL);
		return FALSE;
	}
	sig_handle = g_signal_connect(G_OBJECT(gnt_get_clipboard()), "clipboard_changed", G_CALLBACK(clipboard_changed), NULL);
	return TRUE;
#else
	g_set_error(error, PLUGIN_DOMAIN, 0, _("This plugin cannot be loaded "
			"because it was not built with X11 support."));
	return FALSE;
#endif
}

static gboolean
gnt_clipboard_unload(G_GNUC_UNUSED GPluginPlugin *plugin,
                     G_GNUC_UNUSED gboolean shutdown,
                     G_GNUC_UNUSED GError **error)
{
#ifdef HAVE_X11
	if (child) {
		kill(child, SIGTERM);
		child = 0;
	}
	g_signal_handler_disconnect(G_OBJECT(gnt_get_clipboard()), sig_handle);
#endif
	return TRUE;
}

GPLUGIN_NATIVE_PLUGIN_DECLARE(gnt_clipboard)