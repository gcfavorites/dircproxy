/* dircproxy
 * Copyright (C) 2000 Scott James Remnant <scott@netsplit.com>.
 * All Rights Reserved.
 *
 * dircproxy.h
 * --
 * @(#) $Id: dircproxy.h,v 1.21 2000/09/01 12:15:36 keybuk Exp $
 *
 * This file is distributed according to the GNU General Public
 * License.  For full details, read the top of 'main.c' or the
 * file called COPYING that was distributed with this code.
 */

#ifndef __DIRCPROXY_DIRCPROXY_H
#define __DIRCPROXY_DIRCPROXY_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#else /* HAVE_CONFIG_H */
#define PACKAGE "dircproxy"
#define VERSION "-debug"
#endif /* HAVE_CONFIG_H */

#include "memdebug.h"

/* Configuration values that aren't in the config file */

/* OLD_RFC1459_PARAM_SPACE
 * RFC1459 says parameters are seperated by one or mode spaces,
 * however RFC2812 says they are seperated by a single space (thus
 * allowing empty parameters).  Define this to use the old RFC1459
 * behaviour IF (and only IF) you have problems.
 */
#undef OLD_RFC1459_PARAM_SPACE

/* ENCRYPTED_PASSWORDS
 * If this is defined, then passwords in the config file are assumed to
 * be encrypted using the system's crypt() function.  This gives added
 * security and means that people who manage to read your config file
 * can't pretend to be you on IRC.
 */
#define ENCRYPTED_PASSWORDS

/* FALLBACK_USERNAME
 * Before sending username's to the server in a USER command, we strip it
 * of bogus characters.  It shouldn't happen, but if somehow it ends up with
 * no other characters left, this value will be used.
 */
#define FALLBACK_USERNAME "user"

/* FALLBACK_NICKNAME
 * When sending a nickname while detached, its possible that we can get
 * errors back from the server.  To this end, we have to generate a new
 * nickname, because the client isn't around to do it for us.  It really
 * will try to generate you a new one, but if all else fails we need
 * something to fall back on
 */
#define FALLBACK_NICKNAME "dircproxy"

/* GLOBAL_CONFIG_FILENAME
 * Filename of the global configuration file.  This file is for when you
 * want to run the dircproxy like any other daemon.
 * It goes under SYSCONFDIR.
 */
#define GLOBAL_CONFIG_FILENAME "dircproxyrc"

/* USER_CONFIG_FILENAME
 * Loaded from the user running dircproxy's home directory.  This file is
 * for when you want to run dircproxy as your user with your own configuration
 */
#define USER_CONFIG_FILENAME ".dircproxyrc"

/* SYSCONFDIR
 * This *should* be defined by the configure script.  Its the etc directory
 * relevant to where dircproxy is being installed to.
 */
#ifndef SYSCONFDIR
#define SYSCONFDIR "/usr/local/etc"
#endif /* SYSCONFDIR */


/* Defaults for values in the configuration file.  Change the config file
 * instead of these */

/* DEFAULT_LISTEN_PORT
 * What port do we listen on for new client connections?
 */
#define DEFAULT_LISTEN_PORT "57000"

/* DEFAULT_SERVER_PORT
 * What port do we connect to IRC servers on if the server string doesn't
 * explicitly set one
 */
#define DEFAULT_SERVER_PORT "6667"

/* DEFAULT_SERVER_RETRY
 * How many seconds after disconnection or last connection attempt do we
 * wait before retrying again?
 */
#define DEFAULT_SERVER_RETRY 15

/* DEFAULT_SERVER_DNSRETRY
 * How many seconds after last connection attempt do we wait before trying
 * again if the error was DNS related?
 */
#define DEFAULT_SERVER_DNSRETRY 60

/* DEFAULT_SERVER_MAXATTEMPTS
 * If we are disconnected from the server, how many times should we iterate
 * the server list before giving up and declaring the proxied connection
 * dead?
 * 0 = iterate forever
 */
#define DEFAULT_SERVER_MAXATTEMPTS 0

/* DEFAULT_SERVER_MAXINITATTEMPTS
 * On first connection, how many times should we iterate the server list
 * before giving up and declaring the proxied connection dead?
 * 0 = iterate forever, not recommended!
 */
#define DEFAULT_SERVER_MAXINITATTEMPTS 5

/* DEFAULT_SERVER_PINGTIMEOUT
 * How many seconds after receiving a PING do we wait until we assume the
 * server is stoned?  Receipt of another ping resets this timer.
 * 0 = don't do stoned checking
 */
#define DEFAULT_SERVER_PINGTIMEOUT 0

