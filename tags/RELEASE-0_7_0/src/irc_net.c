/* dircproxy
 * Copyright (C) 2000 Scott James Remnant <scott@netsplit.com>.
 * All Rights Reserved.
 *
 * irc_net.c
 *  - Polling of sockets and acting on any data
 * --
 * @(#) $Id: irc_net.c,v 1.21 2000/09/01 12:58:16 keybuk Exp $
 *
 * This file is distributed according to the GNU General Public
 * License.  For full details, read the top of 'main.c' or the
 * file called COPYING that was distributed with this code.
 */

#include <stdio.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <dircproxy.h>
#include "sock.h"
#include "dns.h"
#include "timers.h"
#include "sprintf.h"
#include "irc_log.h"
#include "irc_string.h"
#include "irc_client.h"
#include "irc_server.h"
#include "irc_net.h"

/* forward declarations */
static int _ircnet_listen(struct sockaddr_in *);
static struct ircproxy *_ircnet_newircproxy(void);
static int _ircnet_client_connected(struct ircproxy *);
static int _ircnet_acceptclient(int);
static void _ircnet_freeproxy(struct ircproxy *);
static int _ircnet_expunge_proxies(void);
static void _ircnet_rejoin(struct ircproxy *, void *);

/* list of connection classes */
struct ircconnclass *connclasses = 0;

/* whether we are a dedicated proxy or not */
static int dedicated_proxy = 0;

/* list of currently proxied connections */
static struct ircproxy *proxies = 0;

/* socket we are listening for new client connections on */
static int listen_sock = -1;

/* Create a socket to listen on. 0 = ok, other = error */
int ircnet_listen(const char *port) {
  struct sockaddr_in local_addr;

  local_addr.sin_family = AF_INET;
  local_addr.sin_addr.s_addr = INADDR_ANY;
  local_addr.sin_port = dns_portfromserv(port);

  if (!local_addr.sin_port)
    return -1;

  return _ircnet_listen(&local_addr);
}

/* Does the actual work of creating a listen socket. 0 = ok, other = error */
int _ircnet_listen(struct sockaddr_in *local_addr) {
  int this_sock;

  this_sock = sock_make();
  if (this_sock == -1)
    return -1;

  if (local_addr) {
    if (bind(this_sock, (struct sockaddr *)local_addr,
             sizeof(struct sockaddr_in))) {
      syscall_fail("bind", "listen", 0);
      sock_close(this_sock);
      return -1;
    }
  }

  if (listen(this_sock, SOMAXCONN)) {
    syscall_fail("listen", 0, 0);
    sock_close(this_sock);
    return -1;
  }

  if (listen_sock != -1) {
    debug("Closing existing listen socket %d", listen_sock);
    sock_close(listen_sock);
  }
  debug("Listening on socket %d", this_sock);
  listen_sock = this_sock;

  return 0;
}

