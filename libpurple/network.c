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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#include "purpleprivate.h"

#include <gio/gio.h>

#include <fcntl.h>

#ifdef _WIN32
# include "win32/libc_interface.h"
# include <nspapi.h>
#endif

#include "debug.h"
#include "account.h"
#include "network.h"
#include "prefs.h"

static gboolean force_online = FALSE;

void
purple_network_set_public_ip(const char *ip)
{
	g_return_if_fail(ip != NULL);

	/* XXX - Ensure the IP address is valid */

	purple_prefs_set_string("/purple/network/public_ip", ip);
}

const char *
purple_network_get_public_ip(void)
{
	return purple_prefs_get_string("/purple/network/public_ip");
}

static gchar *
purple_network_get_local_system_ip_from_gio(GSocketConnection *sockconn)
{
	GSocketAddress *addr;
	GInetSocketAddress *inetsockaddr;
	gchar *ip;

	addr = g_socket_connection_get_local_address(sockconn, NULL);
	if ((inetsockaddr = G_INET_SOCKET_ADDRESS(addr)) != NULL) {
		GInetAddress *inetaddr =
		        g_inet_socket_address_get_address(inetsockaddr);
		if (g_inet_address_get_family(inetaddr) == G_SOCKET_FAMILY_IPV4 &&
		    !g_inet_address_get_is_loopback(inetaddr)) {
			ip = g_inet_address_to_string(inetaddr);
			g_object_unref(addr);
			return ip;
		}
	}
	g_object_unref(addr);

	return g_strdup("0.0.0.0");
}

/*
 * purple_network_is_ipv4:
 * @hostname: The hostname to be verified.
 *
 * Checks, if specified hostname is valid ipv4 address.
 *
 * Returns: TRUE, if the hostname is valid.
 */
static gboolean
purple_network_is_ipv4(const gchar *hostname)
{
	g_return_val_if_fail(hostname != NULL, FALSE);

	/* We don't accept ipv6 here. */
	if (strchr(hostname, ':') != NULL)
		return FALSE;

	return g_hostname_is_ip_address(hostname);
}

void
purple_network_discover_my_ip(void)
{
	const char *ip = NULL;

	/* Check if the user specified an IP manually */
	if (!purple_prefs_get_bool("/purple/network/auto_ip")) {
		ip = purple_network_get_public_ip();
		/* Make sure the IP address entered by the user is valid */
		if (ip != NULL && purple_network_is_ipv4(ip)) {
			return;
		}
	}
}

gchar *
purple_network_get_my_ip_from_gio(GSocketConnection *sockconn)
{
	/* Check if the user specified an IP manually */
	if (!purple_prefs_get_bool("/purple/network/auto_ip")) {
		const gchar *ip = NULL;

		ip = purple_network_get_public_ip();

		/* Make sure the IP address entered by the user is valid */
		if ((ip != NULL) && (purple_network_is_ipv4(ip))) {
			return g_strdup(ip);
		}
	}

	/* Just fetch the IP of the local system */
	return purple_network_get_local_system_ip_from_gio(sockconn);
}

gboolean
purple_network_is_available(void)
{
	if(force_online) {
		return TRUE;
	}

	return g_network_monitor_get_network_available(g_network_monitor_get_default());
}

void
purple_network_force_online(void)
{
	force_online = TRUE;
}

gboolean
_purple_network_set_common_socket_flags(int fd)
{
	int flags;
	gboolean succ = TRUE;

	g_return_val_if_fail(fd >= 0, FALSE);

	flags = fcntl(fd, F_GETFL);

	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) != 0) {
		purple_debug_warning("network",
			"Couldn't set O_NONBLOCK flag\n");
		succ = FALSE;
	}

#ifndef _WIN32
	if (fcntl(fd, F_SETFD, FD_CLOEXEC) != 0) {
		purple_debug_warning("network",
			"Couldn't set FD_CLOEXEC flag\n");
		succ = FALSE;
	}
#endif

	return succ;
}

void
purple_network_init(void)
{
	purple_prefs_add_none  ("/purple/network");
	purple_prefs_add_bool  ("/purple/network/auto_ip", TRUE);
	purple_prefs_add_string("/purple/network/public_ip", "");
	purple_prefs_add_bool  ("/purple/network/map_ports", TRUE);
	purple_prefs_add_bool  ("/purple/network/ports_range_use", FALSE);
	purple_prefs_add_int   ("/purple/network/ports_range_start", 1024);
	purple_prefs_add_int   ("/purple/network/ports_range_end", 2048);
}

void
purple_network_uninit(void)
{
}