/* DEFAULT_CHANNEL_REJOIN
 * If we are kicked off a channel, how many seconds do we wait before attempting
 * to rejoin.
 * -1 = Don't rejoin
 *  0 = Immediately
 */
#define DEFAULT_CHANNEL_REJOIN 15

/* DEFAULT_DISCONNECT_EXISTING
 * If a connecting user tries to use a proxy that is already in user, do
 * we disconnect that proxy?
 *  1 = Yes, causes problems with auto-reconnecting clients
 *  0 = No, disconnect the new user
 */
#define DEFAULT_DISCONNECT_EXISTING 0

/* DEFAULT_DROP_MODES
 * User modes to automatically drop when the client detaches.
 */
#define DEFAULT_DROP_MODES "oOws"

/* DEFAULT_LOCAL_ADDRESS
 * This can be set to a domain name on the local machine that dircproxy
 * should bind to before connection to an irc server.
 * 0 = don't do this
 */
#define DEFAULT_LOCAL_ADDRESS 0

/* DEFAULT_AWAY_MESSAGE
 * If the client detaches without leaving an AWAY message, set this as the
 * AWAY message until it comes back.
 * 0 = don't do this
 */
#define DEFAULT_AWAY_MESSAGE "Not available, messages are logged"

/* DEFAULT_CHAN_LOG_DIR
 * Default directory to log channel text to.
 * 0 = Place in temporary directory and delete after use
 */
#define DEFAULT_CHAN_LOG_DIR 0

/* DEFAULT_CHAN_LOG_ALWAYS
 * Log channel text even while the client is online?
 *  1 = Yes
 *  0 = No
 */
#define DEFAULT_CHAN_LOG_ALWAYS 1

/* DEFAULT_CHAN_LOG_TIMESTAMP
 * Include a timestamp with the logged text.
 *  1 = Yes
 *  0 = No
 */
#define DEFAULT_CHAN_LOG_TIMESTAMP 0

/* DEFAULT_CHAN_LOG_MAXSIZE
 * Maximum number of lines a log file should be.  Once they reach this size,
 * dircproxy will roll the log removing lines from the front.
 *  0 = No limit
 */
#define DEFAULT_CHAN_LOG_MAXSIZE 0

/* DEFAULT_CHAN_LOG_RECALL
 * Number of lines to automatically recall on reconnection to dircproxy.
 * If this is defined, then it is also used as the default size for the
 * /DIRCPROXY RECALL command
 * -1 = All lines (not recommended if always)
 *  0 = Don't recall any
 */
#define DEFAULT_CHAN_LOG_RECALL 128

/* DEFAULT_OTHER_LOG_DIR
 * Default directory to log private messages and notices to.
 * 0 = Place in temporary directory and delete after use
 */
#define DEFAULT_OTHER_LOG_DIR 0

/* DEFAULT_OTHER_LOG_ALWAYS
 * Log private messages and notices even while the client is online?
 *  1 = Yes
 *  0 = No
 */
#define DEFAULT_OTHER_LOG_ALWAYS 0

/* DEFAULT_OTHER_LOG_TIMESTAMP
 * Include a timestamp with the logged text.
 *  1 = Yes
 *  0 = No
 */
#define DEFAULT_OTHER_LOG_TIMESTAMP 0

/* DEFAULT_OTHER_LOG_MAXSIZE
 * Maximum number of lines a log file should be.  Once they reach this size,
 * dircproxy will roll the log removing lines from the front.
 *  0 = No limit
 */
#define DEFAULT_OTHER_LOG_MAXSIZE 0

/* DEFAULT_OTHER_LOG_RECALL
 * Number of lines to automatically recall on reconnection to dircproxy.
 * If this is defined, then it is also used as the default size for the
 * /DIRCPROXY RECALL command
 * -1 = All lines (not recommended if always)
 *  0 = Don't recall any
 */
#define DEFAULT_OTHER_LOG_RECALL -1

/* DEFAULT_MOTD_LOGO
 * Display a nice dircproxy logo in the message of the day.  This logo is
 * just fancy really, but its kinda nice to have it there imho.
 *  1 = Yes, display it
 *  0 = Don't display it
 */
#define DEFAULT_MOTD_LOGO 1

/* DEFAULT_MOTD_STATS
 * Display what channels you were on, and the log file sizes etc in the
 * message of the day when you reconnect.
 *  1 = Yes, display it
 *  0 = Don't display it
 */
#define DEFAULT_MOTD_STATS 1

/* functions in main.c */
extern int syscall_fail(const char *, const char *, const char *);
extern int error(const char *, ...);
extern int debug(const char *, ...);

#endif /* __DIRCPROXY_DIRCPROXY_H */