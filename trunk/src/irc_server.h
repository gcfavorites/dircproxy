/* dircproxy
 * Copyright (C) 2000 Scott James Remnant <scott@netsplit.com>.
 * All Rights Reserved.
 *
 * irc_server.h
 * --
 * @(#) $Id: irc_server.h,v 1.2 2000/09/28 10:37:15 keybuk Exp $
 *
 * This file is distributed according to the GNU General Public
 * License.  For full details, read the top of 'main.c' or the
 * file called COPYING that was distributed with this code.
 */

#ifndef __DIRCPROXY_IRC_SERVER_H
#define __DIRCPROXY_IRC_SERVER_H

/* required includes */
#include "irc_net.h"

/* functions */
extern int ircserver_connect(struct ircproxy *);
extern int ircserver_connected(struct ircproxy *);
extern int ircserver_connectfailed(struct ircproxy *, int);
extern int ircserver_data(struct ircproxy *);
extern int ircserver_close_sock(struct ircproxy *);
extern void ircserver_resetidle(struct ircproxy *);
extern int ircserver_send_command(struct ircproxy *, const char *, const char *,
                                  ...);
extern int ircserver_send_peercmd(struct ircproxy *, const char *, const char *,
                                  ...);

#endif /* __DIRCPROXY_IRC_SERVER_H */
