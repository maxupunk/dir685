/* This module matches until it expires, at which point the entire
 * rule is deleted
 *
 * This module is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This module is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this module; if not, write to:
 *      The Free Software Foundation, Inc.
 *      59 Temple Place, Suite 330
 *      Boston, MA  02111-1307  USA
 *
 * Copyright � 2005 Bryan Cardillo <dillo@seas.upenn.edu>
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/workqueue.h>
#include <linux/vmalloc.h>
#include <linux/time.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ipt_expire.h>

#if CONFIG_NETFILTER_DEBUG
#define dprintk(format, args...) \
	printk("ipt_expire[%s]: " format "\n", __FUNCTION__, ## args)
#else
#define dprintk(format, args...)
#endif

MODULE_AUTHOR("Bryan Cardillo <dillo@seas.upenn.edu>");
MODULE_DESCRIPTION("an iptables expiring match module");
MODULE_LICENSE("GPL");
MODULE_VERSION("1.1");
static int __init ipt_exp_init(void);
static void __exit ipt_exp_exit(void);
module_init(ipt_exp_init);
module_exit(ipt_exp_exit);

static int ipt_exp_match(const struct sk_buff *,
		const struct net_device *, const struct net_device *,
		const void *, int, int *);
static int ipt_exp_checkentry(const char *, const struct ipt_ip *,
		void *, unsigned int, unsigned int);
static int ipt_exp_add_table(const char *);
static void ipt_exp_remove_table(const char *);
static void ipt_exp_schedule_expiration(time_t);
static void ipt_exp_work_fn(void *);
static int ipt_exp_get_info(const char *, struct ipt_getinfo *);
static int ipt_exp_get_entries(struct ipt_getinfo *, struct ipt_get_entries *);
static int ipt_exp_get_active(struct ipt_getinfo *,
		struct ipt_get_entries *, struct ipt_replace *);
static int ipt_exp_copy_active(struct ipt_entry *, struct ipt_replace *);
static int ipt_exp_is_expired(struct ipt_entry_match *);
static int ipt_exp_replace_expired(struct ipt_replace *);
static int ipt_exp_get_counters(struct ipt_get_entries *,
		struct ipt_replace *, struct ipt_counters_info *);
static int ipt_exp_copy_counter(struct ipt_entry *, struct ipt_replace *,
		struct ipt_counters_info *, int *);
static int ipt_exp_restore_counters(struct ipt_counters_info *);

/**
 * struct for list of tables
 */
struct ipt_exp_table {
	/**
	 * the table name
	 */
	char name[IPT_TABLE_MAXNAMELEN];
	/**
	 * a list_head structure enabling list inclusion
	 */
	struct list_head list;
};

/**
 * work_struct for scheduling the deletion of expired rules
 */
static DECLARE_WORK(ipt_exp_work, &ipt_exp_work_fn, NULL);

/**
 * iptables match
 */
static struct ipt_match ipt_expire_match = {
	.name		= "expire",
	.match		= &ipt_exp_match,
	.checkentry	= &ipt_exp_checkentry,
	.me		= THIS_MODULE
};

/**
 * the list of tables contained expiring entries
 */
static spinlock_t ipt_exp_tables_lock = SPIN_LOCK_UNLOCKED;
static LIST_HEAD(ipt_exp_tables);

/**
 * initialize module and register iptables match
 * @see module_init()
 * @see ipt_register_match()
 */
static int __init
ipt_exp_init(void)
{
	dprintk("initializing");
	ipt_register_match(&ipt_expire_match);
	return 0;
}

/**
 * cleanup module and unregister iptables match
 * @see module_exit()
 * @see ipt_unregister_match()
 */
static void __exit
ipt_exp_exit(void)
{
	unsigned long flags;
	struct ipt_exp_table *t, *tmp;

	dprintk("exiting");

	ipt_unregister_match(&ipt_expire_match);
	cancel_delayed_work(&ipt_exp_work);
	
	spin_lock_irqsave(&ipt_exp_tables_lock, flags);
	list_for_each_entry_safe(t, tmp, &ipt_exp_tables, list)
		kfree(t);
	spin_unlock_irqrestore(&ipt_exp_tables_lock, flags);
}