/* Poll all the sockets for activity. Returns number that did things */
int ircnet_poll(void) {
  fd_set readset, writeset;
  struct timeval timeout;
  struct ircproxy *p;
  int nr, ns, hs;

  FD_ZERO(&readset);
  FD_ZERO(&writeset);
  nr = ns = hs = 0;

  _ircnet_expunge_proxies();

  p = proxies;
  while (p) {
    if (p->server_status & IRC_SERVER_CREATED) {
      hs = (p->server_sock > hs ? p->server_sock : hs);
      ns++;

      FD_SET(p->server_sock, &readset);
      if (!(p->server_status & IRC_SERVER_CONNECTED))
        FD_SET(p->server_sock, &writeset);
    }

    if (p->client_status & IRC_CLIENT_CONNECTED) {
      if ((p->server_status == IRC_SERVER_ACTIVE) 
          || !(p->client_status & IRC_CLIENT_AUTHED)
          || !(p->client_status & IRC_CLIENT_GOTNICK)
          || !(p->client_status & IRC_CLIENT_GOTUSER)) {
        hs = (p->client_sock > hs ? p->client_sock : hs);
        ns++;
        FD_SET(p->client_sock, &readset);
      }
    }

    p = p->next;
  }

  if (listen_sock != -1) {
    hs = (listen_sock > hs ? listen_sock : hs);
    ns++;
    FD_SET(listen_sock, &readset);
  }

  if (!ns)
    return 0;

  timeout.tv_sec = 1;
  timeout.tv_usec = 0;
  nr = select(hs + 1, &readset, &writeset, 0, &timeout);

  if (nr == -1) {
    if ((errno != EINTR) && (errno != EAGAIN)) {
      syscall_fail("select", 0, 0);
      return -1;
    } else {
      return ns;
    }
  } else if (!nr) {
    return ns;
  }

  if ((listen_sock != -1) && FD_ISSET(listen_sock, &readset))
    _ircnet_acceptclient(listen_sock);

  p = proxies;
  while (p) {
    if ((p->client_status & IRC_CLIENT_CONNECTED)
        && FD_ISSET(p->client_sock, &readset) && !p->dead)
      ircclient_data(p);

    if ((p->server_status & IRC_SERVER_CREATED) && !p->dead) {
      if (p->server_status & IRC_SERVER_CONNECTED) {
        if (FD_ISSET(p->server_sock, &readset))
          ircserver_data(p);
      } else {
        if (FD_ISSET(p->server_sock, &writeset)
            || FD_ISSET(p->server_sock, &writeset)) {
          int error, len;

          len = sizeof(int);
          if (getsockopt(p->server_sock, SOL_SOCKET,
                         SO_ERROR, &error, &len) < 0) {
            ircserver_connectfailed(p, error);
          } else if (error) {
            ircserver_connectfailed(p, error);
          } else {
            ircserver_connected(p);
          }
        }
      }
    }

    p = p->next;
  }

  return ns;
}

/* Creates a new ircproxy structure */
static struct ircproxy *_ircnet_newircproxy(void) {
  struct ircproxy *p;

  p = (struct ircproxy *)malloc(sizeof(struct ircproxy));
  memset(p, 0, sizeof(struct ircproxy));

  return p;
}

/* Finishes off the creation of an ircproxy structure */
static int _ircnet_client_connected(struct ircproxy *p) {
  p->next = proxies;
  proxies = p;

  return ircclient_connected(p);
}


/* Creates a client hooked onto a socket */
int ircnet_hooksocket(int sock) {
  struct ircproxy *p;
  int len;

  p = _ircnet_newircproxy();
  p->client_sock = sock;

  len = sizeof(struct sockaddr_in);
  if (getpeername(p->client_sock, (struct sockaddr *)&(p->client_addr), &len)) {
    syscall_fail("getpeername", "", 0);
    free(p);
    return -1;
  }

  p->die_on_close = 1;

  return _ircnet_client_connected(p);
}

/* Accept a client. 0 = okay */
static int _ircnet_acceptclient(int sock) {
  struct ircproxy *p;
  int len;

  p = _ircnet_newircproxy();

  len = sizeof(struct sockaddr_in);
  p->client_sock = accept(sock, (struct sockaddr *)&(p->client_addr), &len);
  if (p->client_sock == -1) {
    syscall_fail("accept", 0, 0);
    free(p);
    return -1;
  }

  return _ircnet_client_connected(p);
}

/* Fetch a proxy for a connection class if one exists */
struct ircproxy *ircnet_fetchclass(struct ircconnclass *class) {
  struct ircproxy *p;

  p = proxies;
  while (p) {
     if (!p->dead && (p->conn_class == class))
       return p;
     p = p->next;
  }

  return 0;
}

/* Fetch a channel from a proxy */
struct ircchannel *ircnet_fetchchannel(struct ircproxy *p, const char *name) {
  struct ircchannel *c;

  c = p->channels;
  while (c) {
    if (!irc_strcasecmp(c->name, name))
      return c;

    c = c->next;
  }

