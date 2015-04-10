/* vi: set sw=4 ts=4: */
/*
 *   Copyright 1996, Michiel Boland.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Mysterons */

static const char rcsid[] = "$Id: request.c,v 1.1.1.1 2005/05/19 10:53:06 r01122 Exp $";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <fcntl.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include "mathopd.h"
/*------------------------------*/
/* papa add start for syslog 2007/03/30 */
#include <syslog.h>
#include "asyslog.h"
#include "elbox_config.h"
#include "rgdb_getset.h"

/* ********************************* */

/*------------------------------*/
/* Added by Paul Liu 20050905 */
/* Add support for session control.*/
#include "web-session.h"
/*------------------------------*/


static const char m_get[] =				"GET";
static const char m_head[] =			"HEAD";
static const char m_post[] =			"POST";
/*------------------------------*/
/* Modified by Paul Liu 20061003 */
/* Add support for UPNP methods */
static const char m_msearch[] =			"M-SEARCH";
static const char m_subscribe[] =		"SUBSCRIBE";
static const char m_unsubscribe[] =		"UNSUBSCRIBE";
static const char m_notify[] =			"NOTIFY";
/*------------------------------*/

static time_t timerfc(const char *s)
{
	static const int daytab[2][12] = {
		{ 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 },
		{ 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 }
	};
	unsigned sec, min, hour, day, mon, year;
	char month[3];
	int c;
	unsigned n;
	char flag;
	char state;
	char isctime;
	enum { D_START, D_END, D_MON, D_DAY, D_YEAR, D_HOUR, D_MIN, D_SEC };

	sec = 60;
	min = 60;
	hour = 24;
	day = 32;
	year = 1969;
	isctime = 0;
	month[0] = 0;
	state = D_START;
	n = 0;
	flag = 1;
	do {
		c = *s++;
		switch (state) {
		case D_START:
			if (c == ' ') {
				state = D_MON;
				isctime = 1;
			} else if (c == ',')
				state = D_DAY;
			break;
		case D_MON:
			if (isalpha(c)) {
				if (n < 3)
					month[n++] = c;
			} else {
				if (n < 3)
					return -1;
				n = 0;
				state = isctime ? D_DAY : D_YEAR;
			}
			break;
		case D_DAY:
			if (c == ' ' && flag)
				;
			else if (isdigit(c)) {
				flag = 0;
				n = 10 * n + (c - '0');
			} else {
				day = n;
				n = 0;
				state = isctime ? D_HOUR : D_MON;
			}
			break;
		case D_YEAR:
			if (isdigit(c))
				n = 10 * n + (c - '0');
			else {
				year = n;
				n = 0;
				state = isctime ? D_END : D_HOUR;
			}
			break;
		case D_HOUR:
			if (isdigit(c))
				n = 10 * n + (c - '0');
			else {
				hour = n;
				n = 0;
				state = D_MIN;
			}
			break;
		case D_MIN:
			if (isdigit(c))
				n = 10 * n + (c - '0');
			else {
				min = n;
				n = 0;
				state = D_SEC;
			}
			break;
		case D_SEC:
			if (isdigit(c))
				n = 10 * n + (c - '0');
			else {
				sec = n;
				n = 0;
				state = isctime ? D_YEAR : D_END;
			}
			break;
		}
	} while (state != D_END && c);
	switch (month[0]) {
	case 'A':
		mon = (month[1] == 'p') ? 4 : 8;
		break;
	case 'D':
		mon = 12;
		break;
	case 'F':
		mon = 2;
		break;
	case 'J':
		mon = (month[1] == 'a') ? 1 : ((month[2] == 'l') ? 7 : 6);
		break;
	case 'M':
		mon = (month[2] == 'r') ? 3 : 5;
		break;
	case 'N':
		mon = 11;
		break;
	case 'O':
		mon = 10;
		break;
	case 'S':
		mon = 9;
		break;
	default:
		return -1;
	}
	if (year <= 100)
		year += (year < 70) ? 2000 : 1900;
	--mon;
	--day;
	if (sec >= 60 || min >= 60 || hour >= 60 || day >= 31)
		return -1;
	if (year < 1970)
		return 0;
	return sec + 60L * (min + 60L * (hour + 24L * ( day +
	    daytab[year % 4 == 0 && (year % 100 || year % 400 == 0)][mon] +
	    365L * (year - 1970L) + ((year - 1969L) >> 2))));
}

char *rfctime(time_t t, char *buf)
{
	struct tm *tp;

	tp = gmtime(&t);
	if (tp == 0) {
		log_d("gmtime failed!?!?!?");
		strcpy(buf, "?");
		return buf;
	}
	strftime(buf, 31, "%a, %d %b %Y %H:%M:%S GMT", tp);
	return buf;
}

static char *getline(struct pool *p, int fold)
{
	char *s, *olds, *sp, *end;
	int f;

	end = p->middle;
	s = p->start;
	if (s >= end)
		return 0;
	olds = s;
	sp = s;
	f = 0;
	while (s < end) {
		switch (*s++) {
		case '\n':
			if (s == end || fold == 0 || (*s != ' ' && *s != '\t')) {
				if (f)
					*sp = 0;
				else
					s[-1] = 0;
				p->start = s;
				return olds;
			}
		case '\r':
		case '\t':
			s[-1] = ' ';
		case ' ':
			if (f == 0) {
				f = 1;
				sp = s - 1;
			}
			break;
		default:
			f = 0;
			break;
		}
	}
	log_d("getline: fallen off the end");
	return 0;
}

static char *dirmatch(char *s, char *t)
{
	size_t n;

	n = strlen(t);
	if (n == 0)
		return s;
	return !strncmp(s, t, n) && (s[n] == '/' || s[n] == 0 || s[n - 1] == '~') ? s + n : 0;
}

static char *exactmatch(char *s, char *t)
{
	size_t n;

	n = strlen(t);
	return !strncmp(s, t, n) && s[n] == '/' && s[n + 1] == 0 ? s + n : 0;
}

static int evaluate_access(unsigned long ip, struct access *a)
{
	while (a && ((ip & a->mask) != a->addr))
		a = a->next;
	return a ? a->type : ALLOW;
}

static int get_mime(struct request *r, const char *s)
{
	struct mime *m;
	char *saved_type;
	int saved_class;
	int l, le, lm, n;

	saved_type = 0;
	saved_class = 0;
	lm = 0;
	l = strlen(s);
	m = r->c->mimes;
	while (m) {
		if (m->ext) {
			le = strlen(m->ext);
			/*------------------------------*/
			/* Added by Paul Liu 20050922	*/
			/* Add support for HTTP header check */
			if (!strncasecmp(m->ext, "h:", 2)) {
				for (n=0; n < r->nheaders; n++)
					if(r->headers[n].rh_name && !strcasecmp(r->headers[n].rh_name, m->ext+2)) {
						saved_type = m->name;
						saved_class = m->class;
						break;
					}
				if(saved_type != 0) //-----Break when a header match, don't care ext file
					break;
			} else if (le > lm && le <= l && !strcasecmp(s + l - le, m->ext)) {
				lm = le;
				saved_type = m->name;
				saved_class = m->class;
			}
			/*------------------------------*/
		} else if (saved_type == 0) {
			saved_type = m->name;
			saved_class = m->class;
		}
		m = m->next;
	}
	if (saved_type) {
		if (debug)
			log_d("get_mime: type=%s, class=%d", saved_type, saved_class);
		r->content_type = saved_type;
		r->class = saved_class;
		r->num_content = lm;
		return 0;
	}
	return -1;
}