/**
 * match if the expiration time has't passed
 * @param skb socket buffer
 * @param in inbound network device
 * @param out outbound network device
 * @param matchinfo match specific data
 * @param offset match offset (?)
 * @param hotdrop set to 1 to drop packet immediately when returning false
 * @return non-zero for active rules, zero otherwise
 * @see struct ipt_match
 */
static int
ipt_exp_match(const struct sk_buff *skb,
		const struct net_device *in, const struct net_device *out,
		const void *matchinfo, int offset, int *hotdrop)
{
	const struct ipt_exp_info *info = matchinfo;

	if (get_seconds() < info->expiration)
		return 1;

	return 0;
}

/**
 * check a new iptables entry
 * @param tablename table name for new entry
 * @param ip ip info for new entry
 * @param matchinfo match specific data
 * @param matchsize size of matchinfo data
 * @param hookmask valid netfilter hooks (?)
 * @return non-zero for valid entries, zero otherwise
 */
static int
ipt_exp_checkentry(const char *tablename, const struct ipt_ip *ip,
		void *matchinfo, unsigned int matchsize, unsigned int hookmask)
{
	struct ipt_exp_info *info = matchinfo;

	if (matchsize != IPT_ALIGN(sizeof(struct ipt_exp_info)))
		return 0;

	if (info->expiration <= get_seconds())
		return 0;

	if (ipt_exp_add_table(tablename))
		return 0;

	ipt_exp_schedule_expiration(info->expiration);

	return 1;
}

/**
 * add a table to the set of tables to be searched for expired rules
 * @param tablename the name of the table
 * @return zero on success, non-zero on failure
 */
static int
ipt_exp_add_table(const char *tablename)
{
	unsigned long flags;
	struct ipt_exp_table *t;

	spin_lock_irqsave(&ipt_exp_tables_lock, flags);
	list_for_each_entry(t, &ipt_exp_tables, list)
		if (strncmp(t->name, tablename, IPT_TABLE_MAXNAMELEN) == 0)
			break;
	spin_unlock_irqrestore(&ipt_exp_tables_lock, flags);

	if (&t->list == &ipt_exp_tables) {
		if (!(t = kmalloc(sizeof(struct ipt_exp_table), GFP_KERNEL))) {
			dprintk("error allocating memory");
			return -ENOMEM;
		}
		strlcpy(t->name, tablename, IPT_TABLE_MAXNAMELEN);
		spin_lock_irqsave(&ipt_exp_tables_lock, flags);
		list_add_tail(&t->list, &ipt_exp_tables);
		spin_unlock_irqrestore(&ipt_exp_tables_lock, flags);
	}
	return 0;
}

/**
 * remove a table from the set of tables to be searched for expired rules
 * @param tablename the name of the table to be removed
 */
static void
ipt_exp_remove_table(const char *tablename)
{
	unsigned long flags;
	struct ipt_exp_table *t, *tmp;

	spin_lock_irqsave(&ipt_exp_tables_lock, flags);
	list_for_each_entry_safe(t, tmp, &ipt_exp_tables, list) {
		if (strncmp(t->name, tablename, IPT_TABLE_MAXNAMELEN) == 0) {
			list_del(&t->list);
			kfree(t);
		}
	}
	spin_unlock_irqrestore(&ipt_exp_tables_lock, flags);
}

/**
 * schedule the next removal of expired rules
 * @param expiration the time to be scheduled
 */
static void
ipt_exp_schedule_expiration(time_t expiration)
{
	unsigned long delay = (expiration - get_seconds() < 1) ? HZ :
					(expiration - get_seconds()) * HZ;

	schedule_delayed_work(&ipt_exp_work, delay);
}

/**
 * delete expired iptables rules
 * @param __notused
 * @see schedule_delayed_work()
 */
