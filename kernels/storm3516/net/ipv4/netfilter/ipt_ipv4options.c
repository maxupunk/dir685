/*
  This is a module which is used to match ipv4 options.
  This file is distributed under the terms of the GNU General Public
  License (GPL). Copies of the GPL can be obtained from:
  ftp://prep.ai.mit.edu/pub/gnu/GPL

  11-mars-2001 Fabrice MARIE <fabrice@netfilter.org> : initial development.
  12-july-2001 Fabrice MARIE <fabrice@netfilter.org> : added router-alert otions matching. Fixed a bug with no-srr
  12-august-2001 Imran Patel <ipatel@crosswinds.net> : optimization of the match.
  18-november-2001 Fabrice MARIE <fabrice@netfilter.org> : added [!] 'any' option match.
  19-february-2004 Harald Welte <laforge@netfilter.org> : merge with 2.6.x
*/

#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ip.h>

#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_ipv4options.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Fabrice Marie <fabrice@netfilter.org>");

static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,
      int *hotdrop)
{
	const struct ipt_ipv4options_info *info = matchinfo;   /* match info for rule */
	const struct iphdr *iph = skb->nh.iph;
	const struct ip_options *opt;

	if (iph->ihl * 4 == sizeof(struct iphdr)) {
		/* No options, so we match only the "DONTs" and the "IGNOREs" */

		if (((info->options & IPT_IPV4OPTION_MATCH_ANY_OPT) == IPT_IPV4OPTION_MATCH_ANY_OPT) ||
		    ((info->options & IPT_IPV4OPTION_MATCH_SSRR) == IPT_IPV4OPTION_MATCH_SSRR) ||
		    ((info->options & IPT_IPV4OPTION_MATCH_LSRR) == IPT_IPV4OPTION_MATCH_LSRR) ||
		    ((info->options & IPT_IPV4OPTION_MATCH_RR) == IPT_IPV4OPTION_MATCH_RR) ||
		    ((info->options & IPT_IPV4OPTION_MATCH_TIMESTAMP) == IPT_IPV4OPTION_MATCH_TIMESTAMP) ||
                    ((info->options & IPT_IPV4OPTION_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_MATCH_ROUTER_ALERT))
			return 0;
		return 1;
	}
	else {
		if ((info->options & IPT_IPV4OPTION_MATCH_ANY_OPT) == IPT_IPV4OPTION_MATCH_ANY_OPT)
			/* there are options, and we don't need to care which one */
			return 1;
		else {
			if ((info->options & IPT_IPV4OPTION_DONT_MATCH_ANY_OPT) == IPT_IPV4OPTION_DONT_MATCH_ANY_OPT)
				/* there are options but we don't want any ! */
				return 0;
		}
	}

	opt = &(IPCB(skb)->opt);

	/* source routing */
	if ((info->options & IPT_IPV4OPTION_MATCH_SSRR) == IPT_IPV4OPTION_MATCH_SSRR) {
		if (!((opt->srr) && (opt->is_strictroute)))
			return 0;
	}
	else if ((info->options & IPT_IPV4OPTION_MATCH_LSRR) == IPT_IPV4OPTION_MATCH_LSRR) {
		if (!((opt->srr) && (!opt->is_strictroute)))
			return 0;
	}
	else if ((info->options & IPT_IPV4OPTION_DONT_MATCH_SRR) == IPT_IPV4OPTION_DONT_MATCH_SRR) {
		if (opt->srr)
			return 0;
	}
	/* record route */
	if ((info->options & IPT_IPV4OPTION_MATCH_RR) == IPT_IPV4OPTION_MATCH_RR) {
		if (!opt->rr)
			return 0;
	}
	else if ((info->options & IPT_IPV4OPTION_DONT_MATCH_RR) == IPT_IPV4OPTION_DONT_MATCH_RR) {
		if (opt->rr)
			return 0;
	}
	/* timestamp */
	if ((info->options & IPT_IPV4OPTION_MATCH_TIMESTAMP) == IPT_IPV4OPTION_MATCH_TIMESTAMP) {
		if (!opt->ts)
			return 0;
	}
	else if ((info->options & IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP) == IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP) {
		if (opt->ts)
			return 0;
	}
	/* router-alert option  */
	if ((info->options & IPT_IPV4OPTION_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_MATCH_ROUTER_ALERT) {
		if (!opt->router_alert)
			return 0;
	}
	else if ((info->options & IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT) {
		if (opt->router_alert)
			return 0;
	}

	/* we match ! */
	return 1;
}

static int
checkentry(const char *tablename,
	   const struct ipt_ip *ip,
	   void *matchinfo,
	   unsigned int matchsize,
	   unsigned int hook_mask)
{
	const struct ipt_ipv4options_info *info = matchinfo;   /* match info for rule */
	/* Check the size */
	if (matchsize != IPT_ALIGN(sizeof(struct ipt_ipv4options_info)))
		return 0;
	/* Now check the coherence of the data ... */
	if (((info->options & IPT_IPV4OPTION_MATCH_ANY_OPT) == IPT_IPV4OPTION_MATCH_ANY_OPT) &&
	    (((info->options & IPT_IPV4OPTION_DONT_MATCH_SRR) == IPT_IPV4OPTION_DONT_MATCH_SRR) ||
	     ((info->options & IPT_IPV4OPTION_DONT_MATCH_RR) == IPT_IPV4OPTION_DONT_MATCH_RR) ||
	     ((info->options & IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP) == IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP) ||
	     ((info->options & IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT) ||
	     ((info->options & IPT_IPV4OPTION_DONT_MATCH_ANY_OPT) == IPT_IPV4OPTION_DONT_MATCH_ANY_OPT)))
		return 0; /* opposites */
	if (((info->options & IPT_IPV4OPTION_DONT_MATCH_ANY_OPT) == IPT_IPV4OPTION_DONT_MATCH_ANY_OPT) &&
	    (((info->options & IPT_IPV4OPTION_MATCH_LSRR) == IPT_IPV4OPTION_MATCH_LSRR) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_SSRR) == IPT_IPV4OPTION_MATCH_SSRR) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_RR) == IPT_IPV4OPTION_MATCH_RR) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_TIMESTAMP) == IPT_IPV4OPTION_MATCH_TIMESTAMP) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_MATCH_ROUTER_ALERT) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_ANY_OPT) == IPT_IPV4OPTION_MATCH_ANY_OPT)))
		return 0; /* opposites */
	if (((info->options & IPT_IPV4OPTION_MATCH_SSRR) == IPT_IPV4OPTION_MATCH_SSRR) &&
	    ((info->options & IPT_IPV4OPTION_MATCH_LSRR) == IPT_IPV4OPTION_MATCH_LSRR))
		return 0; /* cannot match in the same time loose and strict source routing */
	if ((((info->options & IPT_IPV4OPTION_MATCH_SSRR) == IPT_IPV4OPTION_MATCH_SSRR) ||
	     ((info->options & IPT_IPV4OPTION_MATCH_LSRR) == IPT_IPV4OPTION_MATCH_LSRR)) &&
	    ((info->options & IPT_IPV4OPTION_DONT_MATCH_SRR) == IPT_IPV4OPTION_DONT_MATCH_SRR))
		return 0; /* opposites */
	if (((info->options & IPT_IPV4OPTION_MATCH_RR) == IPT_IPV4OPTION_MATCH_RR) &&
	    ((info->options & IPT_IPV4OPTION_DONT_MATCH_RR) == IPT_IPV4OPTION_DONT_MATCH_RR))
		return 0; /* opposites */
	if (((info->options & IPT_IPV4OPTION_MATCH_TIMESTAMP) == IPT_IPV4OPTION_MATCH_TIMESTAMP) &&
	    ((info->options & IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP) == IPT_IPV4OPTION_DONT_MATCH_TIMESTAMP))
		return 0; /* opposites */
	if (((info->options & IPT_IPV4OPTION_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_MATCH_ROUTER_ALERT) &&
	    ((info->options & IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT) == IPT_IPV4OPTION_DONT_MATCH_ROUTER_ALERT))
		return 0; /* opposites */

	/* everything looks ok. */
	return 1;
}

static struct ipt_match ipv4options_match = { 
	.name = "ipv4options",
	.match = match,
	.checkentry = checkentry,
	.me = THIS_MODULE
};

static int __init init(void)
{
	return ipt_register_match(&ipv4options_match);
}

static void __exit fini(void)
{
	ipt_unregister_match(&ipv4options_match);
}

module_init(init);
module_exit(fini);