static void close_rfd(struct request *r)
{
	if (r->cn->rfd == -1)
		return;
	if (debug)
		log_d("close_rfd: %d", r->cn->rfd);
	close(r->cn->rfd);
	r->cn->rfd = -1;
}

static int assign_rfd(struct request *r, int fd)
{
	if (r->cn->rfd != -1)
		log_d("assign_rfd: rfd already assigned!?!?");
	fcntl(fd, F_SETFD, FD_CLOEXEC);
	if (fstat(fd, &r->finfo) == -1) {
		lerror("fstat");
		return -1;
	}
	close_rfd(r);
	r->cn->rfd = fd;
	return 0;
}

static int get_path_info(struct request *r)
{
	char *p, *pa, *end, *cp, *start, *cds;
	int fd, first;
	size_t m;
#if ELBOX_PROGS_GPL_SYSLOGD_AP 
        int lan_connection_type;
	char    Val_ac_auth[32],Val_ip[32],Val_AP_ip[32],lan_connection[2];
	char check_login[2],check_login_1[2],check_login_num[2];
	char *https_client_ip = NULL;
#endif
	m = r->location_length;
	if (m == 0)
		return -1;
	p = r->path_translated;
	start = p + m;
	end = p + strlen(p);
	pa = r->path_args;
	*pa = 0;
	cp = end;
	first = 0;
	while (cp >= start && cp[-1] == '/')
		--cp;
	while (cp >= start) {
		if (cp != end)
			*cp = 0;
		fd = open(p, O_RDONLY | O_NONBLOCK);
 		if (debug)
 			log_d("get_path_info: open(\"%s\") = %d", p, fd);
 
 /* papa add start for syslog 2007/03/30 */
#if ELBOX_PROGS_GPL_SYSLOGD_AP 
 		cli_get_val(Val_ac_auth, sizeof(Val_ac_auth), "var/proc/web/session:%d/user/ac_auth",r->sessionid);
		cli_get_val(Val_ip, sizeof(Val_ip), "var/proc/web/session:%d/ip",r->sessionid);
		RGDBGET(lan_connection,2, "/wan/rg/inf:1/mode");
		lan_connection_type = atoi(lan_connection);
		if(lan_connection_type ==1)
			RGDBGET(Val_AP_ip,32, "/wan/rg/inf:1/static/ip");
		else if(lan_connection_type ==2)
			RGDBGET(Val_AP_ip,32, "/wan/rg/inf:1/dhcp/sys/hostname");

		webprintf("Val_AP_ip=%s, Val_ip=%s\n",Val_AP_ip, Val_ip);
		/* added/modified by Leo, 2007/05/08 11:58:28 */
		if((https_client_ip = (char *)calloc(1,32)) == NULL)
 			perror("Mathopd allocate memory failed!");
 		else
		{
 			RGDBGET(https_client_ip,32, "/runtime/stunnel/clientip");
	 		/*
	 		if(strlen(https_client_ip) != 0 && lan_connection_type == 2)
	 		{
	 			int len = strlen(https_client_ip) - strlen(strchr(https_client_ip, ':'));
	 			webprintf("length=%d\n", len);
				snprintf(https_client_ip, len+1, "%s", https_client_ip);
				webprintf("https_client_ip=%s\n",https_client_ip);
			}
			*/
		}
#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
				
		RGDBGET(check_login,2, "/runtime/sys/web/login/check");
		RGDBGET(check_login_num,2, "/runtime/sys/web/login/check_num");
		if((!strcasecmp("1", Val_ac_auth)) && (strcasecmp("1", check_login)) == 0) //after login success, reload web page.
		{
			RGDBSET("/runtime/sys/web/login/check","0");
		}
		else if((strcasecmp("1", Val_ac_auth)) && (strcasecmp("1", check_login_num) == 0)) // close web page then login
		{
			RGDBSET("/runtime/sys/web/login/check_num","0");			
		}
		else if((!strcasecmp("1", Val_ac_auth)) && (strcasecmp("1", check_login) != 0) && (strcasecmp("1", check_login_num) != 0))
		{
			RGDBSET("/runtime/sys/web/login/check","1");
			RGDBSET("/runtime/sys/web/login/check_num","1");
		}

#endif	
		if((!strcasecmp("/www/index.php", p))&&(!strcasecmp("1", Val_ac_auth)))
		{
#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
			RGDBGET(check_login_1,2, "/runtime/sys/web/login/check");	
		
			if(strcasecmp(check_login_1, "1") == 0)
			{
#endif		
				if(lan_connection_type == 1)
				{
	 			/* static ip */
	 				if(strcmp("127.0.0.1", Val_ip) == 0 )
	 				{
	 				/* use https */
	#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
	 					syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web login success from %s with https", https_client_ip);
	#endif
	 				}
	 				else
	 				{
	 				/* use http */
	#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
	 			syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web login success from %s",Val_ip);						
	#endif
	 				}
	 			}
	 			else
	 			{
	 			/* dynamic ip */
	 				if(strcmp("127.0.0.1", Val_ip) == 0)
	 				{
	 				/* use https */
	#if ELBOX_PROGS_GPL_SYSLOGD_AP 	 				
	 				syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web login success from %s with https",https_client_ip);
	#endif
	 				}
	 				else
	 				{
	 				/* use http */
	#if ELBOX_PROGS_GPL_SYSLOGD_AP 	 				
	 					syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web login success from %s", Val_ip);
	#endif
					}
				} 
#if ELBOX_PROGS_GPL_SYSLOGD_AP 					
			}
#endif		
		}	
	
		if((!strcasecmp("/www/login_fail.php", p))&&(Val_ac_auth!="1"))
		{
			if(lan_connection_type == 1)
			{
 			/* static ip */
 				if(strcmp("127.0.0.1", Val_ip) == 0 )
 				{
 				/* use https */
#if ELBOX_PROGS_GPL_SYSLOGD_AP
 					syslog(ALOG_AP_SYSACT|LOG_WARNING,"[SYSACT]Web login failure from %s with https", https_client_ip);
#endif
 				}
 				else
 				{
 				/* use http */
#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
 			syslog(ALOG_AP_SYSACT|LOG_WARNING,"[SYSACT]Web login failure from %s",Val_ip);						
#endif
				}
#ifdef ELBOX_PROGS_GPL_NET_SNMP
sendtrap("[SNMP-TRAP][Specific=2]");
#endif
            	}
 			else
			{
 			/* dynamic ip */
 				if(strcmp("127.0.0.1", Val_ip) == 0)
 				{
 				/* use https */
#if ELBOX_PROGS_GPL_SYSLOGD_AP  				
 				syslog(ALOG_AP_SYSACT|LOG_WARNING,"[SYSACT]Web login failure from %s with https", https_client_ip);
#endif
 				}
 				else
 				{
 				/* use http */
#if ELBOX_PROGS_GPL_SYSLOGD_AP  				
 					syslog(ALOG_AP_SYSACT|LOG_WARNING,"[SYSACT]Web login failure from %s",Val_ip);
#endif
				}
#ifdef ELBOX_PROGS_GPL_NET_SNMP
sendtrap("[SNMP-TRAP][Specific=2]");
#endif			
			}
		} 		
		
		if((!strcasecmp("/www/logout.php", p))&&(Val_ac_auth!="1"))
		{			
			if(lan_connection_type == 1)
			{
 			/* static ip */
 				if(strcmp("127.0.0.1", Val_ip) == 0 )
 				{
 				/* use https */
#if ELBOX_PROGS_GPL_SYSLOGD_AP
 					syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web logout from %s with https", https_client_ip);
#endif
				}
 				else
				{
 				/* use http */
#if ELBOX_PROGS_GPL_SYSLOGD_AP 				
 			syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web logout from %s",Val_ip);						
#endif
				}
#ifdef ELBOX_PROGS_GPL_NET_SNMP
sendtrap("[SNMP-TRAP][Specific=11]");
#endif
 			}
 			else
 			{
 			/* dynamic ip */
 				if(strcmp("127.0.0.1", Val_ip) == 0)
 				{
 				/* use https */
#if ELBOX_PROGS_GPL_SYSLOGD_AP 	 				
 				syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web logout from %s with https", https_client_ip);
#endif
 				}
 				else
 				{
 				/* use http */
#if ELBOX_PROGS_GPL_SYSLOGD_AP 	 				
 					syslog(ALOG_AP_SYSACT|LOG_NOTICE,"[SYSACT]Web logout from %s",Val_ip);
#endif
				}
#ifdef ELBOX_PROGS_GPL_NET_SNMP
sendtrap("[SNMP-TRAP][Specific=11]");
#endif			
			}		
#if ELBOX_PROGS_GPL_SYSLOGD_AP 		
RGDBSET("/runtime/sys/web/login/check_num","0");
#endif					
}		
		free(https_client_ip);
		/* ********************************* */
#endif
 /* papa add end for syslog 2007/03/30 */
		if (fd == -1)
			switch (errno) {
			case ENOENT:
				/*------------------------------*/
				/* Added by Paul Liu 20040514	*/
				/* Return 0 for none exists file for Internal Call */
				if (r->class != CLASS_FILE) return 0;
				/*------------------------------*/
			case ENOTDIR:
				//break;
			default:
				log_d("cannot open %s", p);
				lerror("open");
				return -1;
			}
		else {
			if (assign_rfd(r, fd) == -1) {
				close(fd);
				return -1;
			}
			if (r->curdir[0] == 0) {
				first = 1;
				strcpy(r->curdir, p);
			}
		}
		if (cp != end)
			*cp = '/';
		if (fd != -1) {
			strcpy(pa, cp);
			if (S_ISDIR(r->finfo.st_mode))
				*cp++ = '/';
			else if (first) {
				cds = strrchr(r->curdir, '/');
				if (cds)
					*cds = 0;
			}
			*cp = 0;
			if (debug)
				log_d("get_path_info: curdir = \"%s\"", r->curdir);
			return 0;
		} else if (r->c->path_info_ok == 0)
			return -1;
		while (--cp >= start && *cp != '/')
			;
	}
	return -1;
}