static void
ipt_exp_work_fn(void *__notused)
{
	struct ipt_exp_table *t;

	/*
	 * FIXME what about locking here?
	 */
	list_for_each_entry(t, &ipt_exp_tables, list) {
		size_t sz;
		struct ipt_getinfo info;
		struct ipt_get_entries *entries = NULL;
		struct ipt_replace *replace = NULL;
		struct ipt_counters_info *counters = NULL;

		dprintk("expiring %s entries", t->name);

		/* get table info */
		if (ipt_exp_get_info(t->name, &info))
			continue;

		/* allocate memory */
		sz = sizeof(struct ipt_get_entries) + info.size;
		if (!(entries = vmalloc(sz))) {
			dprintk("error allocating entry table");
			break;
		}
		sz = sizeof(struct ipt_replace) + info.size;
		if (!(replace = vmalloc(sz))) {
			dprintk("error allocating replacement table");
			goto out_free_entries;
		}
		memset(replace, 0, sz);
		sz = sizeof(struct ipt_counters) * info.num_entries;
		if (!(replace->counters = vmalloc(sz))) {
			dprintk("error allocating counters");
			goto out_free_entries_replace;
		}
		memset(replace->counters, 0, sz);
		sz += sizeof(struct ipt_counters_info);
		if (!(counters = vmalloc(sz))) {
			dprintk("error allocating new counters");
			goto out_free_entries_replace_counters;
		}
		memset(counters, 0, sz);

		/* get all entries, then copy active ones */
		if (ipt_exp_get_entries(&info, entries))
			goto out_free_entries_replace_counters_info;
		if (ipt_exp_get_active(&info, entries, replace))
			goto out_free_entries_replace_counters_info;

		/* replace table */
		if (replace->size < info.size) {
			if (ipt_exp_get_counters(entries, replace, counters))
				goto out_free_entries_replace_counters_info;
			if (ipt_exp_replace_expired(replace))
				goto out_free_entries_replace_counters_info;
			if (ipt_exp_restore_counters(counters))
				goto out_free_entries_replace_counters_info;
		}

		dprintk("expired %s entries", t->name);

out_free_entries_replace_counters_info:
		vfree(counters);
out_free_entries_replace_counters:
		vfree(replace->counters);
out_free_entries_replace:
		vfree(replace);
out_free_entries:
		vfree(entries);
	}
}

/**
 * get info on an a table
 * @param name the name of the table
 * @param info the location to store the retrieved info
 * @return zero on success, non-zero otherwise
 */
static int
ipt_exp_get_info(const char *name, struct ipt_getinfo *info)
{
	int ret = 0;
	int sz = sizeof(struct ipt_getinfo);

	dprintk("getting entry info");

	strlcpy(info->name, name, IPT_TABLE_MAXNAMELEN);
	ret = nf_getsockopt(NULL, PF_INET, IPT_SO_GET_INFO, (char *)info, &sz);
	switch (ret) {
		case 0:
			break;
		case -ENOENT:
			/* table is gone */
			ipt_exp_remove_table(name);
			break;
		default:
			dprintk("error getting iptables info");
			break;
	}

	return ret;
}

/**
 * get the entries for a table
 * @param info the location of info about the table
 * @param entries the location to allocate and store the retrieved entries
 * @return zero on success, non-zero otherwise
 */
static int
ipt_exp_get_entries(struct ipt_getinfo *info, struct ipt_get_entries *entries)
{
	int ret = 0;
	int sz = sizeof(struct ipt_get_entries) + info->size;

	dprintk("getting existing entries");

	strlcpy(entries->name, info->name, IPT_TABLE_MAXNAMELEN);
	entries->size = info->size;
	ret = nf_getsockopt(NULL, PF_INET,
			IPT_SO_GET_ENTRIES, (char *)entries, &sz);
	if (ret)
		dprintk("error getting iptables entries");

	return ret;
}

/**
 * iterate over a tables entries, copying entries which
 * are not expired to the replacement table
 * @param info the location of info about the table
 * @param entries the location of the table entries
 * @param replace the location to allocate and store the replacement entries
 * @return zero on success, non-zero otherwise
 */
static int
ipt_exp_get_active(struct ipt_getinfo *info,
		struct ipt_get_entries *entries, struct ipt_replace *replace)
{
	dprintk("copying active entries");

	/* copy stuff */
	strlcpy(replace->name, info->name, IPT_TABLE_MAXNAMELEN);
	memcpy(replace->hook_entry, info->hook_entry, sizeof(info->hook_entry));
	memcpy(replace->underflow, info->underflow, sizeof(info->underflow));
	replace->valid_hooks = info->valid_hooks;
	replace->num_counters = info->num_entries;

	/* set size and entry count */
	replace->size = 0;
	replace->num_entries = 0;