  return 0;
}

/* Add a channel to a proxy */
int ircnet_addchannel(struct ircproxy *p, const char *name) {
  struct ircchannel *c;

  if (ircnet_fetchchannel(p, name))
    return 0;

  c = (struct ircchannel *)malloc(sizeof(struct ircchannel));
  memset(c, 0, sizeof(struct ircchannel));
  c->name = x_strdup(name);
  debug("Joined channel '%s'", c->name);

  if (p->channels) {
    struct ircchannel *cc;

    cc = p->channels;
    while (cc->next)
      cc = cc->next;

    cc->next = c;
  } else {
    p->channels = c;
  }

  if (p->conn_class->chan_log_always) {
    if (irclog_open(p, c->name))
      ircclient_send_channotice(p, c->name,
                                "(warning) Unable to log channel: %s", c->name);
  }

  return 0;
}

/* Remove a channel from a proxy */
int ircnet_delchannel(struct ircproxy *p, const char *name) {
  struct ircchannel *c, *l;

  l = 0;
  c = p->channels;

  debug("Parted channel '%s'", name);

  while (c) {
    if (!irc_strcasecmp(c->name, name)) {
      if (l) {
        l->next = c->next;
      } else {
        p->channels = c->next;
      }

      ircnet_freechannel(c);
      return 0;
    } else {
      l = c;
      c = c->next;
    }
  }

  debug("    (which didn't exist)");
  return -1;
}

/* Free an ircchannel structure, returns the next */
struct ircchannel *ircnet_freechannel(struct ircchannel *chan) {
  struct ircchannel *ret;

  ret = chan->next;

  irclog_free(&(chan->log));
  free(chan->name);
  free(chan);

  return ret;
}

/* Free an ircproxy structure */
static void _ircnet_freeproxy(struct ircproxy *p) {
  debug("Freeing proxy %p", p);

  if (p->client_status & IRC_CLIENT_CONNECTED)
    ircclient_close(p);

  if (p->server_status & IRC_SERVER_CONNECTED)
    ircserver_close_sock(p);

  timer_delall(p);
  free(p->client_host);

  free(p->nickname);
  free(p->username);
  free(p->hostname);
  free(p->realname);
  free(p->servername);
  free(p->serverver);
  free(p->serverumodes);
  free(p->servercmodes);
  free(p->serverpassword);

  free(p->awaymessage);
  free(p->modes);

  if (p->channels) {
    struct ircchannel *c;

    c = p->channels;
    while (c)
      c = ircnet_freechannel(c);
  }

  irclog_free(&(p->other_log));
  irclog_closetempdir(p);
  free(p);
}

/* Get rid of any dead proxies */
static int _ircnet_expunge_proxies(void) {
  struct ircproxy *p, *l;

  l = 0;
  p = proxies;

  while (p) {
    if (p->dead) {
      struct ircproxy *n;

      n = p->next;
      _ircnet_freeproxy(p);

      if (l) {
        p = l->next = n;
      } else {
        p = proxies = n;
      }
    } else {
      l = p;
      p = p->next;
    }
  }

  return 0;
}

/* Get rid of all the proxies and connection classes */
void ircnet_flush(void) {
  ircnet_flush_proxies(&proxies);
  ircnet_flush_connclasses(&connclasses);
}

/* Get rid of all the proxies */
void ircnet_flush_proxies(struct ircproxy **p) {
  while (*p) {
    struct ircproxy *t;

    t = *p;
    *p = (*p)->next;
    _ircnet_freeproxy(t);
  }
  *p = 0;
}

/* Get rid of all the connection classes */
void ircnet_flush_connclasses(struct ircconnclass **c) {
  while (*c) {
    struct ircconnclass *t;

    t = *c;
    *c = (*c)->next;
    ircnet_freeconnclass(t);
  }
  *c = 0;
}