static void sanitize_path(struct request *r)
{
	char *p, *q, c;
	enum {
		sp_normal,
		sp_slash,
		sp_slashdot,
		sp_slashdotdot
	} s;

	if (debug)
		log_d("sanitize_path: old path: %s", r->path);
	p = q = r->path;
	s = sp_normal;
	do {
		c = *p++;
		switch (s) {
		case sp_normal:
			if (c == '/')
				s = sp_slash;
			break;
		case sp_slash:
			switch (c) {
			case '/':
				/*
				 * replace '//' with '/'
				 */
				--q;
				break;
			case '.':
				s = sp_slashdot;
				break;
			default:
				s = sp_normal;
				break;
			}
			break;
		case sp_slashdot:
			switch (c) {
			case '/':
				/*
				 * replace '/./' with '/'
				 */
				q -= 2;
				s = sp_slash;
				break;
			case '.':
				s = sp_slashdotdot;
				break;
			default:
				s = sp_normal;
				break;
			}
			break;
		case sp_slashdotdot:
			switch (c) {
			case '/':
				/*
				 * replace '/foo/../' with '/'
				 */
				q -= 3;
				while (q > r->path && q[-1] != '/')
					--q;
				if (q > r->path)
					--q;
				s = sp_slash;
				break;
			default:
				s = sp_normal;
				break;
			}
			break;
		}
		*q++ = c;
	} while (c);
	if (debug)
		log_d("sanitize_path: new path: %s", r->path);
}

static int check_path(struct request *r)
{
	char *p;
	char c;
	enum {
		s_normal,
		s_slash,
		s_slashdot,
		s_slashdotdot,
		s_forbidden
	} s;

	p = r->path;
	s = s_normal;
	do {
		c = *p++;
		switch (s) {
		case s_normal:
			if (c == '/')
				s = s_slash;
			break;
		case s_slash:
			if (c == '/')
				s = s_forbidden;
			else if (c == '.')
				s = r->c->allow_dotfiles ? s_slashdot : s_forbidden;
			else
				s = s_normal;
			break;
		case s_slashdot:
			if (c == 0 || c == '/')
				s = s_forbidden;
			else if (c == '.')
				s = s_slashdotdot;
			else
				s = s_normal;
			break;
		case s_slashdotdot:
			if (c == 0 || c == '/')
				s = s_forbidden;
			else
				s = s_normal;
			break;
		case s_forbidden:
			c = 0;
			break;
		}
	} while (c);
	return s == s_forbidden ? -1 : 0;
}

static int makedir(struct request *r)
{
	int l;

	if (r->args)
		l = snprintf(r->newloc, PATHLEN, "%s/?%s", r->url, r->args);
	else
		l = snprintf(r->newloc, PATHLEN, "%s/", r->url);
	if (l >= PATHLEN) {
		log_d("makedir: url too large");
		r->status = 500;
		return 0;
	}
	if (debug)
		log_d("makedir: redirecting to %s", r->newloc);
	r->location = r->newloc;
	r->status = 302;
	return 0;
}

static int append_indexes(struct request *r)
{
	char *p;
	struct simple_list *i;
	int fd;
	size_t l, n;

	p = r->path_translated;
	l = strlen(p);
	r->isindex = 1;
	for (i = r->c->index_names; i; i = i->next) {
		n = strlen(i->name);
		if (l + n >= PATHLEN - 1)
			continue;
		memcpy(p + l, i->name, n);
		p[l + n] = 0;
		fd = open(p, O_RDONLY | O_NONBLOCK);
		if (debug)
			log_d("append_indexes: open(\"%s\") = %d", p, fd);
		if (fd == -1) {
			if (errno != ENOENT) {
				log_d("append_indexes: cannot open %s", p);
				lerror("open");
				p[l] = 0;
				r->error_file = r->c->error_404_file;
				r->status = 404;
				return -1;
			}
		} else {
			if (assign_rfd(r, fd) == -1) {
				close(fd);
				return -1;
			}
			break;
		}
	}
	if (i == 0) {
		p[l] = 0;
		return -1;
	}
	return 0;
}

static int process_external(struct request *r)
{
	r->num_content = -1;
	return process_cgi(r);
}

static int process_special(struct request *r)
{
	const char *ct;

	ct = r->content_type;
	r->num_content = -1;
	if (!strcasecmp(ct, CGI_MAGIC_TYPE)) return process_cgi(r);
	if (r->status) return 0;

	/*------------------------------*/
	/* Added by Paul Liu 20040422		*/
	/* Add internal function call	*/
	if(!strcasecmp(ct, INTERNAL_MAGIC_TYPE)) return process_internal(r);
	/*------------------------------*/

	if (r->method == M_POST)
	{
		if (debug) log_d("POST to specialty rejected");
		r->status = 405;
		return 0;
	}

	/*------------------------------*/
	/* Added by Paul Liu 20040326 */
	/* Add condition compile for IMAP/REDIRECT/DUMP */
#ifdef IMAP
	if (!strcasecmp(ct, IMAP_MAGIC_TYPE)) return process_imap(r);
#endif
#ifdef REDIRECT
	if (!strcasecmp(ct, REDIRECT_MAGIC_TYPE)) return process_redirect(r);
#endif
#ifdef DUMP
	if (!strcasecmp(ct, DUMP_MAGIC_TYPE)) return process_dump(r);
#endif
	/*------------------------------*/

	if (debug) log_d("unknown specialty");
	r->error_file = r->c->error_404_file;
	r->status = 404;
	return 0;
}