	return IPT_ENTRY_ITERATE(entries->entrytable, entries->size, 
					ipt_exp_copy_active, replace);
}

/**
 * copy a table entry if it is not expired
 * @param entry the source entry
 * @param replace the replacement table
 * @return zero;
 */
static int
ipt_exp_copy_active(struct ipt_entry *entry, struct ipt_replace *replace)
{
	if (!(IPT_MATCH_ITERATE(entry, ipt_exp_is_expired))) {
		struct ipt_entry *dest =
				(void *)replace->entries + replace->size;
		memcpy(dest, entry, entry->next_offset);
		replace->size += entry->next_offset;
		replace->num_entries++;
	}

	return 0;
}

/**
 * determine if an entry is expired
 * @param match the entry match to check for expiration
 * @return non-zero for expired entries, zero otherwise
 */
static int
ipt_exp_is_expired(struct ipt_entry_match *match)
{
	if (strcmp(match->u.user.name, "expire") == 0) {
		struct ipt_exp_info *info = (struct ipt_exp_info *)match->data;
		if (info->expiration <= get_seconds())
			return 1;
	}
	return 0;
}

/**
 * update a table
 * @param replace the replacement table
 * @return zero on success, non-zero otherwise
 */
static int
ipt_exp_replace_expired(struct ipt_replace *replace)
{
	int ret = 0;
	int sz = sizeof(struct ipt_replace) + replace->size;

	dprintk("replacing table %s", replace->name);

	ret = nf_setsockopt(NULL, PF_INET,
			IPT_SO_SET_REPLACE, (char *)replace, sz);
	/*
	 * FIXME remove this verbosity once tested?
	 */
	switch (ret) {
		case 0:
			break;
		case -EFAULT:
			dprintk("EFAULT replacing iptables");
			break;
		case -ENOPROTOOPT:
			dprintk("ENOPROTOOPT replacing iptables");
			break;
		case -ENOMEM:
			dprintk("ENOMEM replacing iptables");
			break;
		case -ENOENT:
			dprintk("ENOENT replacing iptables");
			break;
		case -ELOOP:
			dprintk("ELOOP replacing iptables");
			break;
		case -EINVAL:
			dprintk("EINVAL replacing iptables");
			break;
		default:
			dprintk("unknown error (%d) replacing iptables", ret);
			break;
	}
	return ret;
}

/**
 * get counters for unexpired entries
 * @param entries the entries info
 * @param replace the replacement structure, containing the old counters
 * @param counters the new counter info
 * @return zero on success, non-zero on failure
 */
static int
ipt_exp_get_counters(struct ipt_get_entries *entries,
		struct ipt_replace *replace,
		struct ipt_counters_info *counters)
{
	int index = 0;
	dprintk("copying active counters");

	/* init stuff */
	strlcpy(counters->name, replace->name, IPT_TABLE_MAXNAMELEN);
	counters->num_counters = 0;

	return IPT_ENTRY_ITERATE(entries->entrytable, entries->size, 
			ipt_exp_copy_counter, replace, counters, &index);
}

/**
 * copy a rule counter if the rule is still active
 * @param entry the entry
 * @param replace the replacement structure containing the old counters
 * @param counter the new counter info
 * @param index the current entry index
 * @return zero
 */
static int
ipt_exp_copy_counter(struct ipt_entry *entry, struct ipt_replace *replace,
		struct ipt_counters_info *counters, int *index)
{
	if (!(IPT_MATCH_ITERATE(entry, ipt_exp_is_expired)))
		counters->counters[counters->num_counters++] =
				replace->counters[*index];
	(*index)++;
	return 0;
}

/**
 * restore the counters for a table
 * @param counter the counters
 * @return zero on success, non-zero otherwise
 */
static int
ipt_exp_restore_counters(struct ipt_counters_info *counters)
{
	int ret = 0;
	int sz = sizeof(struct ipt_counters_info) +
			sizeof(struct ipt_counters) * counters->num_counters;

	dprintk("restoring counters for %s", counters->name);

	ret = nf_setsockopt(NULL, PF_INET,
			IPT_SO_SET_ADD_COUNTERS, (char *)counters, sz);
	if (ret)
		dprintk("error restoring counters (%d)", ret);
	return ret;
}