/* Free a connection class structure */
void ircnet_freeconnclass(struct ircconnclass *class) {
  struct strlist *s;

  free(class->server_port);
  free(class->drop_modes);
  free(class->local_address);
  free(class->away_message);
  free(class->chan_log_dir);
  free(class->other_log_dir);

  free(class->password);
  s = class->servers;
  while (s) {
    struct strlist *t;

    t = s;
    s = s->next;

    free(t->str);
    free(t);
  }

  s = class->masklist;
  while (s) {
    struct strlist *t;

    t = s;
    s = s->next;

    free(t->str);
    free(t);
  }

  free(class);
}

/* hook to rejoin a channel after a kick */
static void _ircnet_rejoin(struct ircproxy *p, void *data) {
  debug("Rejoining '%s'", (char *)data);
  ircserver_send_command(p, "JOIN", ":%s", (char *)data);
  free(data);
}

/* Set a timer to rejoin a channel */
int ircnet_rejoin(struct ircproxy *p, const char *name) {
  char *str;

  str = x_strdup(name);
  if (p->conn_class->channel_rejoin == 0) {
    _ircnet_rejoin(p, (void *)str);
  } else if (p->conn_class->channel_rejoin > 0) {
    debug("Will rejoin '%s' in %d seconds", str, p->conn_class->channel_rejoin);
    timer_new(p, 0, p->conn_class->channel_rejoin, _ircnet_rejoin, (void *)str);
  } 

  return 0;
}

/* Dedicate this proxy and create a listening socket */
int ircnet_dedicate(struct ircproxy *p) {
  struct ircconnclass *c;

  if (dedicated_proxy)
    return -1;

  /* Can't dedicate if there are multiple proxies */
  if ((p != proxies) || p->next)
    return -1;

  debug("Dedicating proxy to %p", p);

  if (_ircnet_listen(0))
    return -1;

  c = connclasses;
  while (c) {
    if (c == p->conn_class) {
      c = c->next;
    } else {
      struct ircconnclass *t;
      
      t = c;
      c = c->next;
      ircnet_freeconnclass(t);
    }
  }
  connclasses = p->conn_class;
  connclasses->next = 0;

  /* Okay we're dedicated */
  dedicated_proxy = 1;
  ircnet_announce_dedicated(p);
  
  return 0;
}

/* send the dedicated listening port to the user */
int ircnet_announce_dedicated(struct ircproxy *p) {
  struct sockaddr_in local_addr, listen_addr;
  unsigned short int port;
  char *hostname;
  int len;

  if (!IS_CLIENT_READY(p))
    return -1;

  hostname = 0;
  port = 0;

  len = sizeof(struct sockaddr_in);
  if (!getsockname(p->client_sock, (struct sockaddr *)&local_addr, &len)) {
    if (local_addr.sin_addr.s_addr)
      hostname = dns_hostfromaddr(local_addr.sin_addr);
  } else {
    syscall_fail("getsockname", "p->client_sock", 0);
  }

  len = sizeof(struct sockaddr_in);
  if (!getsockname(listen_sock, (struct sockaddr *)&listen_addr, &len)) {
    port = ntohs(listen_addr.sin_port);
  } else {
    syscall_fail("getsockname", "listen_sock", 0);
    free(hostname);
    return -1;
  }

  ircclient_send_notice(p, "Reconnect to this session at %s:%d",
                        (hostname ? hostname : p->hostname), port);
  free(hostname);

  return 0;
}

/* tell the client they can't reconnect */
int ircnet_announce_nolisten(struct ircproxy *p) {
  if (!IS_CLIENT_READY(p))
    return -1;

  ircclient_send_notice(p, "You cannot reconnect to this session");

  return 0;
}

/* tell the client whether we're dedicated or not listening */
int ircnet_announce_status(struct ircproxy *p) {
  if (p->die_on_close) {
    return ircnet_announce_nolisten(p);
  } else if (dedicated_proxy) {
    return ircnet_announce_dedicated(p);
  } else {
    return 0;
  }
}