static int satisfy_range(struct request *r)
{
	r->range_total = r->content_length;
	switch(r->range) {
	case -1:
		if (r->range_suffix == 0)
			return -1;
		r->range_ceiling = r->range_total - 1;
		if (r->range_suffix < r->range_total)
			r->range_floor = r->range_total - r->range_suffix;
		else
			r->range_floor = 0;
		break;
	case 1:
		if (r->range_floor >= r->range_total)
			return -1;
		r->range_ceiling = r->range_total - 1;
		break;
	case 2:
		if (r->range_floor >= r->range_total)
			return -1;
		if (r->range_ceiling >= r->range_total)
			r->range_ceiling = r->range_total - 1;
		break;
	}
	if (r->range_floor == 0 && r->range_ceiling == r->range_total - 1) {
		r->range = 0;
		return 0;
	}
	if (r->if_range_s && r->last_modified > r->if_range) {
		r->range = 0;
		return 0;
	}
	return 0;
}

static int process_fd(struct request *r)
{
	if (r->path_args[0] && r->c->path_args_ok == 0 && (r->path_args[1] || r->isindex == 0)) {
		close_rfd(r);
		if (debug)
			log_d("process_fd: path_args is not empty");
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return 1;
	}
	if (r->method == M_POST) {
		close_rfd(r);
		if (debug)
			log_d("POST to file rejected");
		r->status = 405;
		return 0;
	}
	r->content_length = r->finfo.st_size;
	r->cn->file_offset = 0;
	if (r->status == 0) {
		r->last_modified = r->finfo.st_mtime;
		if (r->last_modified > current_time) {
			sync_time();
			if (r->last_modified > current_time) {
				log_d("file %s has modification time in the future", r->path_translated);
				r->last_modified = current_time;
			}
		}

		/*------------------------------*/
		/* Added by Paul Liu 20040326 */
		/* Remove "file not modified" status */
#if 0
		if (r->last_modified <= r->ims) {
			close_rfd(r);
			r->num_content = -1;
			if (debug)
				log_d("file not modified (%ld <= %ld)", (long) r->last_modified, (long) r->ims);
			r->status = 304;
			return 0;
		}
#endif
		/*------------------------------*/

		if (r->ius && r->last_modified > r->ius) {
			close_rfd(r);
			if (debug)
				log_d("file modified (%ld > %ld)", (long) r->last_modified, (long) r->ius);
			r->status = 412;
			return 0;
		}
		if (r->range == 0)
			r->status = 200;
		else {
			if (satisfy_range(r) == -1) {
				close_rfd(r);
				if (debug)
					log_d("satisfy_range failed");
				r->status = 416;
				return 0;
			}
			if (r->range) {
				if (r->range_floor)
					r->cn->file_offset = r->range_floor;
				r->content_length = r->range_ceiling - r->range_floor + 1;
				if (debug)
					log_d("returning partial content");
				r->status = 206;
			} else {
				if (debug)
					log_d("range covered entire file");
				r->status = 200;
			}
		}
	}
	if (r->method != M_GET)
		close_rfd(r);
	return 0;
}

static int find_vs(struct request *r)
{
	struct virtual *v, *d;

	d = 0;
	v = r->cn->s->children;
	if (r->host)
		while (v) {
			if (v->host) {
				if (strcmp(r->host, v->host) == 0)
					break;
			} else if (v->anyhost)
				d = v;
			v = v->next;
		}
	else
		while (v) {
			if (v->host == 0 && v->anyhost == 0)
				break;
			v = v->next;
		}
	if (v == 0) {
		if (d == 0)
			return -1;
		v = d;
	}
	r->vs = v;
	return 0;
}

#if 0
/*------------------------------*/
/* Modified by Paul Liu 20050802 */
/* Add support for ignore files */
static int ignorefile(struct request *r)
{
	int  i, rlt=-1;
	FILE *pFile;
	char pathbuf[255], pathname[265];
	char *pName, *chkfile[2]={".ignore", ".require"};
	strcpy(pathbuf, r->path_translated);
	pName=strrchr(pathbuf, '/');
	*pName++=0;

	for(i=0; i<2; i++)
	{
		sprintf(pathname, "%s/%s", pathbuf, chkfile[i]);
		if((pFile=fopen(pathname, "r")))    //-----empty file or "ALL" means all files
		{
			rlt=(i?0:-1);
			while(EOF!=fscanf(pFile, "%s", pathname))
			{
				if(!strcasecmp("ALL", pathname) || !strcmp(pathname, pName))
				{
					rlt=(i?-1:0);
					break;
				}
			}
			fclose(pFile);
		}
	}
	return rlt;
}
#endif
/*------------------------------*/

static int check_realm(struct request *r)
{
	char *a;

	if (r == 0 || r->c == 0 || r->c->realm == 0 || r->c->userfile == 0)
		return 0;
	a = r->authorization;
	if (a == 0)
	{
		/* ++ modify by david_hsieh@alphanetworks.com */
		if (r->c->session_control)
		{
			r->user[0] = '\0';
			r->passwd[0] = '\0';
			return 0;
		}
		/* -- modify by david_hsieh@alphanetworks.com */

		return -1;
	}
	if (strncasecmp(a, "basic ", 6))
		return -1;
	a += 6;
	while (*a == ' ')
		++a;

	/* ++ modify by david_hsieh@alphanetworks.com */
	if (r->c->session_control)
	{
		r->user[0] = '\0';
		r->passwd[0] = '\0';
		get_webuser(a, r->user, sizeof r->user, r->passwd, sizeof r->passwd, r->c->do_crypt);
		return 0;
	}
	/* -- modify by david_hsieh@alphanetworks.com */

	/*------------------------------*/
	/* Modified by Paul Liu 20050614 */
	/* Add support for user password Env.*/
	if (webuserok(a, r->c->userfile, r->user, sizeof r->user, r->passwd, sizeof r->passwd, r->c->do_crypt))
		return 0;
	/*------------------------------*/
	return -1;
}

static size_t expand_hostname(char *dest, const char *source, const char *host, int m)
{
	int c, l, n;

	if (m <= 0) /* should never happen */
		return 0;
	n = m;
	do {
		c = *source++;
		switch (c) {
		case '*':
			dest += l = sprintf(dest, "%.*s", n, host);
			n -= l;
			break;
		default:
			*dest++ = c;
			--n;
			break;
		case 0:
			*dest = 0;
			break;
		}
	} while (n && c);
	return m - n;
}

