/* vi: set sw=4 ts=4: */
/*
 * relay.c - the guts of the program.
 *
 * Copyright (C) 1998 Brad M. Garcia <garsh@home.com>
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "query.h"
#include "relay.h"
#include "cache.h"
#include "common.h"
#include "tcp.h"
#include "udp.h"
#include "master.h"
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include <netdb.h>

static time_t	send_time  = 0;
static int		send_count = 0;

static time_t	send_time2 = 0;
static int		send_count2 = 0;

/*
 * server_switch()
 *
 * Abstract: Switch to the next DNS server
 */
static void server_switch()
{
	if (serv_cnt >= 2)
	{
		serv_act = (serv_act+1) % serv_cnt;
		log_debug("Switching to DNS Server #%d", serv_act+1);
	}
	else
	{
		serv_act = 0;
	}
	send_count = 0;
}

/* server2_switch() */
static void server2_switch()
{
	if (serv2_cnt >= 2)
	{
		serv2_act = (serv2_act+1) % serv2_cnt;
		log_debug("Switching to DNS (Session2) Server #%d", serv2_act+1);
	}
	else
	{
		serv2_act = 0;
	}
	send_count2 = 0;
}

/*
 * handle_query()
 *
 * In:      fromaddrp - address of the sender of the query.
 *
 * In/Out:  msg       - the query on input, the reply on output.
 *          len       - length of the query/reply
 *
 * Out:     srvidx    - index of the server to which to forward the query
 *
 * Returns:  1 if the query should be forwarded to the srvidx server
 *           0 if msg now contains the reply
 *
 * Takes a single DNS query and determines what to do with it.
 * This is common code used for both TCP and UDP.
 *
 * Assumptions: There is only one request per message.
 */
int handle_query(const struct sockaddr_in *fromaddrp, char *msg, int *len,
		unsigned *srvidx)
{
	int       replylen;

	if (opt_debug)
	{
		char cname_buf[80];
		sprintf_cname(&msg[12], cname_buf, 80);
		log_debug("Received DNS query for \"%s\"", cname_buf);
		dump_dnspacket("query", msg, *len);
	}

#ifndef NOMASTERMODE
	/* First, check to see if we are master server */
	if ((replylen = master_lookup(msg, *len)) > 0) {
		log_debug("Replying to query as master");
		*len = replylen;
		return 0;
	}
#endif

	/* Next, see if we have the answer cached */
	if ((replylen = cache_lookup(msg, *len)) > 0) {
		log_debug("Replying to query with cached answer.");
		*len = replylen;
		return 0;
	}

#if 0
	{
		char cname_buf[80];
		sprintf_cname(&msg[12], cname_buf, 80);
		log_debug("Received DNS query for \"%s\"", cname_buf);
		printf("URL is %s\n", match_keylist(&msg[12]) ? "MATCH" : "not match");
	}
#endif

	if (match_keylist(&msg[12]))
	{
		/* If there are no servers, reply with "entry not found" */
		if (serv2_cnt == 0)
		{
			log_debug("Replying to query with \"entry not found\"");
			msg[2] |= 0x84;
			msg[3] = 0x83;
			return 0;
		}

		/* Default case. Send to a server until it "times out" */
		{
			time_t t = time(NULL);
			if (send_time2 != t)
			{
				send_time2 = t;
				if (++send_count2 > 5) server2_switch();
			}
			*srvidx = serv2_act + 10;

			dnsquery_add(fromaddrp, msg, *len);
			log_debug("Forwarding the query to DNS2 server %s", inet_ntoa(dns_srv2[*srvidx -10].addr.sin_addr));
		}
	}
	else
	{
		/* If there are no servers, reply with "entry not found" */
		if (serv_cnt == 0)
		{
			log_debug("Replying to query with \"entry not found\"");
			msg[2] |= 0x84;
			msg[3] = 0x83;
			return 0;
		}

		/* If domain names are specified, pick the correct server */
		if (dns_srv[0].domain != NULL)
		{
			dnsquery_add(fromaddrp, msg, *len);
			for (*srvidx = 0; *srvidx < serv_cnt - 1; (*srvidx)++)
			{
				if (!strcmp(dns_srv[*srvidx].domain, &msg[12] + strlen(&msg[12]) -
							strlen(dns_srv[*srvidx].domain)))
				{
					break;
				}
			}
			log_debug("Forwarding the query to DNS server %s", inet_ntoa(dns_srv[*srvidx].addr.sin_addr));
			return 1;
		}

		/* Default case.  Send to a server until it "times out". */
		{
			time_t t = time(NULL);
			if (send_time != t)
			{
				send_time = t;
				if (++send_count > 5) server_switch();
			}
			*srvidx = serv_act;

			dnsquery_add(fromaddrp, msg, *len);
			log_debug("Forwarding the query to DNS server %s", inet_ntoa(dns_srv[*srvidx].addr.sin_addr));
		}
	}
	return 1;
}

