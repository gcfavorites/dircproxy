/* dircproxy
 * Copyright (C) 2000 Scott James Remnant <scott@netsplit.com>.
 * All Rights Reserved.
 *
 * dcc_net.h
 * --
 * @(#) $Id: dcc_net.h,v 1.3 2000/11/10 15:06:20 keybuk Exp $
 *
 * This file is distributed according to the GNU General Public
 * License.  For full details, read the top of 'main.c' or the
 * file called COPYING that was distributed with this code.
 */

#ifndef __DIRCPROXY_IRC_DCC_H
#define __DIRCPROXY_IRC_DCC_H

/* required includes */
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

/* a proxied dcc connection */
struct dccproxy {
  int dead;
  int type;
  time_t start;

  int sender_sock;
  int sender_status;
  struct sockaddr_in sender_addr;

  int sendee_sock;
  int sendee_status;
  struct sockaddr_in sendee_addr;

  /* DCC SEND only */
  u_int32_t bytes_sent, bytes_ackd;

  /* DCC SEND (Capture) only */
  char *cap_filename;
  FILE *cap_file;
  u_int32_t bytes_max;

  struct dccproxy *next;
};

/* types of dcc proxy */
#define DCC_CHAT             0x01
#define DCC_SEND_SIMPLE      0x10
#define DCC_SEND_FAST        0x20
#define DCC_SEND_CAPTURE     0x40
#define DCC_SEND             0x70

/* states a sender can be in */
#define DCC_SENDER_NONE      0x00
#define DCC_SENDER_CREATED   0x01
#define DCC_SENDER_CONNECTED 0x02
#define DCC_SENDER_ACTIVE    0x03

/* states a sendee can be in */
#define DCC_SENDEE_NONE      0x00
#define DCC_SENDEE_LISTENING 0x01
#define DCC_SENDEE_CONNECTED 0x02
#define DCC_SENDEE_ACTIVE    0x02
#define DCC_SENDEE_CREATED   0x03

/* functions */
extern int dccnet_new(int, long, int *, size_t, int *,
                      struct in_addr, int, const char *, long);
extern int dccnet_expunge_proxies(void);
extern void dccnet_flush(void);

#endif /* __DIRCPROXY_IRC_DCC_H */