/*------------------------------*/
/* Added by Paul Liu 20060317 */
/* Add support for Alias file which allow multi-alias in a control block . */
static char *parse_alias_file(char *file, char *alias, char *location, int maxlen, int exact_match)
{
	FILE *pFile=NULL;
	if((pFile=fopen(file, "r")))
	{
		int		c, idx=0, state=1;
		char	*pVal=NULL, *s=NULL;
		char	buf[512];
		while(state && idx<sizeof(buf))
		{
			if(EOF==(c=fgetc(pFile)) && state!=4)
				break;
			switch(state)
			{
				case 1:
					if(c==' ' || !isspace(c))
					{
						state=2;
						buf[idx++]=c;
					}
					break;

				case 2:
					switch(c)
					{
						case ':':
						case ',':
						case ';':
							buf[idx++]=0;
							pVal=buf+idx;
							state=3;
							break;

						case '\r':
						case '\n':
							buf[0]=idx=0;
							pVal=NULL;
							state=1;
							break;

						default:
							buf[idx++]=c;
							break;
					}
					break;

				case 3:
					if(!isspace(c))
					{
						buf[idx++]=c;
						state=4;
					}
					break;

				case 4:
					switch(c)
					{
						case '\r':
						case '\n':
						case EOF:
							buf[idx]=0;
							s = exact_match ? exactmatch(alias, buf) : dirmatch(alias, buf);
							if(s)
							{
								strncpy(location, pVal, maxlen);
								return s;
							}
							else
							{
								buf[0]=idx=0;
								pVal=0;
								state=1;
							}
							break;

						default:
							buf[idx++]=c;
							break;
					}
					break;
			}
		}
		fclose(pFile);
	}
	return 0;
}
/*------------------------------*/

struct control *faketoreal(char *x, char *y, struct request *r, int update, int maxlen)
{
	struct control *c;
	char *s, *t;
	char buf[256];
	char *location;
	struct passwd *p;
	int l;

	if (r->vs == 0)
	{
		log_d("virtualhost not initialized!");
		return 0;
	}
	c = r->vs->vserver->controls;
	while (c)
	{
		/*------------------------------*/
		/* Added/Modified by Paul Liu 20060317 */
		/* Add support for Alias file which allow multi-alias in a control block . */
		s=0;
		location=0;
		if(c->alias_file)
		{
			s=parse_alias_file(c->alias_file, x, buf, sizeof(buf), c->exact_match);
			location=buf;
		}
		if(!s && c->locations && c->alias)
		{
			s = c->exact_match ? exactmatch(x, c->alias) : dirmatch(x, c->alias);
			location=c->locations->name;
		}

		if (s && (c->clients == 0 || evaluate_access(r->cn->peer.sin_addr.s_addr, c->clients) == APPLY))
		{
			if (c->user_directory == 0)
			{
				if (r->host)
					l = expand_hostname(y, location, r->host, maxlen - 1);
				else
					l = sprintf(y, "%.*s", maxlen - 1, location);
				r->location_length = l;
				if (location[0] == '/' || !c->path_args_ok)
					sprintf(y + l, "%.*s", maxlen - (l + 1), s);
				break;
			}
			else
			{
				if (*s == '/')
					++s;
				t = strchr(s, '/');
				if (t)
					*t = 0;
				p = getpwnam(s);
				if (t)
					*t = '/';
				if (p && p->pw_dir)
				{
					l = strlen(p->pw_dir);
					if (l + 2 > maxlen)
					{
						log_d("overflow in faketoreal");
						return 0;
					}
					l = sprintf(y, "%s/%.*s", p->pw_dir, maxlen - (l + 2), location); 
					r->location_length = l;
					maxlen -= l + 1;
					if (t && (location[0] == '/' || !c->path_args_ok))
						sprintf(y + l, "%.*s", maxlen, t);
					break;
				}
			}
		}
		/*------------------------------*/
		c = c->next;
	}
	if (c && update)
		c->locations = c->locations->next;
	return c;
}

static void process_path(struct request *r)
{
	/*------------------------------*/
	/* Modified by Paul Liu 20050802 */
	/* Add support for session control.*/
	const char *pStr=NULL;

	/* Added by Paul Liu 20050913 */
	/* Add file filter for session control. */
	struct simple_list *i;
	/*------------------------------*/

	if (find_vs(r) == -1) {
		if (debug)
			log_d("find_vs failed (host=%s)", r->host ? r->host : "[not set]");
		r->status = 400;
		return;
	}
	if (r->vs->vserver->controls && r->vs->vserver->controls->sanitize_path)
		sanitize_path(r);
	if ((r->c = faketoreal(r->path, r->path_translated, r, 1, sizeof r->path_translated)) == 0) {
		if (debug)
			log_d("faketoreal failed");
		r->status = 500;
		return;
	}
	if (check_path(r) == -1) {
		if (debug)
			log_d("check_path failed for %s", r->path);
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return;
	}
	if (r->c->accesses && evaluate_access(r->cn->peer.sin_addr.s_addr, r->c->accesses) == DENY) {
		if (debug)
			log_d("access denied");
		r->error_file = r->c->error_403_file;
		r->status = 403;
		return;
	}

	/*------------------------------*/
	/* Modified by Paul Liu 20050802 */
	/* Add support for session control.*/
	/* Add support for ignore files */
	if((pStr=strrchr(r->path_translated, '.')))
		pStr++;
	if (r->c->session_control)
	{
		/* Added by Paul Liu 20050913 */
		/* Add file filter for session control. */
		for (i = r->c->session_filter; i; i = i->next)
		{
			if(pStr && !strcmp(i->name, pStr))
			{
				r->sessionid=get_session_id(r);
				break;
			}
		}
	}

#if 0
	if (ignorefile(r)==0) return;
#endif

	if (r->c->session_control)
	{
#if 1   //-----Check for internal/xgi request, which do not check by php.
		if(pStr &&  (!strcmp("xgi", pStr) || !strcmp("_int", pStr)) &&
					(check_session(r) == -1 || get_sessiongrp(r) != 0)) 
#else
		if(check_session(r)==-1)
#endif
		{
			if (debug)
				log_d("login incorrect !");
			r->error_file = r->c->error_401_file;
			r->status = 401;
			return;
		}
	}
	/*------------------------------*/

	if (r->c->realm && check_realm(r) == -1) {
		if (debug)
			log_d("login incorrect");
		r->error_file = r->c->error_401_file;
		r->status = 401;
		return;
	}
}

static int process_path_translated(struct request *r)
{
	if (r->path_translated[0] == 0)
	{
		if (debug) log_d("empty path_translated");
		r->status = 500;
		return 0;
	}
	if (r->path_translated[0] != '/')
	{
		if (r->status) return 0;
		r->location = r->path_translated;
		if (debug) log_d("redirecting");
		r->status = 302;
		return 0;
	}
	if (get_mime(r, r->path_translated) == -1)
	{
		close_rfd(r);
		log_d("get_mime failed for %s", r->path_translated);
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return 1;
	}
	if (get_path_info(r) == -1)
	{
		if (debug) log_d("get_path_info failed for %s", r->path_translated);
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return 1;
	}
	if (r->class == CLASS_FILE && S_ISDIR(r->finfo.st_mode))
	{
		close_rfd(r);
		if (r->status) return 0;
		if (r->path_args[0] != '/') return makedir(r);
		if (append_indexes(r) == -1)
		{
			if (r->status) return 1;
			if (r->path_args[1] == 0 && r->c->auto_index_command)
			{
				if (r->method == M_POST)
				{
					if (debug) log_d("POST to AutoIndexCommand rejected");
					r->status = 405;
					return 0;
				}
				r->error_file = r->c->auto_index_command;
				return 1;
			}
			if (debug) log_d("file not found");
			r->error_file = r->c->error_404_file;
			r->status = 404;
			return 1;
		}

		/*------------------------------------------*/
		/* Added by Paul Liu 20040609					*/
		/* Check Mime again after append_indexes	*/
		if (get_mime(r, r->path_translated) == -1)
		{
			close_rfd(r);
			log_d("get_mime failed for %s", r->path_translated);
			r->error_file = r->c->error_404_file;
			r->status = 404;
			return 1;
		}
		/*------------------------------------------*/

	}
	if (r->path_args[0] && r->c->path_info_ok == 0 && r->isindex == 0)
	{
		close_rfd(r);
		if (debug) log_d("nonempty path_args while PathInfo is off");
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return 1;
	}

	/*------------------------------*/
	/* Added by Paul Liu 20040514		*/
	/* Only check regular file when r->class != EXTERNAL */
	if (r->class != CLASS_EXTERNAL && strcasecmp(r->content_type, INTERNAL_MAGIC_TYPE)  && !S_ISREG(r->finfo.st_mode))
	{
		close_rfd(r);
		log_d("%s is not a regular file", r->path_translated);
		r->error_file = r->c->error_404_file;
		r->status = 404;
		return 1;
	}
	/*------------------------------*/

	if (r->class != CLASS_FILE) close_rfd(r);
	switch (r->class)
	{
	case CLASS_FILE:		return process_fd(r);
	case CLASS_SPECIAL:		return process_special(r);
	case CLASS_EXTERNAL:	return process_external(r);
	}
	log_d("unknown class!?");
	r->status = 500;
	return 0;
}