/*
 * run()
 *
 * Abstract: This function runs continuously, waiting for packets to arrive
 *           and processing them accordingly.
 */
void run()
{
	int                maxsock;
	struct timeval     tout;
	fd_set             fdmask;
	fd_set             fds;
	int                retn;
	int                i, j;

	send_time  = time(NULL);
	send_time2 = time(NULL);
	send_count = 0;
	send_count2 = 0;

	while(1)
	{
		FD_ZERO(&fdmask);
		FD_SET(isock,   &fdmask); log_debug("add fd : %d\n", isock);
		FD_SET(tcpsock, &fdmask); log_debug("add fd : %d\n", tcpsock);
		maxsock = (tcpsock > isock) ? tcpsock : isock;
		for (i = 0; i < serv_cnt; i++)
		{
			if (maxsock < dns_srv[i].sock) maxsock = dns_srv[i].sock;
			FD_SET(dns_srv[i].sock, &fdmask);
			log_debug("add srv[%d] fd : %d\n", i, dns_srv[i].sock);
		}
		for (i = 0; i < serv2_cnt; i++)
		{
			if (maxsock < dns_srv2[i].sock) maxsock = dns_srv2[i].sock;
			FD_SET(dns_srv2[i].sock, &fdmask);
			log_debug("add srv2[%d] fd : %d\n", i, dns_srv2[i].sock);
		}
		maxsock++;

		tout.tv_sec  = 60 * 5; /* five miutes */
		tout.tv_usec = 0;

		fds = fdmask;

		/* Wait for input or timeout */
		log_debug("select >>>>\n");
		retn = select(maxsock, &fds, 0, 0, &tout);
		log_debug("select <<<<\n");

		/* Expire lookups from the cache */
		cache_expire();

#ifndef NOMASTERMODE
		/* Reload the master database if neccessary */
		master_reinit();
#endif

		/* Remove old unanswered queries */
		dnsquery_timeout(60);

		/* Handle errors */
		if (retn < 0) {
			log_msg(LOG_ERR, "select returned %s", strerror(errno));
			continue;
		}
		else if (retn == 0) {
			continue;  /* nothing to do */
		}

		/* Check for replies to DNS queries */
		for (i = serv_act, j = 0; j < serv_cnt; j++) {
			if (FD_ISSET(dns_srv[i].sock, &fds)) {
				log_debug("dns_srv[%d]\n", i);
				handle_udpreply(i);
				if (i == serv_act) send_count = 0;
			}
			i = (i + 1) % serv_cnt;
		}

		/* Check for replies to DNS queries */
		for (i = serv2_act, j = 0; j < serv2_cnt; j++)
		{
			if (FD_ISSET(dns_srv2[i].sock, &fds))
			{
				log_debug("dns_srv2[%d]\n", i);
				handle_udpreply(i+10);
				if (i == serv2_act) send_count2 = 0;
			}
			i = (i+1) % serv2_cnt;
		}
		
		//++Sonnia Un-mark
		/* Check for incoming TCP requests */
		if (FD_ISSET(tcpsock, &fds))
		{
			log_debug("Check for incoming TCP requests\n");
			handle_tcprequest();
		}
		//--Sonnia
	
		/* Check for new DNS queries */
		if (FD_ISSET(isock, &fds))
		{
			log_debug("new DNS queries\n");
			handle_udprequest();
		}

	}
}
