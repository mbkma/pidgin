/*
 * Purple - Internet Messaging Library
 * Copyright (C) Pidgin Developers <devel@pidgin.im>
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
#if !defined(PURPLE_IRCV3_GLOBAL_HEADER_INSIDE) && \
    !defined(PURPLE_IRCV3_COMPILATION)
# error "only <libpurple/protocols/ircv3.h> may be included directly"
#endif

#ifndef PURPLE_IRCV3_CORE_H
#define PURPLE_IRCV3_CORE_H

#include <glib.h>

#define PURPLE_IRCV3_DEFAULT_SERVER "irc.libera.chat"
#define PURPLE_IRCV3_DEFAULT_PLAIN_PORT 6667
#define PURPLE_IRCV3_DEFAULT_TLS_PORT 6697

#define PURPLE_IRCV3_DOMAIN (g_quark_from_static_string("ircv3-plugin"))

#endif /* PURPLE_IRCV3_CORE_H */