static int parse_range_header(struct request *r, const char *s)
{
	char *t;
	int suffix;
	uintmax_t u, v;

	if (strncasecmp(s, "bytes", 5))
		return -1;
	s += 5;
	while (*s == ' ')
		++s;
	if (*s != '=')
		return -1;
	do
		++s;
	while (*s == ' ');
	suffix = *s == '-';
	if (suffix) {
		do
			++s;
		while (*s == ' ');
		if (*s == '-')
			return -1;
	}
	u = strtoumax(s, &t, 10);
	if (t == s)
		return -1;
	s = t;
	while (*s == ' ')
		++s;
	if (suffix) {
		if (*s)
			return -1;
		r->range = -1;
		r->range_suffix = u;
		return 0;
	}
	if (*s != '-')
		return -1;
	do
		++s;
	while (*s == ' ');
	if (*s == 0) {
		r->range = 1;
		r->range_floor = u;
		return 0;
	}
	if (*s == '-')
		return -1;
	v = strtoumax(s, &t, 10);
	if (t == s)
		return -1;
	s = t;
	while (*s == ' ')
		++s;
	if (*s)
		return -1;
	if (v < u)
		return -1;
	r->range = 2;
	r->range_floor = u;
	r->range_ceiling = v;
	return 0;
}

static int parse_http_version(struct request *r)
{
	const char *v;
	char *e;
	unsigned long ma, mi;

	v = r->version;
	if (v == 0)
		return 0;
	if (strncmp(v, "HTTP", 4))
		return -1;
	v += 4;
	while (*v == ' ')
		++v;
	if (*v != '/')
		return -1;
	do
		++v;
	while (*v == ' ');
	if (*v == '-')
		return -1;
	ma = strtoul(v, &e, 10);
	if (e == v)
		return -1;
	v = e;
	while (*v == ' ')
		++v;
	if (*v != '.')
		return -1;
	do
		++v;
	while (*v == ' ');
	if (*v == '-')
		return -1;
	mi = strtoul(v, &e, 10);
	if (e == v || *e)
		return -1;
	if (ma == 0 || ma > INT_MAX || mi > INT_MAX)
		return -1;
	r->protocol_major = ma;
	r->protocol_minor = mi;
	return 0;
}

static const char *header_list_next(const char *s, size_t *lp)
{
	int inquotedstring;
	char c, lastc;
	size_t l;

	while (*s == ',' || *s == ' ')
		++s;
	l = 0;
	lastc = 0;
	inquotedstring = 0;
	while ((c = s[l]) != 0) {
		if (c == ',' && inquotedstring == 0)
			break;
		if (c == '"' && (inquotedstring == 0 || lastc != '\\'))
			inquotedstring = inquotedstring == 0;
		lastc = c;
		++l;
	}
	while (l > 0 && s[l - 1] == ' ')
		--l;
	*lp = l;
	return s;
}

static void parse_connection_header(struct request *r, const char *s)
{
	size_t l;

	while (1) {
		s = header_list_next(s, &l);
		if (l == 0)
			break;
		if (l == 10 && strncasecmp(s, "keep-alive", l) == 0)
			r->cn->keepalive = 1;
		else if (l == 5 && strncasecmp(s, "close", l) == 0)
			r->cn->keepalive = 0;
		s += l;
	}
}

static int parse_expect_header(struct request *r, const char *s)
{
	size_t l;

	while (1) {
		s = header_list_next(s, &l);
		if (l == 0)
			break;
		if (l != 12 || strncasecmp(s, "100-continue", l))
			return -1;
		r->send_continue = 1;
		s += l;
	}
	return 0;
}

