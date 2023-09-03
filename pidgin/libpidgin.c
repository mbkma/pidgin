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

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <locale.h>

#include <purple.h>

#include "pidginapplication.h"
#include "pidgincore.h"

int
pidgin_start(int argc, char *argv[]) {
	GApplication *app;
	int ret;

	bindtextdomain(PACKAGE, PURPLE_LOCALEDIR);
	bind_textdomain_codeset(PACKAGE, "UTF-8");
	textdomain(PACKAGE);

	/* Locale initialization is not complete here.  See gtk_init_check() */
	setlocale(LC_ALL, "");

	app = pidgin_application_new();
	g_application_set_default(app);

	ret = g_application_run(app, argc, argv);

	/* Make sure purple has quit in case something in GApplication
	 * has caused g_application_run() to finish on its own. This can
	 * happen, for example, if the desktop session is ending.
	 */
	if(purple_get_core() != NULL) {
		purple_core_quit();
	}

	if(g_application_get_is_registered(app) &&
	   g_application_get_is_remote(app))
	{
		g_printerr("%s\n",
		           _("Exiting because another libpurple client is already "
		             "running."));
	}

	/* Now that we're sure purple_core_quit() has been called,
	 * this can be freed.
	 */
	g_object_unref(app);

#ifdef _WIN32
	winpidgin_cleanup();
#endif

	return ret;
}
