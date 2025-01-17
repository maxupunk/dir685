/*
 * tc_util.c		Misc TC utility functions.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <math.h>

#include "utils.h"
#include "tc_util.h"

int get_qdisc_handle(__u32 *h, const char *str)
{
	__u32 maj;
	char *p;

	maj = TC_H_UNSPEC;
	if (strcmp(str, "none") == 0)
		goto ok;
	maj = strtoul(str, &p, 16);
	if (p == str)
		return -1;
	maj <<= 16;
	if (*p != ':' && *p!=0)
		return -1;
ok:
	*h = maj;
	return 0;
}

int get_tc_classid(__u32 *h, const char *str)
{
	__u32 maj, min;
	char *p;

	maj = TC_H_ROOT;
	if (strcmp(str, "root") == 0)
		goto ok;
	maj = TC_H_UNSPEC;
	if (strcmp(str, "none") == 0)
		goto ok;
	maj = strtoul(str, &p, 16);
	if (p == str) {
		maj = 0;
		if (*p != ':')
			return -1;
	}
	if (*p == ':') {
		maj <<= 16;
		str = p+1;
		min = strtoul(str, &p, 16);
		if (*p != 0)
			return -1;
		maj |= min;
	} else if (*p != 0)
		return -1;

ok:
	*h = maj;
	return 0;
}

int print_tc_classid(char *buf, int len, __u32 h)
{
	if (h == TC_H_ROOT)
		sprintf(buf, "root");
	else if (h == TC_H_UNSPEC)
		snprintf(buf, len, "none");
	else if (TC_H_MAJ(h) == 0)
		snprintf(buf, len, ":%x", TC_H_MIN(h));
	else if (TC_H_MIN(h) == 0)
		snprintf(buf, len, "%x:", TC_H_MAJ(h)>>16);
	else
		snprintf(buf, len, "%x:%x", TC_H_MAJ(h)>>16, TC_H_MIN(h));
	return 0;
}

char * sprint_tc_classid(__u32 h, char *buf)
{
	if (print_tc_classid(buf, SPRINT_BSIZE-1, h))
		strcpy(buf, "???");
	return buf;
}

/* See http://physics.nist.gov/cuu/Units/binary.html */
static const struct rate_suffix {
	const char *name;
	double scale;
} suffixes[] = {
	{ "bit",	1. },
	{ "Kibit",	1024. },
	{ "kbit",	1000. },
	{ "mibit",	1024.*1024. },
	{ "mbit",	1000000. },
	{ "gibit",	1024.*1024.*1024. },
	{ "gbit",	1000000000. },
	{ "tibit",	1024.*1024.*1024.*1024. },
	{ "tbit",	1000000000000. },
	{ "Bps",	8. },
	{ "KiBps",	8.*1024. },
	{ "KBps",	8000. },
	{ "MiBps",	8.*1024*1024. },
	{ "MBps",	8000000. },
	{ "GiBps",	8.*1024.*1024.*1024. },
	{ "GBps",	8000000000. },
	{ "TiBps",	8.*1024.*1024.*1024.*1024. },
	{ "TBps",	8000000000000. },
	{ NULL }
};


int get_rate(unsigned *rate, const char *str)
{
	char *p;
	double bps = strtod(str, &p);
	const struct rate_suffix *s;

	if (p == str)
		return -1;

	if (*p == '\0') {
		*rate = bps / 8.;	/* assume bytes/sec */
		return 0;
	}

	for (s = suffixes; s->name; ++s) {
		if (strcasecmp(s->name, p) == 0) {
			*rate = (bps * s->scale) / 8.;
			return 0;
		}
	}

	return -1;
}

int get_rate_and_cell(unsigned *rate, int *cell_log, char *str)
{
	char * slash = strchr(str, '/');

	if (slash)
		*slash = 0;

	if (get_rate(rate, str))
		return -1;

	if (slash) {
		int cell;
		int i;

		if (get_integer(&cell, slash+1, 0))
			return -1;
		*slash = '/';

		for (i=0; i<32; i++) {
			if ((1<<i) == cell) {
				*cell_log = i;
				return 0;
			}
		}
		return -1;
	}
	return 0;
}

void print_rate(char *buf, int len, __u32 rate)
{
	double tmp = (double)rate*8;
	extern int use_iec;

	if (use_iec) {
		if (tmp >= 1024*1023 && 
		    fabs(1024*1024*rint(tmp/(1024*1024)) - tmp) < 1024)
			snprintf(buf, len, "%gMibit", rint(tmp/(1024*1024)));
		else if (tmp >= 1024-16 && fabs(1024*rint(tmp/1024) - tmp) < 16)
			snprintf(buf, len, "%gKibit", rint(tmp/1024));
		else
			snprintf(buf, len, "%ubit", rate);

	} else { 
		if (tmp >= 999999 && 
		    fabs(1000000.*rint(tmp/1000000.) - tmp) < 1000)
			snprintf(buf, len, "%gMbit", rint(tmp/1000000.));
		else if (tmp >= 990 && fabs(1000.*rint(tmp/1000.) - tmp) < 10)
			snprintf(buf, len, "%gKbit", rint(tmp/1000.));
		else
			snprintf(buf, len, "%ubit", rate);
	}
}

char * sprint_rate(__u32 rate, char *buf)
{
	print_rate(buf, SPRINT_BSIZE-1, rate);
	return buf;
}