static int process_headers(struct request *r)
{
	char *l, *u, *s;
	time_t i;
	size_t n;
	unsigned long cl;

	do {
		l = getline(&r->cn->header_input, 0);
		if (l == 0)
			return -1;
	} while (*l == 0);
	r->method_s = l;
	u = strchr(l, ' ');
	if (u == 0)
		return -1;
	*u++ = 0;
	r->url = u;
	s = strrchr(u, 'H');
	if (s == 0 || s == u || s[-1] != ' ')
		return -1;
	r->version = s;
	s[-1] = 0;
	s = strchr(u, '?');
	if (s) {
		r->args = s + 1;
		*s = 0;
	}
	if (parse_http_version(r) == -1) {
		if (debug)
			log_d("parse_http_version failed for \"%s\"", r->version);
		r->status = 400;
		return 0;
	}
	if (r->protocol_major && r->protocol_minor)
		r->cn->keepalive = 1;
	n = 0;
	while ((l = getline(&r->cn->header_input, 1)) != 0) {
		s = strchr(l, ':');
		if (s == 0)
			continue;
		*s++ = 0;
		while (*s == ' ')
			++s;
		if (*s == 0)
			continue;
		if (!strcasecmp(l, "Connection")) {
			parse_connection_header(r, s);
			continue;
		} else if (!strcasecmp(l, "Keep-Alive"))
			continue;
		else if (!strcasecmp(l, "Expect")) {
			if (parse_expect_header(r, s) == -1) {
				if (debug)
					log_d("parse_expect_header failed for \"%s\"", s);
				r->status = 417;
				return 0;
			}
			continue;
		} else if (!strcasecmp(l, "Content-Type")) {
			/*------------------------------*/
			/* Added by Paul Liu 20040426	*/
			/* Check for multipart upload */
			if (!strncasecmp(s, "multipart/form-data;", 20))
			{
				l = s+20;
				*l++=0;
				if (!strncasecmp(l, "boundary=", 9))
					r->multi_boundary=l+9;
			}
			/*------------------------------*/
			r->in_content_type = s;
			continue;
		} else if (!strcasecmp(l, "Content-Length")) {
			r->in_content_length = s;
			continue;
		} else if (!strcasecmp(l, "Transfer-Encoding")) {
			r->in_transfer_encoding = s;
			continue;
		}
		if (n < tuning.num_headers) {
			r->headers[n].rh_name = l;
			r->headers[n++].rh_value = s;
		}
		if (!strcasecmp(l, "User-agent"))
			r->user_agent = s;
		else if (!strcasecmp(l, "Referer"))
			r->referer = s;
		else if (!strcasecmp(l, "Authorization"))
			r->authorization = s;
		else if (!strcasecmp(l, "Host")) {
			sanitize_host(s);
			r->host = s;
		} else if (!strcasecmp(l, "If-Modified-Since"))
			r->ims_s = s;
		else if (!strcasecmp(l, "If-Unmodified-Since"))
			r->ius_s = s;
		else if (!strcasecmp(l, "Range")) {
			if (r->range_s) {
				log_d("multiple Range headers");
				r->status = 400;
				return 0;
			} else
				r->range_s = s;
		} else if (!strcasecmp(l, "If-Range"))
			r->if_range_s = s;
	}
	r->nheaders = n;
	s = r->method_s;
	/*------------------------------*/
	/* Modified by Paul Liu 20061003 */
	/* Add support for UPNP methods */
	/* Treat M-SEARCH, SUBSCRIBE, UNSUBSCRIBE, NOTIFY as GET */
	if (strcmp(s, m_get) == 0 || strcmp(s, m_msearch) == 0 || strcmp(s, m_subscribe) == 0 || strcmp(s, m_unsubscribe) == 0 || strcmp(s, m_notify) == 0)
	/*------------------------------*/
		r->method = M_GET;
	else {
		if (strcmp(s, m_head) == 0)
			r->method = M_HEAD;
		else if (strcmp(s, m_post) == 0)
			r->method = M_POST;
		else {
			if (debug)
				log_d("method \"%s\" not implemented", s);
			r->status = 501;
			return 0;
		}
	}
	s = r->url;
	if (strlen(s) > STRLEN) {
		if (debug)
			log_d("url too long (%zu > %d)", strlen(s), STRLEN);
		r->status = 414;
		return 0;
	}
	/*------------------------------*/
	/* Modified by Paul Liu 20061003 */
	/* Add support for UPNP methods */
	/* Allow '*' for M-SEARCH method */
	if (*s != '/' && *s != '*') {
	/*------------------------------*/
		u = strchr(s, '/');
		if (u == 0 || u[1] != '/' || u[2] == 0 || u[2] == '/') {
			if (debug)
				log_d("absoluteURI \"%s\" should contain a net_loc", r->url);
			r->status = 400;
			return 0;
		}
		u += 2;
		s = strchr(u, '/');
		if (s == 0)
			strcpy(r->rhost, u);
		else {
			memcpy(r->rhost, u, s - u);
			r->rhost[s - u] = 0;
		}
		r->host = r->rhost;
		sanitize_host(r->host);
	}
	if (r->host == 0 && r->protocol_minor == 1) {
		if (debug)
			log_d("HTTP/1.1 request without Host");
		r->status = 400;
		return 0;
	}
	if (r->host && r->host[0] == 0) {
		if (debug)
			log_d("empty Host header");
		r->status = 400;
		return 0;
	}
	if (s == 0)
		strcpy(r->path, "/");
	else if (unescape_url(s, r->path) == -1) {
		if (debug)
			log_d("unescape_url failed for \"%s\"", s);
		r->status = 400;
		return 0;
	}
	if (r->protocol_major > 1 || (r->protocol_major == 1 && r->protocol_minor > 1)) {
		r->status = 505;
		return 0;
	}
	if (r->in_transfer_encoding) {
		if (strcasecmp(r->in_transfer_encoding, "chunked")) {
			log_d("unimplemented transfer-coding \"%s\"", r->in_transfer_encoding);
			r->status = 501;
			return 0;
		}
		if (r->in_content_length) {
			log_d("ignoring Content-Length header from client");
			r->in_content_length = 0;
		}
	}
	s = r->in_content_length;
	if (s) {
		if (*s == '-' || (cl = strtoul(s, &u, 10), u == s || *u || cl >= UINT_MAX)) {
			log_d("bad Content-Length from client: \"%s\"", s);
			r->status = 400;
			return 0;
		}
		r->in_mblen = cl;
	}
	if (r->method == M_GET) {
		s = r->ims_s;
		if (s) {
			i = timerfc(s);
			if (i != -1 && i <= current_time)
				r->ims = i;
		}
		s = r->ius_s;
		if (s) {
			i = timerfc(s);
			if (i != -1)
				r->ius = i;
		}
		s = r->if_range_s;
		if (s) {
			i = timerfc(s);
			if (i != -1)
				r->if_range = i;
		}
		s = r->range_s;
		if (s) {
			if (parse_range_header(r, s) == -1)
				log_d("ignoring Range header \"%s\"", s);
		}
	} else if (r->method == M_POST) {
		if (r->in_content_length == 0) {
			if (debug)
				log_d("POST: length required");
			r->status = 411;
			return 0;
		}
	}
	if (r->send_continue) {
		if (r->protocol_minor == 0 && r->protocol_major == 1) {
			if (debug)
				log_d("suppressing '100 Continue' response (HTTP/1.0 client)");
			r->send_continue = 0;
		} else if ((r->in_content_length && r->in_mblen) || r->in_transfer_encoding) {
			if (r->cn->header_input.end > r->cn->header_input.middle) {
				if (debug)
					log_d("suppressing '100 Continue' response (more input received)");
				r->send_continue = 0;
			}
		} else {
			if (debug)
				log_d("suppressing '100 Continue' response (no body)");
			r->send_continue = 0;
		}
	}

	/*------------------------------*/
	/* Added by Paul Liu 20040426	*/
	/* Check for multipart upload	*/
	if (r->multi_boundary) return -1;
	/*------------------------------*/
	return 1;
}

static const char *http_code_phrase(int status)
{
	switch (status) {
	case 100:
		return "100 Continue";
	case 200:
		return "200 OK";
	case 204:
		return "204 No Content";
	case 206:
		return "206 Partial Content";
	case 302:
		return "302 Moved";
	case 304:
		return "304 Not Modified";
	case 400:
		return "400 Bad Request";
	case 401:
		return "401 Not Authorized";
	case 403:
		return "403 Forbidden";
	case 404:
		return "404 Not Found";
	case 405:
		return "405 Method Not Allowed";
	case 411:
		return "411 Length Required";
	case 412:
		return "412 Precondition Failed";
	case 414:
		return "414 Request-URI Too Long";
	case 416:
		return "416 Requested Range Not Satisfiable";
	case 417:
		return "417 Expectation Failed";
	case 501:
		return "501 Not Implemented";
	case 503:
		return "503 Service Unavailable";
	case 505:
		return "505 HTTP Version Not Supported";
	default:
		return "500 Internal Server Error";
	}
}

int pool_print(struct pool *p, const char *fmt, ...)
{
	va_list ap;
	size_t n;
	int r;

	if (p->ceiling <= p->end)
		return -1;
	n = p->ceiling - p->end;
	va_start(ap, fmt);
	r = vsnprintf(p->end, n, fmt, ap);
	va_end(ap);
	if ((size_t) r >= n)
		return -1;
	p->end += r;
	return r;
}

static int prepare_reply(struct request *r)
{
	struct pool *p;
	int send_message;
	char gbuf[40];
	struct simple_list *h;
	const char *status_line;
	char *cl_start, *cl_end;

	if (r->forked)
		return 0;
	if (r->in_content_length || r->in_transfer_encoding) {
		if (debug)
			log_d("client sent request-body; turning off keepalive");
		r->cn->keepalive = 0;
	}
	p = &r->cn->output;
	send_message = r->cn->rfd == -1 && r->method != M_HEAD && r->status != 204 && r->status != 304;
	status_line = http_code_phrase(r->status);
	cl_start = cl_end = 0;
	if (send_message) {
		r->num_content = 0;
		r->content_type = "text/html";
		r->content_length = p->ceiling - p->floor; /* hack - this gives us enough room in the output to change it to something less */
	}
	if (r->status >= 400)
		r->last_modified = 0;
	/*------------------------------*/
	/* Modified by Paul Liu 20061016  */
	/* Add support for user defined Server Name */
	if (pool_print(p, "HTTP/1.1 %s\r\nServer: %s\r\nDate: %s\r\n", status_line, r->cn->s->name, rfctime(current_time, gbuf)) == -1)
		return -1;
	/*------------------------------*/
	switch (r->status) {
	case 206:
		if (pool_print(p, "Content-Range: bytes %ju-%ju/%ju\r\n", r->range_floor, r->range_ceiling, r->range_total) == -1)
			return -1;
		break;
	case 302:
		if (r->location)
			if (pool_print(p, "Location: %s\r\n", r->location) == -1)
				return -1;
		break;
	case 401:
		if (r->c && r->c->realm)
			if (pool_print(p, "WWW-Authenticate: Basic realm=\"%s\"\r\n", r->c->realm) == -1)
				return -1;
		break;
	case 405:
		if (pool_print(p, "Allow: GET, HEAD\r\n") == -1)
			return -1;
		break;
	case 416:
		if (pool_print(p, "Content-Range: bytes */%ju\r\n", r->range_total) == -1)
			return -1;
		break;
	}
	if (r->num_content >= 0) {
		if (pool_print(p, "Content-Type: %s\r\n", r->content_type) == -1)
			return -1;
		if (r->content_length >= 0) {
			if (pool_print(p, "Content-Length: ") == -1)
				return -1;
			cl_start = p->end;
			if (pool_print(p, "%jd", r->content_length) == -1)
				return -1;
			cl_end = p->end;
			if (pool_print(p, "\r\n") == -1)
				return -1;
		}
		if (r->last_modified)
			if (pool_print(p, "Last-Modified: %s\r\n", rfctime(r->last_modified, gbuf)) == -1)
				return -1;
	}
	if (r->cn->keepalive) {
		if (r->protocol_minor == 0)
			if (pool_print(p, "Connection: keep-alive\r\n") == -1)
				return -1;
	} else if (r->protocol_minor)
		if (pool_print(p, "Connection: close\r\n") == -1)
			return -1;
	if (r->c && r->status == 200)
		for (h = r->c->extra_headers; h; h = h->next)
			if (pool_print(p, "%s\r\n", h->name) == -1)
				return -1;
	if (pool_print(p, "\r\n") == -1)
		return -1;
	p->middle = p->end;
	if (send_message == 0)
		return 0;
	if (pool_print(p, "<title>%s</title>\n<h1>%s</h1>\n", status_line, status_line) == -1)
		return -1;
	switch (r->status) {
	case 302:
		if (pool_print(p, "This document has moved to URL <a href=\"%s\">%s</a>.\n", r->location, r->location) == -1)
			return -1;
		break;
	case 401:
		if (pool_print(p, "You need proper authorization to use this resource.\n") == -1)
			return -1;
		break;
	case 400:
	case 405:
	case 501:
	case 505:
		if (pool_print(p, "Your request was not understood or not allowed by this server.\n") == -1)
			return -1;
		break;
	case 403:
		if (pool_print(p, "Access to this resource has been denied to you.\n") == -1)
			return -1;
		break;
	case 404:
		if (pool_print(p, "The resource requested could not be found on this server.\n") == -1)
			return -1;
		break;
	case 503:
		if (pool_print(p, "The server is temporarily busy.\n") == -1)
			return -1;
		break;
	}
	if (r->c && r->c->admin)
		if (pool_print(p, "<p>Please contact the site administrator at <i>%s</i>.\n", r->c->admin) == -1)
			return -1;
	r->content_length = p->end - p->middle;
	if (cl_start == 0) {
		log_d("cl_start is null!?!?");
		return -1;
	}
	sprintf(cl_start, "%*jd", (int) (cl_end - cl_start), r->content_length);
	*cl_end = '\r';
	return 0;
}

void init_request(struct request *r)
{
	r->vs = 0;
	r->user_agent = 0;
	r->referer = 0;
	r->authorization = 0;
	r->host = 0;
	r->in_content_type = 0;
	r->in_content_length = 0;
	/*--------------------------*/
	/* Added by Paul Liu 20040426	*/
	/* Check for multipart		*/
	r->multi_boundary = 0;
	/*--------------------------*/
	r->ims_s = 0;
	r->path[0] = 0;
	r->path_translated[0] = 0;
	r->path_args[0] = 0;
	r->num_content = -1;
	r->class = 0;
	r->content_length = -1;
	r->last_modified = 0;
	r->ims = 0;
	r->location = 0;
	r->method_s = 0;
	r->url = 0;
	r->args = 0;
	r->version = 0;
	r->protocol_major = 0;
	r->protocol_minor = 0;
	r->method = 0;
	r->status = 0;
	r->isindex = 0;
	r->c = 0;
	r->error_file = 0;
	r->user[0] = 0;
	/*--------------------------------------*/
	/* Modified by Paul Liu 20050614			*/
	/* Add support for user password Env.	*/
	r->passwd[0] = 0;
	/*--------------------------------------*/
	r->location_length = 0;
	r->nheaders = 0;
	r->range_s = 0;
	r->if_range_s = 0;
	r->if_range = 0;
	r->range = 0;
	r->range_floor = 0;
	r->range_ceiling = 0;
	r->range_suffix = 0;
	r->range_total = 0;
	r->ius_s = 0;
	r->ius = 0;
	r->rhost[0] = 0;
	r->in_transfer_encoding = 0;
	r->in_mblen = 0;
	r->curdir[0] = 0;
	r->send_continue = 0;
	r->forked = 0;
}

int process_request(struct request *r)
{
	int s=1, n;

	/*------------------------------*/
	/* Modified by Paul Liu 20040426	*/
	/* Check for multipart upload	*/

	/* Modified by Paul Liu 20050729*/
	/* Check for contents in more than one packages */
	if (!r->multi_boundary)
	{
		if(!r->in_mblen)    //-----Never been hear before
			s = process_headers(r);
		if (r->in_mblen > strlen(r->cn->header_input.middle))   //-----Has process_headers but not finish read client data
			s = -1;
	}
	/*------------------------------*/

	switch (s)
	{
	case -1:	return -1;
	case 0:		break;
	default:
		process_path(r);
		if (r->status && r->error_file == 0) break;
		n = 0;

		do
		{
			if (r->error_file)
			{
				r->location_length = sprintf(r->path_translated, "%.*s", PATHLEN - 1, r->error_file);
				r->error_file = 0;
			}
			if (debug) log_d("process_request: %s", r->path_translated);
			s = process_path_translated(r);
		} while (++n < 3 && s == 1 && r->error_file);

		if (n == 3 && s == 1)
		{
			log_d("process_request: loop detected");
			r->status = 500;
		}
	}

	if (r->status)
	{
		if (prepare_reply(r) == -1)
		{
			log_d("cannot prepare reply for client");
			return -1;
		}
	}
	else if (r->send_continue)
	{
		if (debug) log_d("sending 100-continue");
		pool_print(&r->cn->output, "HTTP/1.1 %s\r\n\r\n", http_code_phrase(100));
	}

	if (debug) log_d("process_request finished (s=%d)", s);
	return s >= 0 ? 0 : -1;
}

int cgi_error(struct request *r)
{
	int rv;

	if (debug) log_d("cgi_error");

	r->status = 500;
	r->forked = 0;
	close_rfd(r);
	rv = prepare_reply(r);
	if (rv == -1)
	{
		log_d("prepare_reply failed in cgi_error");
		close_connection(r->cn);
		return -1;
	}
	set_connection_state(r->cn, HC_WRITING);
	return rv;
}