int get_usecs(unsigned *usecs, const char *str)
{
	double t;
	char *p;

	t = strtod(str, &p);
	if (p == str)
		return -1;

	if (*p) {
		if (strcasecmp(p, "s") == 0 || strcasecmp(p, "sec")==0 ||
		    strcasecmp(p, "secs")==0)
			t *= 1000000;
		else if (strcasecmp(p, "ms") == 0 || strcasecmp(p, "msec")==0 ||
			 strcasecmp(p, "msecs") == 0)
			t *= 1000;
		else if (strcasecmp(p, "us") == 0 || strcasecmp(p, "usec")==0 ||
			 strcasecmp(p, "usecs") == 0)
			t *= 1;
		else
			return -1;
	}

	*usecs = t;
	return 0;
}


void print_usecs(char *buf, int len, __u32 usec)
{
	double tmp = usec;

	if (tmp >= 1000000)
		snprintf(buf, len, "%.1fs", tmp/1000000);
	else if (tmp >= 1000)
		snprintf(buf, len, "%.1fms", tmp/1000);
	else
		snprintf(buf, len, "%uus", usec);
}

char * sprint_usecs(__u32 usecs, char *buf)
{
	print_usecs(buf, SPRINT_BSIZE-1, usecs);
	return buf;
}

int get_size(unsigned *size, const char *str)
{
	double sz;
	char *p;

	sz = strtod(str, &p);
	if (p == str)
		return -1;

	if (*p) {
		if (strcasecmp(p, "kb") == 0 || strcasecmp(p, "k")==0)
			sz *= 1024;
		else if (strcasecmp(p, "gb") == 0 || strcasecmp(p, "g")==0)
			sz *= 1024*1024*1024;
		else if (strcasecmp(p, "gbit") == 0)
			sz *= 1024*1024*1024/8;
		else if (strcasecmp(p, "mb") == 0 || strcasecmp(p, "m")==0)
			sz *= 1024*1024;
		else if (strcasecmp(p, "mbit") == 0)
			sz *= 1024*1024/8;
		else if (strcasecmp(p, "kbit") == 0)
			sz *= 1024/8;
		else if (strcasecmp(p, "b") != 0)
			return -1;
	}

	*size = sz;
	return 0;
}

int get_size_and_cell(unsigned *size, int *cell_log, char *str)
{
	char * slash = strchr(str, '/');

	if (slash)
		*slash = 0;

	if (get_size(size, str))
		return -1;

	if (slash) {
		int cell;
		int i;

		if (get_integer(&cell, slash+1, 0))
			return -1;
		*slash = '/';

		for (i=0; i<32; i++) {
			if ((1<<i) == cell) {
				*cell_log = i;
				return 0;
			}
		}
		return -1;
	}
	return 0;
}

void print_size(char *buf, int len, __u32 sz)
{
	double tmp = sz;

	if (sz >= 1024*1024 && fabs(1024*1024*rint(tmp/(1024*1024)) - sz) < 1024)
		snprintf(buf, len, "%gMb", rint(tmp/(1024*1024)));
	else if (sz >= 1024 && fabs(1024*rint(tmp/1024) - sz) < 16)
		snprintf(buf, len, "%gKb", rint(tmp/1024));
	else
		snprintf(buf, len, "%ub", sz);
}

char * sprint_size(__u32 size, char *buf)
{
	print_size(buf, SPRINT_BSIZE-1, size);
	return buf;
}

static double percent_scale = (double)(1ull << 32) / 100.;

int get_percent(__u32 *percent, const char *str)
{
	char *p;
	double per = strtod(str, &p);

	if (per > 100.)
		return -1;
	if (*p && strcmp(p, "%"))
		return -1;

	*percent = per * percent_scale;
	return 0;
}

void print_percent(char *buf, int len, __u32 per)
{
	snprintf(buf, len, "%g%%", (double) per / percent_scale);
}

char * sprint_percent(__u32 per, char *buf)
{
	print_percent(buf, SPRINT_BSIZE-1, per);
	return buf;
}

void print_qdisc_handle(char *buf, int len, __u32 h)
{
	snprintf(buf, len, "%x:", TC_H_MAJ(h)>>16);
}

char * sprint_qdisc_handle(__u32 h, char *buf)
{
	print_qdisc_handle(buf, SPRINT_BSIZE-1, h);
	return buf;
}

char * action_n2a(int action, char *buf, int len)
{
	switch (action) {
	case -1:
		return "continue";
		break;
	case TC_ACT_OK:
		return "pass";
		break;
	case TC_ACT_SHOT:
		return "drop";
		break;
	case TC_ACT_RECLASSIFY:
		return "reclassify";
	case TC_ACT_PIPE:
		return "pipe";
	case TC_ACT_STOLEN:
		return "stolen";
	default:
		snprintf(buf, len, "%d", action);
		return buf;
	}
}

int action_a2n(char *arg, int *result)
{
	int res;

	if (matches(arg, "continue") == 0)
		res = -1;
	else if (matches(arg, "drop") == 0)
		res = TC_ACT_SHOT;
	else if (matches(arg, "shot") == 0)
		res = TC_ACT_SHOT;
	else if (matches(arg, "pass") == 0)
		res = TC_ACT_OK;
	else if (strcmp(arg, "ok") == 0)
		res = TC_ACT_OK;
	else if (matches(arg, "reclassify") == 0)
		res = TC_ACT_RECLASSIFY;
	else {
		char dummy;
		if (sscanf(arg, "%d%c", &res, &dummy) != 1)
			return -1;
	}
	*result = res;
	return 0;
}

void print_tm(FILE * f, struct tcf_t *tm)
{
	int hz = get_hz();
	if (tm->install != 0)
		fprintf(f, " installed %d sec", tm->install/hz);
	if (tm->lastuse != 0)
		fprintf(f, " used %d sec", tm->lastuse/hz);
	if (tm->expires != 0)
		fprintf(f, " expires %d sec", tm->expires/hz);
}
