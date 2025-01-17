/****************************************************************************
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.                
*----------------------------------------------------------------------------
* Name			: sl351x_nat.c
* Description	: 
*		Handle Storlink SL351x NAT Functions
*
*
* Packet Flow:
*
*            (xmit)+<--- SW NAT -->+(xmit)
*                  |       ^^      |
*                  |       ||      |
*                  |       ||      |
*   Client <---> GMAC-x  HW-NAT  GMAC-y  <---> Server
*
*
* History
*
*	Date		Writer		Description
*----------------------------------------------------------------------------
*	03/13/2006	Gary Chen	Create and implement
*	07/03/2008	CH HSU		Support DMZ 
*
****************************************************************************/
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/compiler.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mii.h>
#include <linux/completion.h>
#include <asm/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/semaphore.h>
#include <asm/arch/irqs.h>
#include <asm/arch/it8712.h>
#include <linux/mtd/kvctl.h>
#include <linux/skbuff.h>
#include <linux/if_ether.h>
#include <linux/if_pppox.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/ppp_defs.h>
#include <net/ip.h>

#define	 MIDWAY 
#define	 SL_LEPUS

#include <asm/arch/sl2312.h>
#include <asm/arch/sl351x_gmac.h>
#include <asm/arch/sl351x_hash_cfg.h>
#include <asm/arch/sl351x_nat_cfg.h>
#ifdef CONFIG_NETFILTER
#include <linux/netfilter_ipv4/ip_conntrack.h>
#include <linux/netfilter_ipv4/ip_conntrack_tcp.h>
#endif

//#define NAT_DEBUG_MSG		1
#define _NOT_CHECK_SIP_DIP
//#define	SL351x_NAT_TEST_BY_SMARTBITS		1	// Initialize 32 hash entries and test by SmartBITS
//joel mark
//#define VITESSE_G5SWITCH	1
#ifdef CONFIG_RTL8366SR_PHY
#undef VITESSE_G5SWITCH
#define CONFIG_SL351x_RTLDMZ	1
//#define SKIP_NAT_CFG	1
#endif

#ifdef CONFIG_SL351x_NAT

/*----------------------------------------------------------------------
* Definition
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL3516_ASIC
#define CONFIG_SL351x_NAT_TCP_UDP
#define CONFIG_SL351x_NAT_GRE
#define CONFIG_SL351x_TCP_UDP_RULE_ID	1	//rule 0 is used by ipsec 
#define CONFIG_SL351x_GRE_RULE_ID		2
#else
#define CONFIG_SL351x_NAT_TCP_UDP
//#define CONFIG_SL351x_NAT_GRE
#define CONFIG_SL351x_TCP_UDP_RULE_ID	0
#define CONFIG_SL351x_GRE_RULE_ID		0
#endif

#define	nat_printf
//#define	nat_printf					printk
//#define NAT_DEBUG_MSG
 
#define NAT_FTP_CTRL_PORT 			(21)	// TCP
#define NAT_H323_PORT				(1720)	// TCP
#define NAT_T120_PORT				(1503)	// TCP
#define NAT_PPTP_PORT				(1723)	// TCP
#define NAT_TFTP_PORT 				(69)	// UDP
#define NAT_DNS_PORT 				(53)	// UDP
#define NAT_NTP_PORT				(123)	// UDP
#define NAT_RAS_PORT				(1719)	// UDP
#define NAT_BOOTP67_PORT			(67)	// UDP
#define NAT_BOOTP68_PORT			(68)	// UDP

#define NAT_TCP_PORT_MAX			64
#define NAT_UDP_PORT_MAX			64
#define MIN_READ					0xf6000000
#define MAX_READ					0xf7000000
#define	LAN_VLAN_ID					2
#define DMZ_VLAN_ID					3

#define GRE_PROTOCOL				(0x880b)
#define GRE_PROTOCOL_SWAP			__constant_htons(0x880b)

#if defined (VITESSE_G5SWITCH) || (CONFIG_RTL8366SR_PHY)
extern int Giga_switch;
#endif

#ifdef GMAC_DEBUG_U	
extern int gmac_dump_rxpkt;
extern int gmac_dump_txpkt;
#endif
typedef struct
{
	u16		flags_ver;
	u16		protocol;
	u16		payload_length;
	u16		call_id;
	u32		seq;
	u32		ack;
} GRE_PKTHDR_T;
	
/*----------------------------------------------------------------------
* NAT Configuration
* 
* Note: Any change for network setting, the NAT configuration should 
*       be changed also.
*	cfg->lan_port	0 if GMAC-0, 1: if GMAC-1
*	cfg->wan_port	0 if GMAC-0, 1: if GMAC-1
*	cfg->lan_ipaddr, cfg->lan_gateway, cfg->lan_netmask
*	cfg->wan_ipaddr, cfg->wan_gateway, cfg->wan_netmask
*	
*----------------------------------------------------------------------*/
NAT_CFG_T 		nat_cfg;
static int		nat_initialized;
u32 			nat_collision;

#ifdef CONFIG_SL351x_NAT_TCP_UDP
static u16		fixed_tcp_port_list[]={NAT_FTP_CTRL_PORT,
							   			NAT_H323_PORT,
							   			// NAT_T120_PORT,
							   			NAT_PPTP_PORT,
										0};
static u16		fixed_udp_port_list[]={NAT_DNS_PORT,
									  	NAT_NTP_PORT,
									  	NAT_TFTP_PORT,
										NAT_RAS_PORT,
									  	NAT_BOOTP67_PORT,
									  	NAT_BOOTP68_PORT,
									   	0};
#endif									   	

// #define _HAVE_DYNAMIC_PORT_LIST
#ifdef _HAVE_DYNAMIC_PORT_LIST									   	
static u16		dynamic_tcp_port_list[NAT_TCP_PORT_MAX+1];
static u16		dynamic_udp_port_list[NAT_UDP_PORT_MAX+1]};
#endif

wait_queue_head_t url_block_wait;
unsigned char block_url_info[sizeof(struct iphdr)+sizeof(struct tcphdr)];
unsigned int url_block_len;

/*----------------------------------------------------------------------
* dm_byte
*----------------------------------------------------------------------*/
int sl351x_nat_tcp_udp_output(struct sk_buff *skb, int port);
int sl351x_nat_udp_output(struct sk_buff *skb, int port);
int sl351x_nat_gre_output(struct sk_buff *skb, int port);

extern int mac_set_rule_reg(int mac, int rule, int enabled, u32 reg0, u32 reg1, u32 reg2);
extern void hash_dump_entry(int index);
extern void mac_get_hw_tx_weight(struct net_device *dev, char *weight);
extern void mac_set_hw_tx_weight(struct net_device *dev, char *weight);
extern int phy_read(unsigned char port_no,unsigned char reg);
extern void phy_write(unsigned char port_no,unsigned char reg,unsigned int val);
extern void phy_write_masked(unsigned char port_no,unsigned char reg,unsigned int val,unsigned int mask);

#ifdef SL351x_NAT_TEST_BY_SMARTBITS
static void nat_init_test_entry(void);
#endif

extern unsigned int mii_read(unsigned char phyad,unsigned char regad);
extern void mii_write(unsigned char phyad,unsigned char regad,unsigned int value);
extern void dm_long_1(u32 location, int length);
extern void dm_short(u32 location, int length);
extern void dm_byte(u32 location, int length);
/*----------------------------------------------------------------------
* sl351x_nat_init
*	initialize a NAT matching rule
*	Called by SL351x Driver
*		key		: port, protocol, Sip, Dip, Sport, Dport
*		Action	: Srce Q: HW Free Queue,
*				  Dest Q: HW TxQ
*				  Change DA
*				  Change SA
*                 Change Sip or Dip
*    			  Change Sport or Dport
*----------------------------------------------------------------------*/
void sl351x_nat_init(void)
{
	int					rc;
	GMAC_MRxCR0_T		mrxcr0;
	GMAC_MRxCR1_T		mrxcr1;
	GMAC_MRxCR2_T		mrxcr2;
	NAT_CFG_T			*cfg;
	
	if (nat_initialized)
		return;
		
	nat_initialized = 1;
		
	if ((sizeof(NAT_HASH_ENTRY_T) > HASH_MAX_BYTES) ||
		(sizeof(GRE_HASH_ENTRY_T) > HASH_MAX_BYTES))
	{
		nat_printf("NAT_HASH_ENTRY_T structure Size is too larger!\n");
		while(1);
	}
		
	cfg = (NAT_CFG_T *)&nat_cfg;
	memset((void *)cfg, 0, sizeof(NAT_CFG_T));
#ifdef _HAVE_DYNAMIC_PORT_LIST									   	
	memset((void *)dynamic_tcp_port_list, 0, sizeof(dynamic_tcp_port_list));
	memset((void *)dynamic_udp_port_list, 0, sizeof(dynamic_udp_port_list));
#endif

#if defined (VITESSE_G5SWITCH) || (CONFIG_RTL8366SR_PHY)
	if(Giga_switch)
	{
		cfg->enabled			= 1;
		cfg->tcp_udp_rule_id 	= CONFIG_SL351x_TCP_UDP_RULE_ID;
		cfg->gre_rule_id 		= CONFIG_SL351x_GRE_RULE_ID;
#ifdef CONFIG_RTL8366SR_PHY	
		cfg->lan_port			= 0;
		cfg->wan_port			= 1;		
#else				
		cfg->lan_port			= 1;
		cfg->wan_port			= 0;
#endif			
		cfg->default_hw_txq 	= 3;
		cfg->tcp_tmo_interval 	= 60;
		cfg->udp_tmo_interval 	= 180;
		cfg->gre_tmo_interval 	= 60;
	}
	else
	{
		cfg->enabled			= 1;
		cfg->tcp_udp_rule_id 	= CONFIG_SL351x_TCP_UDP_RULE_ID;
		cfg->gre_rule_id 		= CONFIG_SL351x_GRE_RULE_ID;
		cfg->lan_port			= 0;
		cfg->wan_port			= 1;
		cfg->default_hw_txq 	= 3;
		cfg->tcp_tmo_interval 	= 60;
		cfg->udp_tmo_interval 	= 180;
		cfg->gre_tmo_interval 	= 60;
		
	}
//joel modify
#else
	cfg->enabled			= 0;
	cfg->tcp_udp_rule_id 	= CONFIG_SL351x_TCP_UDP_RULE_ID;
	cfg->gre_rule_id 		= CONFIG_SL351x_GRE_RULE_ID;
	cfg->lan_port			= 1;
	cfg->wan_port			= 0;
	cfg->default_hw_txq 	= 3;
	cfg->tcp_tmo_interval 	= 60;
	cfg->udp_tmo_interval 	= 180;
	cfg->gre_tmo_interval 	= 60;
//joel end	
#endif

#ifndef SKIP_NAT_CFG	
#if 1	//	debug purpose
	cfg->ipcfg[0].total				= 1;
	cfg->ipcfg[0].entry[0].ipaddr	= IPIV(192,168,2,92);
	cfg->ipcfg[0].entry[0].netmask	= IPIV(255,255,255,0);
	cfg->ipcfg[1].total				= 1;
	cfg->ipcfg[1].entry[0].ipaddr	= IPIV(192,168,1,200);
	cfg->ipcfg[1].entry[0].netmask	= IPIV(255,255,255,0);
#endif

#if 0
	cfg->xport.total = 0;
#else
	cfg->xport.total = 5;
	
	// H.323/H.225 Call setup
	cfg->xport.entry[0].protocol = IPPROTO_TCP;
	cfg->xport.entry[0].sport_start = 0;
	cfg->xport.entry[0].sport_end = 0;
	cfg->xport.entry[0].dport_start = 1720;
	cfg->xport.entry[0].dport_end = 1720;
	cfg->xport.entry[1].protocol = IPPROTO_TCP;
	cfg->xport.entry[1].sport_start = 1720;
	cfg->xport.entry[1].sport_end = 1720;
	cfg->xport.entry[1].dport_start = 0;
	cfg->xport.entry[1].dport_end = 0;
	
	// RAS Setup
	cfg->xport.entry[2].protocol = IPPROTO_UDP;
	cfg->xport.entry[2].sport_start = 0;
	cfg->xport.entry[2].sport_end = 0;
	cfg->xport.entry[2].dport_start = 1719;
	cfg->xport.entry[2].dport_end = 1719;
	cfg->xport.entry[3].protocol = IPPROTO_UDP;
	cfg->xport.entry[3].sport_start = 1719;
	cfg->xport.entry[3].sport_end = 1719;
	cfg->xport.entry[3].dport_start = 0;
	cfg->xport.entry[3].dport_end = 0;
	cfg->xport.entry[4].protocol = IPPROTO_TCP;
	cfg->xport.entry[4].sport_start = 0;
	cfg->xport.entry[4].sport_end = 0;
	cfg->xport.entry[4].dport_start = 80;
	cfg->xport.entry[4].dport_end = 80;
	
#endif	
#endif  // SKIP_NAT_CFG

#ifdef CONFIG_SL351x_NAT_TCP_UDP
	mrxcr0.bits32 = 0;
	mrxcr1.bits32 = 0;
	mrxcr2.bits32 = 0;
	mrxcr0.bits.port = 1;
	mrxcr0.bits.priority = 1;
	mrxcr0.bits.l3 = 1;
	mrxcr0.bits.l4 = 1;
	mrxcr1.bits.sip = 1;
	mrxcr1.bits.dip = 1;
	mrxcr1.bits.l4_byte0_15 = 0x0f;	// Byte 0-3
	mrxcr0.bits.sprx = 3;
#ifdef CONFIG_SL351x_RTLDMZ
	mrxcr0.bits.l2 = 1;
	mrxcr0.bits.vlan = 1;	
//	mrxcr0.bits.pppoe = 1;
#endif		
	rc = mac_set_rule_reg(cfg->lan_port, cfg->tcp_udp_rule_id, 1, mrxcr0.bits32, mrxcr1.bits32, mrxcr2.bits32);
	if (rc < 0)
	{
		nat_printf("NAT Failed to set MAC-%d Rule %d!\n", cfg->lan_port, cfg->tcp_udp_rule_id);
	}

	if (cfg->lan_port != cfg->wan_port)
	{
		rc = mac_set_rule_reg(cfg->wan_port, cfg->tcp_udp_rule_id, 1, mrxcr0.bits32, mrxcr1.bits32, mrxcr2.bits32);
		if (rc < 0)
		{
			nat_printf("NAT Failed to set MAC-%d Rule %d!\n", cfg->wan_port, cfg->tcp_udp_rule_id);
		}
	}
#endif
#ifdef CONFIG_SL351x_NAT_GRE
	mrxcr0.bits32 = 0;
	mrxcr1.bits32 = 0;
	mrxcr2.bits32 = 0;
	mrxcr0.bits.port = 1;
	mrxcr0.bits.l3 = 1;
	mrxcr0.bits.l4 = 1;
	mrxcr1.bits.sip = 1;
	mrxcr1.bits.dip = 1;
	mrxcr1.bits.l4_byte0_15 = 0xcc;	// Byte 2, 3, 6, 7
	mrxcr0.bits.sprx = 4;			// see GMAC driver about SPR
#ifdef CONFIG_SL351x_RTLDMZ
	mrxcr0.bits.l2 = 1;
	mrxcr0.bits.vlan = 1;
	mrxcr0.bits.pppoe = 1;
#endif	
	
	rc = mac_set_rule_reg(cfg->lan_port, cfg->gre_rule_id, 1, mrxcr0.bits32, mrxcr1.bits32, mrxcr2.bits32);
	if (rc < 0)
	{
		nat_printf("NAT Failed to set MAC-%d Rule %d!\n", cfg->lan_port, cfg->gre_rule_id);
	}
		
	if (cfg->lan_port != cfg->wan_port)
	{
		rc = mac_set_rule_reg(cfg->wan_port, cfg->gre_rule_id, 1, mrxcr0.bits32, mrxcr1.bits32, mrxcr2.bits32);
		if (rc < 0)
		{
			nat_printf("NAT Failed to set MAC-%d Rule %d!\n", cfg->wan_port, cfg->gre_rule_id);
		}
	}
#endif

	init_waitqueue_head(&url_block_wait);
	
#ifdef SL351x_NAT_TEST_BY_SMARTBITS
	nat_init_test_entry();
#endif
}

/*----------------------------------------------------------------------
* nat_build_keys
*	Note: To call this routine, the key->rule_id MUST be zero
*----------------------------------------------------------------------*/
static inline int nat_build_keys(NAT_KEY_T *key)
{
	return hash_gen_crc16((unsigned char *)key, NAT_KEY_SIZE) & HASH_BITS_MASK;
}

/*----------------------------------------------------------------------
* gre_build_keys
*	Note: To call this routine, the key->rule_id MUST be zero
*----------------------------------------------------------------------*/
static inline int gre_build_keys(GRE_KEY_T *key)
{
	return hash_gen_crc16((unsigned char *)key, GRE_KEY_SIZE) & HASH_BITS_MASK;
}

/*----------------------------------------------------------------------
* nat_write_hash_entry
*----------------------------------------------------------------------*/
static inline int nat_write_hash_entry(int index, void *hash_entry)
{
	int		i;
	u32		*srcep, *destp, *destp2;
	
	srcep = (u32 *)hash_entry;
	destp = destp2 = (u32 *)&hash_tables[index][0];
	
	for (i=0; i<(NAT_HASH_ENTRY_SIZE/sizeof(u32)); i++)
		*destp++ = *srcep++;
	
	consistent_sync(destp2, NAT_HASH_ENTRY_SIZE, PCI_DMA_TODEVICE);
	return 0;
}

/*----------------------------------------------------------------------
* gre_write_hash_entry
*----------------------------------------------------------------------*/
static inline int gre_write_hash_entry(int index, void *hash_entry)
{
	int		i;
	u32		*srcep, *destp, *destp2;
	
	srcep = (u32 *)hash_entry;
	destp = destp2 = (u32 *)&hash_tables[index][0];
	
	for (i=0; i<(GRE_HASH_ENTRY_SIZE/sizeof(u32)); i++)
		*destp++ = *srcep++;
	
	consistent_sync(destp2, GRE_HASH_ENTRY_SIZE, PCI_DMA_TODEVICE);
	return 0;
}

/*----------------------------------------------------------------------
* sl351x_nat_find_ipcfg
*	return NULL if not found
*----------------------------------------------------------------------*/
static NAT_IP_ENTRY_T *sl351x_nat_find_ipcfg(u32 ipaddr, int port)
{
	int				i;
	NAT_IP_ENTRY_T	*ipcfg;
	
	ipcfg = (NAT_IP_ENTRY_T *)&nat_cfg.ipcfg[port].entry[0];
	for (i=0; i<nat_cfg.ipcfg[port].total; i++, ipcfg++)
	{
		if (ipaddr == ipcfg->ipaddr)
		{
			return ipcfg;
		}
	}
	return NULL;
}

/*----------------------------------------------------------------------
* sl351x_nat_assign_qid
*----------------------------------------------------------------------*/
static int sl351x_nat_assign_qid(u8 proto, u32 sip, u32 dip, u16 sport, u16 dport)
{
	int 				i, total, qid;
	NAT_WRULE_ENTRY_T	*entry;
	
	for (qid = 0; qid<CONFIG_NAT_TXQ_NUM; qid++)
	{
		if (qid == nat_cfg.default_hw_txq)
			continue;
			
		entry = (NAT_WRULE_ENTRY_T *)&nat_cfg.wrule[qid].entry[0];
		total = nat_cfg.wrule[qid].total;
		for (i=0; i<total; i++, entry++)
		{
			if (!entry->protocol || entry->protocol==proto)
			{
				//if (!entry->sip_start && !entry->dip_start && !entry->sport_start && !entry->dport_start)
				//	continue; // UI take care
				if (entry->sip_start && !((sip >= entry->sip_start) && 
									   (sip <= entry->sip_end)))
					continue;
				if (entry->dip_start && !((dip >= entry->dip_start) && 
									   (dip <= entry->dip_end)))
					continue;
				if (entry->sport_start && !((sport >= entry->sport_start) && 
									   (sport <= entry->sport_end)))
					continue;
				if (entry->dport_start && !((dport >= entry->dport_start)
					 			       && (dport <= entry->dport_end)))
					continue;
				return qid;
			}
		}
	}
	return nat_cfg.default_hw_txq;
}

/*----------------------------------------------------------------------
* sl351x_hash_scan_and_invalidate
* Invalidate HW NAT session for ioctl parameter of nat_cfg !
*----------------------------------------------------------------------*/
int sl351x_hash_scan_and_invalidate(u32 src,u32 dst, u16 proto, u32 sport, u32 dport)
{	
	int					 i,cnt = 0,clean = 0;
	NAT_HASH_ENTRY_T 	*nat_entry;
	
	for(i = 0;i < HASH_TOTAL_ENTRIES; i++)	// scan hash table
	{	
		if(hash_get_valid_flag(i))		// compare if valid bit on
		{	
			nat_entry = hash_get_entry(i);
			if((src != 0) && (dst != 0))	// both sip,dip are specified
			{	
				if((nat_entry->key.sip == src) && (nat_entry->key.dip == dst)) 
				{
					if(nat_entry->key.ip_protocol == proto){
						if(sport && dport)	// both port are specified and match
						{		
							if((nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16)) && \
								(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16)) )
								clean = 1;
						}
						else if(sport)		// only sport specified
						{			 
							if(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16) )
								clean = 1;
						}
						else if(dport)		// only dport specified
						{			
							if(nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16))
								clean = 1;
							else			// no port specified
								clean = 1;
						}
					}
					else if(proto == 0)
						clean = 1;
				}
			}
			else if(src != 0)				// only sip specified
			{		
				if( nat_entry->key.sip == src)
				{
					if((nat_entry->key.ip_protocol == proto)){
						if(sport && dport)	// both specified and match
						{		
							if((nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16)) && \
								(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16)))
								clean = 1;
						}
						else if(sport)		// only sport specified 
						{			
							if(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16) )
								clean = 1;
						}
						else if(dport)		// only dport specified
						{			
							if(nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16))
								clean = 1;
						}
						else{				// no port specified
							clean = 1;
						}
					}
					else if(proto == 0)
						clean = 1;
				}
			}
			else if(dst != 0)				// only dip specified
			{		
				if(nat_entry->key.dip == dst)
				{
					if(nat_entry->key.ip_protocol == proto){
						if(sport && dport)	// both specified and match
						{		
							if((nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16)) && \
								(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16)))
								clean = 1;
						}
						else if(sport)		// only sport specified
						{			 
							if(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16))
								clean = 1;
						}
						else if(dport)		// only dport specified
						{			
							if(nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16))
								clean = 1;
						}
						else{				// no port specified
							clean = 1;
						}
					}
					else if(proto == 0)
						clean = 1;
				}
			}
			else						// no ip specified
			{	
				if(proto == 0)			
					clean = 1 ;
				if( nat_entry->key.ip_protocol == proto)
				{
					if(sport && dport)	// both specified and match
					{		
						if((nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16)) && \
							(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16)) )
							clean = 1;
					}
					else if(sport)		// only sport specified
					{			 
						if(nat_entry->key.sport >= (sport & 0xFF) && nat_entry->key.sport <= ((sport & 0xFF00) >> 16) )
							clean = 1;
					}
					else if(dport)		// only dport specified
					{			
						if(nat_entry->key.dport >= (dport & 0xFF) && nat_entry->key.dport <= ((dport & 0xFF00) >> 16))
							clean = 1;
					}
					else{				// no port specified
						clean = 1;
					}
				}
			}
			if(clean == 1){
				hash_invalidate_entry(i);
				hash_nat_disable_owner(i);
				cnt++;
			}
			clean = 0;
		}
	}
}

/*----------------------------------------------------------------------
* sl351x_hash_invalid_all
* Invalidate all HW NAT session if "iptalbes -F" was issued !
*----------------------------------------------------------------------*/
int sl351x_hash_invalid_all(void){
	
	u32 *v_ptr, i;
	//printk("Flush all!\n");	
	v_ptr = (volatile u32 *) TOE_V_BIT_BASE;
	
	for (i=0; i < HASH_TOTAL_ENTRIES/32; i++)
		*v_ptr++ = 0;
}
EXPORT_SYMBOL(sl351x_hash_invalid_all);


/*----------------------------------------------------------------------
* sl351x_nat_input
*	Handle NAT input frames
*	Called by SL351x Driver - Handle Default Rx Queue
*	Notes: The caller must make sure that the l3off & l4offset should not be zero.
*	SL351x NAT Frames should meet the following conditions:
*	1. TCP or UDP frame
*	2. Cannot be special ALGs ports which TCP/UDP data is updated
*	3. LAN-IN Frames: 
*		Source IP is in the LAN subnet and Destination is not in the LAN subnet
*	4. WAN-IN Frames
*		Destination IP is in the WAN port IP
*	
*	Example Ports
*	1. TCP/UDP data is updated
*		(a) FTP Control Packet
*		(b) VoIP Packets
*		(c) etc. (add in future)
*	2. UDP Low packet rate, not worth
*		(b) TFTP Destination Port is 69
*		(b) DNS  53
*		(c) NTP  123
*		(d) etc. (add in future)
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RTLDMZ 
void sl351x_nat_input(struct sk_buff *skb, int port, void *l3off, void *l4off, unsigned short rx_vid)
#else
void sl351x_nat_input(struct sk_buff *skb, int port, void *l3off, void *l4off)
#endif
{
	int 				i;
	u32					sip, dip;
	u16					sport, dport;
	int 				found;
	struct ethhdr		*ether_hdr;
	struct pppoe_hdr	*pppoe_hdr;
	u16					ppp_proto;
#ifndef SKIP_NAT_CFG	
	NAT_IP_ENTRY_T		*ipcfg;
	NAT_XPORT_ENTRY_T	*xentry;
#endif	
	struct iphdr		*ip_hdr;
	struct tcphdr		*tcp_hdr;
	NAT_CB_T			*nat_cb;
	u8					proto, pppoe_frame=0;
	NAT_CFG_T			*cfg;
	GRE_PKTHDR_T		*gre_hdr;
#ifdef CONFIG_SL351x_NAT_TCP_UDP
	u16 				*port_ptr;
#endif
#ifdef CONFIG_SL351x_RTLDMZ
	unsigned short		nat_cb_vid,vid_skb = 0;
#endif	
	
	cfg = (NAT_CFG_T *)&nat_cfg;
#ifndef SKIP_NAT_CFG	
	if (!cfg->enabled || !cfg->ipcfg[port].total)
		return;
#endif

#ifdef CONFIG_SL351x_RTLDMZ 
	if (!port)
	{
		ip_hdr = (struct iphdr *)&(skb->data[14]);
		tcp_hdr = (struct tcphdr *)&(skb->data[34]);
		vid_skb = rx_vid;
	}
	else
	{
		ip_hdr = (struct iphdr *)&(skb->data[(u32)l3off]);
		tcp_hdr = (struct tcphdr *)&(skb->data[(u32)l4off]);
	}
#else	
	ip_hdr = (struct iphdr *)&(skb->data[(u32)l3off]);
	tcp_hdr = (struct tcphdr *)&(skb->data[(u32)l4off]);
#endif	

	proto = ip_hdr->protocol;
	gre_hdr = (GRE_PKTHDR_T *)tcp_hdr;
	sport = ntohs(tcp_hdr->source);
	dport = ntohs(tcp_hdr->dest);
	
	sip = ntohl(ip_hdr->saddr);
	dip = ntohl(ip_hdr->daddr);
	
	if (dip == IPIV(255,255,255,255))
		return;
				
//+++joel add for the multicast do not in hardware NAT
	if(skb->data[0]&0x01 == 0x01 && proto == IPPROTO_UDP)/*is UDP multicast do not set to hw nat*/
		return;
//---joel

#ifndef SKIP_NAT_CFG
#ifdef CONFIG_SL351x_RTLDMZ 			
	if ((port == cfg->lan_port) && (rx_vid == 0x2002))
#else
	if (port == cfg->lan_port)
#endif	
	{
		ipcfg = (NAT_IP_ENTRY_T *)&cfg->ipcfg[port].entry[0];
		for (i=0, found=0; i<cfg->ipcfg[port].total; i++, ipcfg++)
		{
			u32 subnet = ipcfg->ipaddr & ipcfg->netmask;
			if (((sip & ipcfg->netmask) == subnet) &&
				((dip & ipcfg->netmask) != subnet))
			{
				found = 1;
				break;
			}
		}
		if (!found)
			return;
	}
	else
#endif //SKIP_NAT_CFG  	
	{
#ifndef _NOT_CHECK_SIP_DIP	// enable it if know and get the wan ip address
		if (!sl351x_nat_find_ipcfg(dip, port))
		{
			printk("WAN->LAN Incorrect Dip %d.%d.%d.%d\n", HIPQUAD(dip));
			return;
		}
#endif		
		ether_hdr = (struct ethhdr *)skb->data;
		pppoe_hdr = (struct pppoe_hdr *)(ether_hdr + 1);
		ppp_proto = *(u16 *)&pppoe_hdr->tag[0];
		if (ether_hdr->h_proto == __constant_htons(ETH_P_PPP_SES)	// 0x8864
			&& ppp_proto == __constant_htons(PPP_IP) )				// 0x21
		{
			pppoe_frame = 1;
		}
	}

#ifdef CONFIG_SL351x_NAT_TCP_UDP
	if (proto == IPPROTO_TCP)
	{

#ifdef	NAT_DEBUG_MSG
#ifdef CONFIG_SL351x_RTLDMZ
		if (port == cfg->lan_port)
			nat_printf("From   GMAC-%d: 0x%-4X rx_vid %d TCP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),vid_skb,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		else
			nat_printf("From   GMAC-%d: 0x%-4X TCP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);		
				
#else				
		nat_printf("From   GMAC-%d: 0x%-4X TCP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);		
#endif	
		if (tcp_flag_word(tcp_hdr) & TCP_FLAG_SYN) nat_printf(" SYN");
		if (tcp_flag_word(tcp_hdr) & TCP_FLAG_FIN) nat_printf(" FIN");
		if (tcp_flag_word(tcp_hdr) & TCP_FLAG_RST) nat_printf(" RST");
		if (tcp_flag_word(tcp_hdr) & TCP_FLAG_ACK) nat_printf(" ACK");
		nat_printf("\n");
#endif				
		// if (tcp_flag_word(tcp_hdr) & (TCP_FLAG_SYN | TCP_FLAG_FIN | TCP_FLAG_RST))
		if (tcp_flag_word(tcp_hdr) & (TCP_FLAG_SYN))
			return;
		port_ptr = fixed_tcp_port_list;
		for (i=0; *port_ptr; i++, port_ptr++)
		{
			if (sport == *port_ptr || dport == *port_ptr)
				return;
		}
#ifdef _HAVE_DYNAMIC_PORT_LIST									   	
		port_ptr = dynamic_tcp_port_list;
		for (i=0; *port_ptr; i++, port_ptr++)
		{
			if (sport == *port_ptr || dport == *port_ptr)
				return;
		}
#endif		
	}
	else if (proto == IPPROTO_UDP)
	{
#ifdef	NAT_DEBUG_MSG
#ifdef CONFIG_SL351x_RTLDMZ
		if (port == cfg->lan_port)
			nat_printf("From   GMAC-%d: 0x%-4X rx_vid %d UDP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),vid_skb,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		else
			nat_printf("From   GMAC-%d: 0x%-4X UDP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		nat_printf("\n");		
#else	
		nat_printf("From   GMAC-%d: 0x%-4X UDP %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		nat_printf("\n");
#endif		
#endif	//NAT_DEBUG_MSG		
		port_ptr = fixed_udp_port_list;
		for (i=0; *port_ptr; i++, port_ptr++)
		{
			if (sport == *port_ptr || dport == *port_ptr)
				return;
		}
#ifdef _HAVE_DYNAMIC_PORT_LIST									   	
		port_ptr = dynamic_udp_port_list;
		for (i=0; *port_ptr; i++, port_ptr++)
		{
			if (sport == *port_ptr || dport == *port_ptr)
				return;
		}
#endif		
	}
	else 
#endif	// CONFIG_SL351x_NAT_TCP_UDP
#ifdef CONFIG_SL351x_NAT_GRE
	if (proto == IPPROTO_GRE)
	{
		if (gre_hdr->protocol != GRE_PROTOCOL_SWAP)
			return;
#ifdef	NAT_DEBUG_MSG
#ifdef CONFIG_SL351x_RTLDMZ
		nat_printf("From   GMAC-%d: 0x%-4X rx_vid %d GRE %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id),vid_skb,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		nat_printf("\n");		
#else
		nat_printf("From   GMAC-%d: 0x%-4X GRE %d.%d.%d.%d [%d] --> %d.%d.%d.%d",
				port, ntohs(ip_hdr->id),
				NIPQUAD(ip_hdr->saddr), ntohs(gre_hdr->call_id), 
				NIPQUAD(ip_hdr->daddr));
		nat_printf("\n");
#endif	
#endif	//NAT_DEBUG_MSG			
	}
	else
#endif	//CONFIG_SL351x_NAT_GRE
		return;
	
#ifndef SKIP_NAT_CFG	
	// check xport list
	xentry = (NAT_XPORT_ENTRY_T *)&cfg->xport.entry[0];
	for (i=0; i<cfg->xport.total; i++, xentry++)
	{
		if (!xentry->protocol || xentry->protocol == proto)
		{
			//if (!xentry->sport_start && !xentry->dport_start) // UI take care
			//	continue;
			if (xentry->sport_start && !((sport >= xentry->sport_start) && 
									   (sport <= xentry->sport_end)))
				continue;
			if (xentry->dport_start && !((dport >= xentry->dport_start)
					 			       && (dport <= xentry->dport_end)))
				continue;
			return;
		}
	}
#endif  //SKIP_NAT_CFG
	
	nat_cb = NAT_SKB_CB(skb);
	if (((u32)nat_cb & 3))
	{
		nat_printf("%s ERROR! nat_cb is not alignment!!!!!!\n", __func__);
		return;
	}

	nat_cb->tag = NAT_CB_TAG;
	memcpy(nat_cb->sa, skb->data+6, 6);
	nat_cb->sip = ip_hdr->saddr;
	nat_cb->dip = ip_hdr->daddr;
	if (proto == IPPROTO_GRE)
	{
		nat_cb->sport = gre_hdr->protocol;
		nat_cb->dport = gre_hdr->call_id;
	}
	else
	{
		nat_cb->sport = tcp_hdr->source;
		nat_cb->dport = tcp_hdr->dest;
	}
	nat_cb->pppoe_frame = pppoe_frame;

#ifdef 	CONFIG_SL351x_RTLDMZ 	
	if (port == cfg->lan_port)
	{
  		nat_cb->reserved[0] = vid_skb & 0x00FF;
   		nat_cb->reserved[1] = (vid_skb & 0xFF00) >> 8;	
   		nat_cb_vid = nat_cb->reserved[0] | nat_cb->reserved[1] ;
   	}
#endif
}

/*----------------------------------------------------------------------
* sl351x_nat_output
*	Handle NAT output frames
*	Called by SL351x Driver - Transmit
*
*	1. If not SL351x NAT frames, return FALSE
*	2. LAN-to-WAN frames
*		(1) Sip must be WAN IP
*	3. If TCP SY/RST/FIN frame, return
*	4. Build the hash key and get the hash index
*	5. If V-Bit is ON, return.
*	6. Write hash entry and validate it
*		
*----------------------------------------------------------------------*/
int sl351x_nat_output(struct sk_buff *skb, int port)
{
	struct iphdr		*ip_hdr;
	u8					proto;
	NAT_CB_T			*nat_cb;
	NAT_CFG_T			*cfg;

	nat_cb = NAT_SKB_CB(skb);
	cfg = (NAT_CFG_T *)&nat_cfg;
	
#ifdef CONFIG_SL351x_RTLDMZ	
	if (port == cfg->lan_port)	//GMAC_PORT0
		ip_hdr = (struct iphdr *)&(skb->data[18]);
	else
		ip_hdr = (struct iphdr *)skb->h.ipiph;	
#else	
	//ip_hdr = (struct iphdr *)skb->h.ipiph; //mark by siyou. 2008/12/31 01:56pm
	ip_hdr = (struct iphdr *)skb->nh.iph;
#endif	
	
	if (nat_cb->tag != NAT_CB_TAG)
		return 0;
	
	if (((u32)nat_cb & 3))
	{
		nat_printf("%s ERROR! nat_cb is not alignment!!!!!!\n", __func__);
		return 0;
	}
	
	//joel for protect kernel panic
	if(!ip_hdr)
	{
		printk("sl351x_nat_output ip_hdr error \n");
		return 0;
	}
	proto = ip_hdr->protocol;
	
	if ((proto == IPPROTO_UDP) && (ip_hdr->frag_off & htons(IP_MF|IP_OFFSET)))
	{
		nat_printf("UDP IP Fragmenation\n"); 
		return 0;
	}
	
	switch (proto)
	{
		case IPPROTO_TCP:
		case IPPROTO_UDP:
			return sl351x_nat_tcp_udp_output(skb, port);
		case IPPROTO_GRE:
			return sl351x_nat_gre_output(skb, port);
	}
	return 0;
}

/*----------------------------------------------------------------------
* sl351x_nat_tcp_udp_output
*	Handle NAT TCP/UDP output frames
*----------------------------------------------------------------------*/
int sl351x_nat_tcp_udp_output(struct sk_buff *skb, int port)
{
	u32					sip, dip;
	struct ethhdr		*ether_hdr;
	struct iphdr		*ip_hdr;
	struct tcphdr		*tcp_hdr;
	struct pppoe_hdr	*pppoe_hdr;
	NAT_CB_T			*nat_cb;
	NAT_CFG_T			*cfg;
	u8					proto;
	u16					sport, dport, ppp_proto;
	u32					hash_data[HASH_MAX_DWORDS];
	NAT_HASH_ENTRY_T	*hash_entry;
	int					hash_index;
	struct ip_conntrack *nat_ip_conntrack;
	enum ip_conntrack_info ctinfo;
#ifdef CONFIG_SL351x_RTLDMZ
	short              	nat_cb_vid,tx_vid;
#endif	

	nat_cb = NAT_SKB_CB(skb);
	cfg = (NAT_CFG_T *)&nat_cfg;
	
	ether_hdr = (struct ethhdr *)skb->data;
#ifdef CONFIG_SL351x_RTLDMZ		
	if (port == cfg->lan_port)	//GMAC_PORT0
		ip_hdr = (struct iphdr *)&(skb->data[18]);
	else
		ip_hdr = (struct iphdr *)skb->h.ipiph;
#else		
	//ip_hdr = (struct iphdr *)skb->h.ipiph; //mark by siyou 2008/12/31 01:57pm
	ip_hdr = (struct iphdr *)skb->nh.iph;
#endif	
	tcp_hdr = (struct tcphdr *)((u32)ip_hdr + (ip_hdr->ihl<<2));
	sip = ntohl(ip_hdr->saddr);
	dip = ntohl(ip_hdr->daddr);
	proto = ip_hdr->protocol;
	sport = ntohs(tcp_hdr->source);
	dport = ntohs(tcp_hdr->dest);

//+++ The HW NAT can't do DNA & SNAT at the same time. add by siyou 2009/1/15 04:19pm.
//    so don't let it enter hw nat when this case happen.
/* Fix with change the action definition in sl351x_hash_cfg.h. by siyou.2009/1/16 11:10am
change NAT_LAN2WAN_ACTIONS and NAT_WAN2LAN_ACTIONS

	if ( ntohl(nat_cb->sip) != sip && ntohl(nat_cb->dip) != dip )
	{
		//printk("HW NAT error: Do SNAT & DNAT at the same time\n");
		return 0;
	}
*/
//---- end add.

#ifdef CONFIG_SL351x_RTLDMZ	
	tx_vid = (*(skb->data + 0x0F)) | ((*(skb->data + 0x0E)) << 8);
	nat_cb_vid = nat_cb->reserved[0] | (nat_cb->reserved[1] << 8);
#endif 	

#ifdef	NAT_DEBUG_MSG
	{
#ifdef CONFIG_SL351x_RTLDMZ		
		if (port == cfg->lan_port)	//GMAC_PORT0	
			nat_printf("To   GMAC-%d: 0x%-4X [%d] tx_vid %d %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id), proto,tx_vid,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
		else
			nat_printf("To   GMAC-%d: 0x%-4X [%d] %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id), proto,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
#else
		nat_printf("To   GMAC-%d: 0x%-4X [%d] %d.%d.%d.%d [%d] --> %d.%d.%d.%d [%d]",
				port, ntohs(ip_hdr->id), proto,
				NIPQUAD(ip_hdr->saddr), sport, 
				NIPQUAD(ip_hdr->daddr), dport);
#endif	
		if (proto == IPPROTO_TCP)
		{
			if (tcp_flag_word(tcp_hdr) & TCP_FLAG_SYN) nat_printf(" SYN");
			if (tcp_flag_word(tcp_hdr) & TCP_FLAG_FIN) nat_printf(" FIN");
			if (tcp_flag_word(tcp_hdr) & TCP_FLAG_RST) nat_printf(" RST");
			if (tcp_flag_word(tcp_hdr) & TCP_FLAG_ACK) nat_printf(" ACK");
		}
		nat_printf("\n");
	}
#endif	//NAT_DEBUG_MSG			
	nat_ip_conntrack = ip_conntrack_get(skb, &ctinfo);
	if (!nat_ip_conntrack)
	{
		nat_printf("IP conntrack info is not found!\n");
		return 0;
	}
	// nat_printf("nat_ip_conntrack = 0x%x, status=0x%lx, ctinfo=%d\n", (u32)nat_ip_conntrack, nat_ip_conntrack->status, ctinfo);
	// if (nat_ip_conntrack->master || nat_ip_conntrack->helper)
	if (nat_ip_conntrack->helper)
	{
		nat_printf("Sport=%d Dport=%d master=0x%x, helper=0x%x\n", sport, dport, (u32)nat_ip_conntrack->master, (u32)nat_ip_conntrack->helper);
		return 0;
	}
	
	//if (proto == IPPROTO_TCP && !(nat_ip_conntrack->status & IPS_ASSURED))
	//	return 0;

#ifdef	NAT_DEBUG_MSG
	nat_printf("nat_ip_conntrack=0x%x, nat_cb->state=%d\n", (u32)nat_ip_conntrack, nat_cb->state);
	nat_printf("lan2wan_hash_index=%d,  wan2lan_hash_index=%d\n", nat_ip_conntrack->lan2wan_hash_index, nat_ip_conntrack->wan2lan_hash_index);
	nat_printf("lan2wan_collision=%d, wan2lan_collision=%d\n", nat_ip_conntrack->lan2wan_collision, nat_ip_conntrack->wan2lan_collision);
#endif
	if (proto == IPPROTO_TCP)
	{
		if (nat_cb->state >= TCP_CONNTRACK_FIN_WAIT && nat_cb->state <= TCP_CONNTRACK_CLOSE)
		{
			if 	(nat_ip_conntrack->lan2wan_hash_index)
			{
#ifdef	NAT_DEBUG_MSG
				nat_printf("Invalidate LAN->WAN hash entry %d\n", nat_ip_conntrack->lan2wan_hash_index - 1);
#endif
				hash_nat_disable_owner(nat_ip_conntrack->lan2wan_hash_index - 1);
				hash_invalidate_entry(nat_ip_conntrack->lan2wan_hash_index - 1);
				nat_ip_conntrack->lan2wan_hash_index = 0;
			}
			if 	(nat_ip_conntrack->wan2lan_hash_index)
			{
#ifdef	NAT_DEBUG_MSG
				nat_printf("Invalidate WAN->LAN hash entry %d\n", nat_ip_conntrack->wan2lan_hash_index - 1);
#endif
				hash_nat_disable_owner(nat_ip_conntrack->wan2lan_hash_index - 1);
				hash_invalidate_entry(nat_ip_conntrack->wan2lan_hash_index - 1);
				nat_ip_conntrack->wan2lan_hash_index = 0;
			}
#ifdef CONFIG_SL351x_RTLDMZ
			if 	(nat_ip_conntrack->lan2dmz_hash_index)
			{
#ifdef	NAT_DEBUG_MSG
				nat_printf("Invalidate LAN->DMZ hash entry %d\n", nat_ip_conntrack->lan2dmz_hash_index - 1);
#endif
				hash_nat_disable_owner(nat_ip_conntrack->lan2dmz_hash_index - 1);
				hash_invalidate_entry(nat_ip_conntrack->lan2dmz_hash_index - 1);
				nat_ip_conntrack->lan2dmz_hash_index = 0;
			}				
			if 	(nat_ip_conntrack->dmz2lan_hash_index)
			{
#ifdef	NAT_DEBUG_MSG
				nat_printf("Invalidate DMZ->LAN hash entry %d\n", nat_ip_conntrack->dmz2lan_hash_index - 1);
#endif
				hash_nat_disable_owner(nat_ip_conntrack->dmz2lan_hash_index - 1);
				hash_invalidate_entry(nat_ip_conntrack->dmz2lan_hash_index - 1);
				nat_ip_conntrack->dmz2lan_hash_index = 0;
			}
#endif	//CONFIG_SL351x_RTLDMZ	
			return 0;
				
		}
		else if (nat_cb->state != TCP_CONNTRACK_ESTABLISHED)
			return 0;
	}
	if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_SYN | TCP_FLAG_FIN | TCP_FLAG_RST)))
	{
	// if (proto == IPPROTO_TCP &&  (tcp_flag_word(tcp_hdr) & (TCP_FLAG_SYN)))
		return 0;
	}
	hash_entry = (NAT_HASH_ENTRY_T *)&hash_data;
	if (port == cfg->wan_port)	// LAN-to-WAN ,if enable DMZ remove vlan tag
	{
		if (nat_ip_conntrack->lan2wan_hash_index || nat_ip_conntrack->lan2wan_collision)
			return 0;
#ifndef _NOT_CHECK_SIP_DIP	// enable it if know and get the wan ip address	
		if (!sl351x_nat_find_ipcfg(sip, port))
		{
			nat_printf("LAN->WAN Incorrect Sip %d.%d.%d.%d\n", HIPQUAD(sip));
			return 0;
		}
#endif
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->lan_port;
		hash_entry->key.rule_id 	= 0;
#ifdef CONFIG_SL351x_RTLDMZ	    		   			
		hash_entry->key.vlan_id     = nat_cb_vid;
		hash_entry->key.pppoe_sid 	= 0;	
#endif  			
		hash_entry->key.ip_protocol = proto;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.sport 		= nat_cb->sport;
		hash_entry->key.dport 		= nat_cb->dport;		
		
		hash_index = nat_build_keys(&hash_entry->key);

#ifdef NAT_DEBUG_LAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			nat_ip_conntrack->lan2wan_collision = 1;
			nat_collision++;
#if 0
			if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_FIN | TCP_FLAG_RST)))
			{
				if (memcmp((void *)&hash_entry->key, hash_get_entry(hash_index), sizeof(NAT_KEY_T)) == 0)
				{			
   					hash_nat_disable_owner(hash_index);
 					hash_invalidate_entry(hash_index); // Must last one, else HW Tx fast SW
 					// nat_printf("Invalidate nat hash entry %d\n", hash_index);
 				}
			}
#endif	
			return 0;
		}
			
		// write hash entry
		hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
#ifdef CONFIG_SL351x_RTLDMZ	    		
		hash_entry->param.vlan = 0;
#endif   	
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = sport;
		hash_entry->param.Dport = dport;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		
		// check PPPoE 
		pppoe_hdr = (struct pppoe_hdr *)(ether_hdr + 1);
		ppp_proto = *(u16 *)&pppoe_hdr->tag[0];
		if (ether_hdr->h_proto == __constant_htons(ETH_P_PPP_SES)	// 0x8864
			&& ppp_proto == __constant_htons(PPP_IP) )				// 0x21
		{
#ifdef CONFIG_SL351x_RTLDMZ
			hash_entry->action.dword = NAT_PPPOE_LAN2WAN_DMZ_ACTIONS;
#else			
			hash_entry->action.dword = NAT_PPPOE_LAN2WAN_ACTIONS;
#endif			
			hash_entry->param.pppoe = htons(pppoe_hdr->sid);
		}
		else
		{
#ifdef CONFIG_SL351x_RTLDMZ
			hash_entry->action.dword = NAT_LAN2WAN_DMZ_ACTIONS;
#else
			hash_entry->action.dword = NAT_LAN2WAN_ACTIONS;
#endif						
			hash_entry->param.pppoe = 0;
		}
			
#ifdef CONFIG_SL351x_RTLDMZ	  
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid = TOE_GMAC1_HW_TXQ2_QID;
#else					
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid +=	(cfg->wan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;
#endif	
		hash_entry->tmo.counter = hash_entry->tmo.interval = 
						(proto == IPPROTO_TCP) ? cfg->tcp_tmo_interval : cfg->udp_tmo_interval;
		nat_write_hash_entry(hash_index, hash_entry);
		//nat_printf("%lu Validate a LAN hash entry %d\n", jiffies/HZ, hash_index);
		//hash_dump_entry(hash_index);
		hash_nat_enable_owner(hash_index);
		hash_validate_entry(hash_index); // Must last one, else HW Tx fast than SW 
 		nat_ip_conntrack->lan2wan_hash_index = hash_index + 1;
 		nat_ip_conntrack->hw_nat |= 1;
		return 0;
	}
	
#ifdef CONFIG_SL351x_RTLDMZ		
	if ((tx_vid == LAN_VLAN_ID) && (nat_cb_vid != 0x2003)) // WAN-to-LAN with vlan id 2 
	{
		if (nat_ip_conntrack->wan2lan_hash_index || nat_ip_conntrack->wan2lan_collision)
			return 0;
			
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->wan_port;
		hash_entry->key.rule_id 	= 0;
		hash_entry->key.vlan_id     = 0;
    	hash_entry->key.pppoe_sid 	= 0;			
		hash_entry->key.ip_protocol = proto;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.sport 		= nat_cb->sport;
		hash_entry->key.dport 		= nat_cb->dport;
		
		hash_index = nat_build_keys(&hash_entry->key);

#ifdef NAT_DEBUG_WAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			nat_ip_conntrack->wan2lan_collision = 1;
			nat_collision++;
#if 0
			if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_FIN | TCP_FLAG_RST)))
			{
				if (memcmp((void *)&hash_entry->key, hash_get_entry(hash_index), sizeof(NAT_KEY_T)) == 0)
				{			
   					hash_nat_disable_owner(hash_index);
 					hash_invalidate_entry(hash_index); // Must last one, else HW Tx fast SW
  					// nat_printf("Invalidate nat hash entry %d\n", hash_index);
				}
			}
#endif
			return 0;
		}
		
		// write hash entry
		hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = sport;
		hash_entry->param.Dport = dport;
		hash_entry->param.vlan = LAN_VLAN_ID;
		hash_entry->param.pppoe = 0;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_WAN2LAN_ACTIONS : NAT_WAN2LAN_ACTIONS;

		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid = TOE_GMAC0_HW_TXQ3_QID;
		hash_entry->action.bits.vlan = 1; 
		hash_entry->tmo.counter = hash_entry->tmo.interval =  
						(proto == IPPROTO_TCP) ? cfg->tcp_tmo_interval : cfg->udp_tmo_interval;
		nat_write_hash_entry(hash_index, hash_entry);
			
		//nat_printf("%lu Validate a WAN hash entry %d\n", jiffies/HZ, hash_index);
		//hash_dump_entry(hash_index);
   		hash_nat_enable_owner(hash_index);
 		hash_validate_entry(hash_index); // Must last one, else HW Tx fast SW
 		nat_ip_conntrack->wan2lan_hash_index = hash_index + 1;
 		nat_ip_conntrack->hw_nat |= 2;
		return 0;
	}
#endif	//CONFIG_SL351x_RTLDMZ	
#ifdef CONFIG_SL351x_RTLDMZ		
	if ((tx_vid == DMZ_VLAN_ID) && (nat_cb_vid != 0x2002))	// WAN-to-DMZ with vlan id 3 
#else	
	else // WAN-to-LAN
#endif	 
	{
		if (nat_ip_conntrack->wan2lan_hash_index || nat_ip_conntrack->wan2lan_collision)
			return 0;
			
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->wan_port;
		hash_entry->key.rule_id 	= 0;
#ifdef CONFIG_SL351x_RTLDMZ	
		hash_entry->key.vlan_id     = 0;
    	hash_entry->key.pppoe_sid 	= 0;			
#endif    	
		hash_entry->key.ip_protocol = proto;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.sport 		= nat_cb->sport;
		hash_entry->key.dport 		= nat_cb->dport;		
		
		hash_index = nat_build_keys(&hash_entry->key);

#ifdef NAT_DEBUG_WAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			nat_ip_conntrack->wan2lan_collision = 1;
			nat_collision++;
#if 0
			if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_FIN | TCP_FLAG_RST)))
			{
				if (memcmp((void *)&hash_entry->key, hash_get_entry(hash_index), sizeof(NAT_KEY_T)) == 0)
				{			
   					hash_nat_disable_owner(hash_index);
 					hash_invalidate_entry(hash_index); // Must last one, else HW Tx fast SW
  					// nat_printf("Invalidate nat hash entry %d\n", hash_index);
				}
			}
#endif
			return 0;
		}
		
		// write hash entry
		hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
#ifdef CONFIG_SL351x_RTLDMZ			
		hash_entry->param.vlan = DMZ_VLAN_ID;
#endif		 
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = sport;
		hash_entry->param.Dport = dport;
		hash_entry->param.pppoe = 0;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_WAN2LAN_ACTIONS : NAT_WAN2LAN_ACTIONS;
#ifdef CONFIG_SL351x_RTLDMZ	
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid += ((tx_vid == LAN_VLAN_ID)) ? TOE_GMAC0_HW_TXQ2_QID : TOE_GMAC0_HW_TXQ3_QID;
		hash_entry->action.bits.vlan = 1;  
#else 
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid += (cfg->lan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;
#endif
		hash_entry->tmo.counter = hash_entry->tmo.interval =  
						(proto == IPPROTO_TCP) ? cfg->tcp_tmo_interval : cfg->udp_tmo_interval;
		nat_write_hash_entry(hash_index, hash_entry);
			
		//nat_printf("%lu Validate a WAN hash entry %d\n", jiffies/HZ, hash_index);
		//hash_dump_entry(hash_index);
   		hash_nat_enable_owner(hash_index);
 		hash_validate_entry(hash_index); // Must last one, else HW Tx fast SW
 		nat_ip_conntrack->wan2lan_hash_index = hash_index + 1;
 		nat_ip_conntrack->hw_nat |= 2;
		return 0;
	}

#ifdef CONFIG_SL351x_RTLDMZ				
	if ((tx_vid == LAN_VLAN_ID) && (nat_cb_vid == 0x2003)) // DMZ-to-LAN ,eth2(DMZ) sent packet to eth0(LAN)
	{
		if (nat_ip_conntrack->lan2dmz_hash_index || nat_ip_conntrack->lan2dmz_collision)
			return 0;
			
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->lan_port;//GMAC_PORT0
		hash_entry->key.rule_id 	= 0;
		hash_entry->key.vlan_id     = nat_cb_vid;	//2003	    	
    	hash_entry->key.pppoe_sid 	= 0;			
		hash_entry->key.ip_protocol = proto;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.sport 		= nat_cb->sport;
		hash_entry->key.dport 		= nat_cb->dport;
		
		hash_index = nat_build_keys(&hash_entry->key);

#ifdef NAT_DEBUG_WAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			nat_ip_conntrack->lan2dmz_collision = 1;
			nat_collision++;
#if 0
			if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_FIN | TCP_FLAG_RST)))
			{
				if (memcmp((void *)&hash_entry->key, hash_get_entry(hash_index), sizeof(NAT_KEY_T)) == 0)
				{			
   					hash_nat_disable_owner(hash_index);
 					hash_invalidate_entry(hash_index); // Must last one, else HW Tx fast SW
  					// nat_printf("Invalidate nat hash entry %d\n", hash_index);
				}
			}
#endif
			return 0;
		}
		
		// write hash entry
		hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
		hash_entry->param.vlan = LAN_VLAN_ID;
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = sport;
		hash_entry->param.Dport = dport;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		
		if ( HIPQUAD1(hash_entry->key.sip) != HIPQUAD1(sip))	
			hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_DMZ2LAN_ACTIONS : NAT_DMZ2LAN_ACTIONS;
		else
			hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_WAN2LAN_ACTIONS : NAT_WAN2LAN_ACTIONS;
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid = TOE_GMAC0_HW_TXQ2_QID;
		hash_entry->action.bits.vlan = 1;  //insert vlan tag
		hash_entry->tmo.counter = hash_entry->tmo.interval =  
						(proto == IPPROTO_TCP) ? cfg->tcp_tmo_interval : cfg->udp_tmo_interval;
		nat_write_hash_entry(hash_index, hash_entry);
			
		//nat_printf("%lu Validate a WAN hash entry %d\n", jiffies/HZ, hash_index);
		//hash_dump_entry(hash_index);
 		hash_nat_enable_owner(hash_index);
 		hash_validate_entry(hash_index); // Must last one, else HW Tx fast SW
 		nat_ip_conntrack->lan2dmz_hash_index = hash_index + 1;
 		nat_ip_conntrack->hw_nat |= 3;
		return 0;
	}	

	if ((tx_vid == DMZ_VLAN_ID) && (nat_cb_vid == 0x2002)) // LAN-to-DMZ ,eth0 sent packet to eth2
	{
		if (nat_ip_conntrack->dmz2lan_hash_index || nat_ip_conntrack->dmz2lan_collision)
			return 0;
			
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->lan_port;//GMAC_PORT0
		hash_entry->key.rule_id 	= 0;
		hash_entry->key.vlan_id     = nat_cb_vid;	//2002    	
    	hash_entry->key.pppoe_sid 	= 0;			
		hash_entry->key.ip_protocol = proto;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.sport 		= nat_cb->sport;
		hash_entry->key.dport 		= nat_cb->dport;
		
		hash_index = nat_build_keys(&hash_entry->key);

#ifdef NAT_DEBUG_WAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			nat_ip_conntrack->dmz2lan_collision = 1;
			nat_collision++;
#if 0
			if (proto == IPPROTO_TCP && (tcp_flag_word(tcp_hdr) & (TCP_FLAG_FIN | TCP_FLAG_RST)))
			{
				if (memcmp((void *)&hash_entry->key, hash_get_entry(hash_index), sizeof(NAT_KEY_T)) == 0)
				{			
   					hash_nat_disable_owner(hash_index);
 					hash_invalidate_entry(hash_index); // Must last one, else HW Tx fast SW
  					// nat_printf("Invalidate nat hash entry %d\n", hash_index);
				}
			}
#endif
			return 0;
		}
		
		// write hash entry
		hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
		hash_entry->param.vlan = DMZ_VLAN_ID;
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = sport;
		hash_entry->param.Dport = dport;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		
		if ( HIPQUAD1(hash_entry->key.dip) != HIPQUAD1(dip))	//LAN to WAN forward to DMZ
			hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_LAN2DMZ_ACTIONS : NAT_LAN2DMZ_ACTIONS;
		else													
			hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_LAN2WAN_ACTIONS : NAT_LAN2WAN_ACTIONS;
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(proto, sip, dip, sport, dport);
		hash_entry->action.bits.dest_qid = TOE_GMAC0_HW_TXQ3_QID;
		hash_entry->action.bits.vlan = 1;  //insert vlan tag
		hash_entry->tmo.counter = hash_entry->tmo.interval =  
						(proto == IPPROTO_TCP) ? cfg->tcp_tmo_interval : cfg->udp_tmo_interval;
		nat_write_hash_entry(hash_index, hash_entry);
			
		//nat_printf("%lu Validate a WAN hash entry %d\n", jiffies/HZ, hash_index);
		//hash_dump_entry(hash_index);
   		hash_nat_enable_owner(hash_index);
 		hash_validate_entry(hash_index); // Must last one, else HW Tx fast SW
 		nat_ip_conntrack->dmz2lan_hash_index = hash_index + 1;
 		nat_ip_conntrack->hw_nat |= 4;
		return 0;
	}	
#endif 	//CONFIG_SL351x_RTLDMZ 	
	return 0;
}

/*----------------------------------------------------------------------
* sl351x_nat_gre_output
*	Handle NAT GRE output frames
*----------------------------------------------------------------------*/
int sl351x_nat_gre_output(struct sk_buff *skb, int port)
{
	u32					sip, dip;
	struct ethhdr		*ether_hdr;
	struct iphdr		*ip_hdr;
	struct pppoe_hdr	*pppoe_hdr;
	GRE_PKTHDR_T		*gre_hdr;
	NAT_CB_T			*nat_cb;
	NAT_CFG_T			*cfg;
	u16					ppp_proto;
	u32					hash_data[HASH_MAX_DWORDS];
	GRE_HASH_ENTRY_T	*hash_entry;
	int					hash_index;
	struct ip_conntrack *nat_ip_conntrack;
	enum ip_conntrack_info ctinfo;
	
	nat_cb = NAT_SKB_CB(skb);
	cfg = (NAT_CFG_T *)&nat_cfg;
	
	ether_hdr = (struct ethhdr *)skb->data;
	//ip_hdr = (struct iphdr *)skb->h.ipiph; //mark by siyou.
	ip_hdr = (struct iphdr *)skb->nh.iph;
	gre_hdr = (GRE_PKTHDR_T *)((u32)ip_hdr + (ip_hdr->ihl<<2));
	sip = ntohl(ip_hdr->saddr);
	dip = ntohl(ip_hdr->daddr);
	
#ifdef	NAT_DEBUG_MSG
	{
		nat_printf("To   GMAC-%d: 0x%-4X GRE %d.%d.%d.%d [%d] --> %d.%d.%d.%d",
				port, ntohs(ip_hdr->id), 
				NIPQUAD(ip_hdr->saddr), ntohs(gre_hdr->call_id), 
				NIPQUAD(ip_hdr->daddr));
		nat_printf("\n");
	}
#endif				
	nat_ip_conntrack = ip_conntrack_get(skb, &ctinfo);
	if (nat_ip_conntrack)
	{
		// if (nat_ip_conntrack->master || nat_ip_conntrack->helper)
		if (nat_ip_conntrack->helper)
		{
			nat_printf("GRE Call-ID=%d, master=0x%x, helper=0x%x\n", ntohs(gre_hdr->call_id), (u32)nat_ip_conntrack->master, (u32)nat_ip_conntrack->helper);
			return 0;
		}
		if (!(nat_ip_conntrack->status & IPS_ASSURED))
			return 0;
	}
	
	hash_entry = (GRE_HASH_ENTRY_T *)&hash_data;
	if (port == cfg->wan_port)	// LAN-to-WAN
	{
#ifdef _NOT_CHECK_SIP_DIP	// enable it if know and get the wan ip address	
		if (!sl351x_nat_find_ipcfg(sip, port))
		{
			nat_printf("LAN->WAN Incorrect Sip %d.%d.%d.%d\n", HIPQUAD(sip));
			return 0;
		}
#endif
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->lan_port;
		hash_entry->key.rule_id 	= 0;	
		hash_entry->key.ip_protocol = IPPROTO_GRE;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.reserved3 	= 0;
		hash_entry->key.reserved4 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.protocol	= nat_cb->sport;
		hash_entry->key.call_id 	= nat_cb->dport;
		
		hash_index = gre_build_keys(&hash_entry->key);
		
#ifdef NAT_DEBUG_LAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			return 0;
		}
			
		// write hash entry
		hash_entry->key.rule_id = cfg->gre_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = 0;
		hash_entry->param.Dport = ntohs(gre_hdr->call_id);
		hash_entry->param.vlan = 0;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		// check PPPoE
		pppoe_hdr = (struct pppoe_hdr *)(ether_hdr + 1);
		ppp_proto = *(u16 *)&pppoe_hdr->tag[0];
		if (ether_hdr->h_proto == __constant_htons(ETH_P_PPP_SES)	// 0x8864
			&& ppp_proto == __constant_htons(PPP_IP) )				// 0x21
		{
			hash_entry->action.dword = NAT_PPPOE_PPTP_LAN2WAN_ACTIONS;
			hash_entry->param.pppoe = htons(pppoe_hdr->sid);
		}
		else
		{
			hash_entry->action.dword = NAT_PPTP_LAN2WAN_ACTIONS;
			hash_entry->param.pppoe = 0;
		}
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(IPPROTO_GRE, sip, dip, 0, ntohs(gre_hdr->call_id));
		hash_entry->action.bits.dest_qid +=	(cfg->wan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;
		hash_entry->tmo.counter = hash_entry->tmo.interval = cfg->gre_tmo_interval;
		gre_write_hash_entry(hash_index, hash_entry);
		// nat_printf("%lu Validate a LAN hash entry %d\n", jiffies/HZ, hash_index);
		// hash_dump_entry(hash_index);
		hash_nat_enable_owner(hash_index);
		hash_validate_entry(hash_index); // Must last one, else HW Tx fast than SW 
		return 0;
	}
	else // WAN-to-LAN
	{
		// Note: unused fields (including rule_id) MUST be zero
		hash_entry->key.Ethertype 	= 0;
		hash_entry->key.port_id 	= cfg->wan_port;
		hash_entry->key.rule_id 	= 0;	
		hash_entry->key.ip_protocol = IPPROTO_GRE;
		hash_entry->key.reserved1 	= 0;
		hash_entry->key.reserved2 	= 0;
		hash_entry->key.reserved3 	= 0;
		hash_entry->key.reserved4 	= 0;
		hash_entry->key.sip 		= ntohl(nat_cb->sip);
		hash_entry->key.dip 		= ntohl(nat_cb->dip);
		hash_entry->key.protocol	= nat_cb->sport;
		hash_entry->key.call_id		= nat_cb->dport;
		
		hash_index = gre_build_keys(&hash_entry->key);
		
#ifdef NAT_DEBUG_WAN_HASH_TIMEOUT
		if (hash_get_nat_owner_flag(hash_index))
			return 0;
#endif
		if (hash_get_valid_flag(hash_index))
		{
			return 0;
		}
		
		// write hash entry
		hash_entry->key.rule_id = cfg->gre_rule_id;
		memcpy(hash_entry->param.da, skb->data, 6);
		memcpy(hash_entry->param.sa, skb->data+6, 6);
		hash_entry->param.Sip = sip;
		hash_entry->param.Dip = dip;
		hash_entry->param.Sport = 0;
		hash_entry->param.Dport = ntohs(gre_hdr->call_id);
		hash_entry->param.vlan = 0;
		hash_entry->param.pppoe = 0;
		hash_entry->param.sw_id = 0;
		hash_entry->param.mtu = 0;
		hash_entry->action.dword = (nat_cb->pppoe_frame) ? NAT_PPPOE_PPTP_WAN2LAN_ACTIONS : NAT_PPTP_WAN2LAN_ACTIONS;
		hash_entry->action.bits.dest_qid = sl351x_nat_assign_qid(IPPROTO_GRE, sip, dip, 0, ntohs(gre_hdr->call_id));
		hash_entry->action.bits.dest_qid += (cfg->lan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;;
		hash_entry->tmo.counter = hash_entry->tmo.interval = cfg->gre_tmo_interval;
		gre_write_hash_entry(hash_index, hash_entry);
			
		// nat_printf("%lu Validate a WAN hash entry %d\n", jiffies/HZ, hash_index);
		// hash_dump_entry(hash_index);
   		hash_nat_enable_owner(hash_index);
 		hash_validate_entry(hash_index); // Must last one, else HW Tx fast SW 
		return 0;
	}
	return 0;
}


#ifdef _HAVE_DYNAMIC_PORT_LIST									   	
/*----------------------------------------------------------------------
* sl_nat_add_port
*----------------------------------------------------------------------*/
void sl_nat_add_port(u8 protocol, u16 port)
{
	int 	i;
	u16		*port_ptr;
	
	if (protocol == IPPROTO_TCP)
		port_ptr = dynamic_tcp_port_list;
	else if (protocol == IPPROTO_UDP)
		port_ptr = dynamic_udp_port_list;
	else
		return;
		
	for (i=0; *port_ptr; i++)
	{
		if (port == *port_ptr)
			return;
		port_ptr++;
	}
	port_ptr++;
	*port_ptr = port;
}

/*----------------------------------------------------------------------
* sl_nat_remove_port
*----------------------------------------------------------------------*/
void sl_nat_remove_port(u8 protocol, u16 port)
{
	int 	i, j;
	u16		*port_ptr, *next;
	
	if (protocol == IPPROTO_TCP)
		port_ptr = dynamic_tcp_port_list;
	else if (protocol == IPPROTO_UDP)
		port_ptr = dynamic_udp_port_list;
	else
		return;
		
	for (i=0; *port_ptr; i++, port_ptr++)
	{
		if (port == *port_ptr)
		{
			port_next = port_ptr + 1;
			for (j=i+1; *port_next; i++, j++)
				*port_ptr++ = *port_next++;
			*port_ptr = 0;
			return;
		}
	}
}
#endif

/*----------------------------------------------------------------------
* sl351x_nat_ioctl
*----------------------------------------------------------------------*/
int sl351x_nat_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	GMAC_INFO_T 		*tp = (GMAC_INFO_T *)dev->priv;
	int 				i, j, port_id,err = 0;
#ifdef VITESSE_G5SWITCH
	int ii,jj;
#endif	
    NATCMD_HDR_T		nat_hdr;
    NAT_REQ_E			ctrl;
	unsigned char		*req_datap;
	NAT_IP_ENTRY_T		*ipcfg;
	NAT_XPORT_ENTRY_T	*xport_entry;
	NAT_WRULE_ENTRY_T	*wrule_entry;
	unsigned int		qid,phy_data_s = -1,location_r = -1,length_r = -1,location_w = -1,data_w = -1;
	unsigned int		size_r,size_w;
	unsigned short		phy_addr = -1,phy_reg = -1,phy_len = -1, phy_addr_s = 0, phy_reg_s = 0;
	
	if (copy_from_user((void *)&nat_hdr, rq->ifr_data, sizeof(nat_hdr)))
		return -EFAULT;
	req_datap = (unsigned char *)rq->ifr_data + sizeof(nat_hdr);
	port_id = tp->port_id;
	switch (nat_hdr.cmd) {
	case NATSSTATUS:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_STATUS_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.status, req_datap, sizeof(ctrl.status)))
			return -EFAULT;
		if (ctrl.status.enable != 0 && ctrl.status.enable != 1)
			return -EPERM;
		// sl351x_nat_set_enabled_flag(ctrl.status.enable);
		if (nat_cfg.enabled && (ctrl.status.enable == 0))
		{
			for (i=0; i<HASH_TOTAL_ENTRIES; i++)
			{
				if (hash_get_nat_owner_flag(i))
				{
					hash_nat_disable_owner(i);
					hash_invalidate_entry(i);
				}
			}
		}
		nat_cfg.enabled = ctrl.status.enable;
		break;
	case NATGSTATUS:	
		if (nat_hdr.len != sizeof(NAT_STATUS_T))
			return -EPERM;
		ctrl.status.enable = nat_cfg.enabled;
		if (copy_to_user(req_datap, (void *)&ctrl.status, sizeof(ctrl.status)))
			return -EFAULT;
		break;
	case NATSETPORT:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_PORTCFG_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.portcfg, req_datap, sizeof(ctrl.portcfg)))
			return -EFAULT;
		if (ctrl.portcfg.portmap == 0)
			nat_cfg.lan_port = port_id;
		else if (ctrl.portcfg.portmap == 1)
			nat_cfg.wan_port = port_id;				
		else
			return -EPERM;
		break;
	case NATGETPORT:
		if (nat_hdr.len != sizeof(NAT_PORTCFG_T))
			return -EPERM;
		if (nat_cfg.lan_port == port_id)
			ctrl.portcfg.portmap = 0;
		else if (nat_cfg.wan_port == port_id)
			ctrl.portcfg.portmap = 1;
		else
			return -EPERM;
		if (copy_to_user(req_datap, (void *)&ctrl.portcfg, sizeof(ctrl.portcfg)))
			return -EFAULT;
		break;
	case NATADDIP:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_IPCFG_T))
			return -EPERM;
		i = nat_cfg.ipcfg[port_id].total;
		if (i >= CONFIG_NAT_MAX_IP_NUM)
			return -E2BIG;
		if (copy_from_user((void *)&nat_cfg.ipcfg[port_id].entry[i], req_datap, sizeof(NAT_IPCFG_T)))
			return -EFAULT;
		nat_cfg.ipcfg[port_id].total++;
		break;
	case NATDELIP:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_IPCFG_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.ipcfg, req_datap, sizeof(ctrl.ipcfg)))
			return -EFAULT;
		ipcfg = (NAT_IP_ENTRY_T *)&nat_cfg.ipcfg[port_id].entry[0];
		for (i=0; i<nat_cfg.ipcfg[port_id].total; i++, ipcfg++)
		{
			if (ipcfg->ipaddr == ctrl.ipcfg.entry.ipaddr)
			{
				NAT_IP_ENTRY_T *ipcfg_next;
				ipcfg_next = ipcfg + 1;
				for (j=i+1; j < nat_cfg.ipcfg[port_id].total; i++, j++)
				{
					memcpy((void *)ipcfg, (void *)ipcfg_next, sizeof(NAT_IP_ENTRY_T));
					ipcfg++;
					ipcfg_next++;	
				}
				ipcfg->ipaddr = 0;
				ipcfg->netmask = 0;
				nat_cfg.ipcfg[port_id].total--;
				return 0;
			}
		}
		return -ENOENT;
	case NATGETIP:
		if (nat_hdr.len != sizeof(NAT_IPCFG_ALL_T))
			return -EPERM;
		if (copy_to_user(req_datap, (void *)&nat_cfg.ipcfg[port_id], sizeof(NAT_IPCFG_ALL_T)))
			return -EFAULT;
		break;
	case NATAXPORT:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_XPORT_T))
			return -EPERM;
		i = nat_cfg.xport.total;
		if (i >= CONFIG_NAT_MAX_XPORT)
			return -E2BIG;
		if (copy_from_user((void *)&nat_cfg.xport.entry[i], req_datap, sizeof(NAT_XPORT_T)))
			return -EFAULT;
		nat_cfg.xport.total++;
		break;
	case NATDXPORT:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_XPORT_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.xport, req_datap, sizeof(NAT_XPORT_T)))
			return -EFAULT;
		xport_entry = (NAT_XPORT_ENTRY_T *)&nat_cfg.xport.entry[0];
		for (i=0; i<nat_cfg.xport.total; i++, xport_entry++)
		{
			if (memcmp((void *)xport_entry, (void *)&ctrl.xport, sizeof(NAT_XPORT_ENTRY_T)) == 0)
			{
				NAT_XPORT_ENTRY_T *xport_next;
				xport_next = xport_entry + 1;
				for (j=i+1; j < nat_cfg.xport.total; i++, j++)
				{
					memcpy((void *)xport_entry, (void *)xport_next, sizeof(NAT_XPORT_ENTRY_T));
					xport_entry++;
					xport_next++;	
				}
				memset((void *)xport_entry, 0, sizeof(NAT_XPORT_ENTRY_T));
				nat_cfg.xport.total--;
				return 0;
			}
		}
		return -ENOENT;
	case NATGXPORT:
		if (nat_hdr.len != sizeof(NAT_XPORT_ALL_T))
			return -EPERM;
		if (copy_to_user(req_datap, (void *)&nat_cfg.xport, sizeof(NAT_XPORT_ALL_T)))
			return -EFAULT;
		break;
	case NATSWEIGHT:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_WEIGHT_T))
			return -EPERM;
		if (copy_from_user((void *)&nat_cfg.weight, req_datap, sizeof(NAT_WEIGHT_T)))
			return -EFAULT;
		mac_set_hw_tx_weight(dev, (char *)&nat_cfg.weight);
		break;
	case NATGWEIGHT:
		if (nat_hdr.len != sizeof(NAT_WEIGHT_T))
			return -EPERM;
		mac_get_hw_tx_weight(dev, (char *)&nat_cfg.weight);
		if (copy_to_user(req_datap, (void *)&nat_cfg.weight, sizeof(NAT_WEIGHT_T)))
			return -EFAULT;
		break;
	case NATAWRULE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_WRULE_T))
			return -EPERM;
		if (copy_from_user((void *)&qid, req_datap, sizeof(qid)))
			return -EFAULT;
		if (qid > CONFIG_NAT_TXQ_NUM)
			return -EPERM;
		i = nat_cfg.wrule[qid].total;
		if (i >= CONFIG_NAT_MAX_WRULE)
			return -E2BIG;
		if (copy_from_user((void *)&nat_cfg.wrule[qid].entry[i], req_datap+sizeof(qid), sizeof(NAT_WRULE_T)))
			return -EFAULT;
		nat_cfg.wrule[qid].total++;
		break;
	case NATDWRULE:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_WRULE_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.wrule, req_datap, sizeof(NAT_WRULE_T)))
			return -EFAULT;
		qid = ctrl.wrule.qid;
		if (qid >= CONFIG_NAT_TXQ_NUM)
			return -EPERM;
		wrule_entry = (NAT_WRULE_ENTRY_T *)&nat_cfg.wrule[qid].entry[0];
		for (i=0; i<nat_cfg.wrule[qid].total; i++, wrule_entry++)
		{
			if (memcmp((void *)wrule_entry, (void *)&ctrl.wrule.entry, sizeof(NAT_WRULE_ENTRY_T)) == 0)
			{
				NAT_WRULE_ENTRY_T *wrule_next;
				wrule_next = wrule_entry + 1;
				for (j=i+1; j < nat_cfg.wrule[qid].total; i++, j++)
				{
					memcpy((void *)wrule_entry, (void *)wrule_next, sizeof(NAT_WRULE_ENTRY_T));
					wrule_entry++;
					wrule_next++;	
				}
				memset((void *)wrule_entry, 0, sizeof(NAT_WRULE_ENTRY_T));
				nat_cfg.wrule[qid].total--;
				return 0;
			}
		}
		return -ENOENT;
	case NAT_DEL_ENTRY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_WRULE_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.wrule, req_datap, sizeof(NAT_WRULE_T)))
			return -EFAULT;
		printk("sip:%x dip:%x pro:%d sport:%d-%d dport:%d-%d \n",
			ctrl.wrule.entry.sip_start,ctrl.wrule.entry.dip_start,ctrl.wrule.entry.protocol,
			ctrl.wrule.entry.sport_start,ctrl.wrule.entry.sport_end,ctrl.wrule.entry.dport_start,ctrl.wrule.entry.dport_end);
		
		sl351x_hash_scan_and_invalidate(ctrl.wrule.entry.sip_start,
						ctrl.wrule.entry.dip_start, 
						ctrl.wrule.entry.protocol,
						ctrl.wrule.entry.sport_start | (ctrl.wrule.entry.sport_end) << 16 ,
						ctrl.wrule.entry.dport_start | (ctrl.wrule.entry.dport_end) << 16 );
		
		break;	
	case NATGWRULE:
		if (nat_hdr.len != sizeof(NAT_WRULE_ALL_T))
			return -EPERM;
		if (copy_from_user((void *)&qid, req_datap, sizeof(qid)))
			return -EFAULT;
		if (qid >= CONFIG_NAT_TXQ_NUM)
			return -EPERM;
		if (copy_to_user(req_datap, (void *)&nat_cfg.wrule[qid], sizeof(NAT_WRULE_ALL_T)))
			return -EFAULT;
		break;
	case NATSDEFQ:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_QUEUE_T))
			return -EPERM;
		if (copy_from_user((void *)&nat_cfg.default_hw_txq, req_datap, sizeof(u32)))
			return -EFAULT;
		break;
	case NATGDEFQ:
		if (nat_hdr.len != sizeof(NAT_QUEUE_T))
			return -EPERM;
		if (copy_to_user(req_datap, (void *)&nat_cfg.default_hw_txq, sizeof(u32)))
			return -EFAULT;
	case NATRMIPCFG:
		nat_cfg.ipcfg[port_id].total = 0;
		break;
	case NATTESTENTRY:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		if (nat_hdr.len != sizeof(NAT_TESTENTRY_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.init_entry, req_datap, sizeof(ctrl.init_entry)))
			return -EFAULT;
		if (ctrl.init_entry.init_enable != 0 && ctrl.init_entry.init_enable != 1)
			return -EPERM;
		nat_cfg.init_enabled = ctrl.init_entry.init_enable;
		break;
	case GMIIREG:
		if (nat_hdr.len != sizeof(NAT_GMIIREG_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.get_mii_reg, req_datap, sizeof(ctrl.get_mii_reg)))
			return -EFAULT; //Invalid argument
		
		phy_addr = ctrl.get_mii_reg.phy_addr;
		phy_reg = ctrl.get_mii_reg.phy_reg;
		phy_len = ctrl.get_mii_reg.phy_len;
#ifdef VITESSE_G5SWITCH		
		if (Giga_switch == 1)
		{
			if (phy_addr == 2)
			{
				nat_printf("\n");
				for(jj=0;jj<4;jj++)
				{
					unsigned int data_switch;
					for (ii=0; ii< phy_len ; ii++)
					{
						data_switch = phy_read(jj,phy_reg);
						nat_printf("MII Switch Phy %d Port %d Reg %d Data = 0x%x\n", phy_addr, jj,phy_reg, data_switch);
						phy_reg++;
					}
					phy_reg = phy_reg - phy_len;
					nat_printf("\n");
				}
			}
			else if (phy_addr == 1)
			{
				for (i=0; i< phy_len ; i++)
				{
					unsigned int data;
					data = mii_read(phy_addr,phy_reg); 
					nat_printf("MII Phy %d Reg %d Data = 0x%x\n", phy_addr, phy_reg++, data);
				}
			}
			else
				err = 1;
		}
		else 
#endif		
		{
			if ((phy_addr == 1) || (phy_addr == 2))
			{
				for (i=0; i< phy_len ; i++)
				{
					unsigned int data;
					data = mii_read(phy_addr,phy_reg); 
					nat_printf("MII Phy %d Reg %d Data = 0x%x\n", phy_addr, phy_reg++, data);
				}
			}
			else 
				err = 1;
		}
		
		if (err == 1)
		{
			nat_printf("Syntax error!\n");
			nat_printf("Syntax: MII read [-a phy addr] [-r phy reg] [-l length]\n");
			nat_printf("Options:\n");
			nat_printf("\t-a  Phy address\n");
			nat_printf("\t-r  Phy registers\n");
			nat_printf("\t-l  Display total registers\n");
			nat_printf("MII Phy address -a %d error !! Phy address must be 1 or 2.\n", phy_addr);
		}
		break; 
		
	case SMIIREG:
		if (nat_hdr.len != sizeof(NAT_SMIIREG_T))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.set_mii_reg, req_datap, sizeof(ctrl.set_mii_reg)))
			return -EFAULT;
		
		phy_addr_s = ctrl.set_mii_reg.phy_addr;
		phy_reg_s = ctrl.set_mii_reg.phy_reg;
		phy_data_s = ctrl.set_mii_reg.phy_data;
#ifdef VITESSE_G5SWITCH			
		if (Giga_switch == 1)
		{
			if (phy_addr_s == 2)
			{
				nat_printf("\n");
				for(jj=0;jj<4;jj++)
				{
						phy_write(jj,phy_reg_s,phy_data_s);
						phy_write_masked(jj,phy_reg_s,phy_data_s,0xffff);
						nat_printf("Write MII Switch Phy %d Port %d Reg %d Data = 0x%x\n", phy_addr_s, jj,phy_reg_s, phy_data_s);
				}
			}
			else if (phy_addr_s == 1)
			{
					mii_write(phy_addr_s,phy_reg_s,phy_data_s); 
					nat_printf("Write MII Phy %d Reg %d Data = 0x%x\n", phy_addr_s, phy_reg_s, phy_data_s);
			}
			else
				err = 1;
		}
		else 
#endif		
		{
			if ((phy_addr_s == 1) || (phy_addr_s == 2))
			{
					mii_write(phy_addr_s,phy_reg_s,phy_data_s); 
					nat_printf("Write MII Phy %d Reg %d Data = 0x%x\n", phy_addr_s, phy_reg_s, phy_data_s);
			}
			else 
				err = 1;
		}
		
		if (err == 1)
		{
			nat_printf("Syntax error!\n");
			nat_printf("Syntax: MII write [-a phy addr] [-r phy reg] [-d data]\n");
			nat_printf("Options:\n");
			nat_printf("\t-a  Phy address\n");
			nat_printf("\t-r  Phy registers\n");
			nat_printf("\t-d  date\n");
			nat_printf("MII Phy address -a %d error !! Phy address must be 1 or 2.\n", phy_addr);
		}
		break;	
#ifdef GMAC_DEBUG_U			
	case DUMPRX:	
		if (nat_hdr.len != sizeof(NAT_DUMP_RX))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.dump_rx_packet, req_datap, sizeof(ctrl.dump_rx_packet)))
			return -EFAULT;
		if (ctrl.dump_rx_packet.enable != 0 && ctrl.dump_rx_packet.enable != 1)
			return -EPERM;	
		gmac_dump_rxpkt	= ctrl.dump_rx_packet.enable;
		break;	
	case DUMPTX:	
		if (nat_hdr.len != sizeof(NAT_DUMP_TX))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.dump_tx_packet, req_datap, sizeof(ctrl.dump_tx_packet)))
			return -EFAULT;
		if (ctrl.dump_tx_packet.enable != 0 && ctrl.dump_tx_packet.enable != 1)
			return -EPERM;	
		gmac_dump_txpkt	= ctrl.dump_tx_packet.enable;
		break;	
#endif		
	case REGREAD:	
		if (nat_hdr.len != sizeof(NAT_REGREAD))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.reg_read, req_datap, sizeof(ctrl.reg_read)))
			return -EFAULT;
		location_r = ctrl.reg_read.location;
		length_r = ctrl.reg_read.length;
		size_r = ctrl.reg_read.size;
    	
    	if (size_r == 1 && ((MIN_READ <= location_r) && (location_r <= MAX_READ)))//IO MEM can not dump use dm_byte.
    		size_r = 4;
		if (size_r == 1)
			dm_byte(location_r, length_r);
		if (size_r == 2)
			dm_short(location_r, length_r);
		if (size_r == 4)
			dm_long_1(location_r, length_r);
		break;		
	case REGWRITE:	
		if (nat_hdr.len != sizeof(NAT_REGWRITE))
			return -EPERM;
		if (copy_from_user((void *)&ctrl.reg_write, req_datap, sizeof(ctrl.reg_write)))
			return -EFAULT;
		location_w = ctrl.reg_write.location;
		data_w = ctrl.reg_write.data;
		size_w = ctrl.reg_write.size;
		if (size_w == 1)
		{
			if (data_w > 0xff)
				err = 1;
			else
			{	
				writeb(data_w,location_w);
				nat_printf("Write Data 0x%X to Location 0x%X\n",(u32)data_w, location_w);
			}
		}
		if (size_w == 2)
		{
			if (data_w > 0xffff)
				err = 1;
			else
			{
				writew(data_w,location_w);
				nat_printf("Write Data 0x%X to Location 0x%X\n",(u32)data_w, location_w);
			}
		}
		if (size_w == 4)
		{
			if (data_w > 0xffffffff)
				err = 1;
			else
			{	
				writel(data_w,location_w);
				nat_printf("Write Data 0x%X to Location 0x%X\n",(u32)data_w, location_w);
			}
		}
		if (err == 1)
		{
			nat_printf("Syntax:	gmac write mem [-b <location>] [-d <data>] [-1|2|4]\n");
			nat_printf("Options:\n");
			nat_printf("\t-b  Register Address\n");
			nat_printf("\t-d  Data Vaule\n");
			if (size_w == 1)
				nat_printf("\t-1  Data 0x%X < 0xFF\n",data_w);
			if (size_w == 2)	
				nat_printf("\t-2  Data 0x%X < 0xFFFF\n",data_w);
			if (size_w == 4)	
				nat_printf("\t-4  Data 0x%X < 0xFFFFFFFF\n",data_w);
		}
		break;
		
	case WAIT_BLOCKED_URL:	
		if (nat_hdr.len != (sizeof(struct iphdr)+sizeof(struct tcphdr) ))
			return -EPERM;
		memset(block_url_info,0,sizeof(struct iphdr)+sizeof(struct tcphdr));
		interruptible_sleep_on(&url_block_wait);
		nat_hdr.len = url_block_len;
		//req_datap -= sizeof(short) ;
		//if (copy_to_user(req_datap, (void *)&nat_hdr.len, sizeof(short)))
		//	return -EFAULT;
		if (copy_to_user(req_datap, (void *)block_url_info, sizeof(struct iphdr)+sizeof(struct tcphdr)))
			return -EFAULT;
		break;
			
	default:
		return -EPERM;
	}
	
	return 0;
}

/*----------------------------------------------------------------------
* 	nat_init_test_entry
*	Initialize NAT test hash entries
*
*	SmartBits P1  -----> Lepus GMAC 0 --------------+
*													|
*													|
*             P3  <----- Lepus GMAC 1 -- HW TxQ0 <--+
*									  -- HW TxQ1 <--+
*									  -- HW TxQ2 <--+
*									  -- HW TxQ3 <--+
*
*	SmartBits P1  <----- Lepus GMAC 0 -- HW TxQ0 <--+
*									  -- HW TxQ1 <--+
*                                     -- HW TxQ2 <--+
*									  -- HW TxQ3 <--+
*													|
*													|
*             P3  -----> Lepus GMAC 1 --------------+
*
*   LAN GMAC0 <--------------------------------------------> GMAC1 WAN
*	192.168.[x].[y]:50 --> 168.95.[x].[y]:80 ---TXQ[y-1]---> 192.168.2.254:200[y] --> 168.95.[x].[y]:80
*	192.168.[x].[y]:50 <-- 168.95.[x].[y]:80 <--TXQ[y-1]---- 192.168.2.254:200[y] <-- 168.95.[x].[y]:80
*   where:
*		[x] : Packet Type
*		[y] : Tx Queue, 1 for TxQ0, 2 for TxQ1, 3 for TxQ2, 4 for TxQ3,
*
*
* Packet Type:								 
* 1. TCP Frames <---> TCP Frames
*   LAN GMAC0 <--------------------------------> GMAC1 WAN
*	192.168.1.1:50 --> 168.95.1.1:80 ---TXQ0---> 192.168.2.254:2001 --> 168.95.1.1:80
*	192.168.1.1:50 <-- 168.95.1.1:80 <--TXQ0---- 192.168.2.254:2001 <-- 168.95.1.1:80
*                                                                                 
*	192.168.1.2:50 --> 168.95.1.2:80 ---TXQ1---> 192.168.2.254:2002 --> 168.95.1.2:80
*	192.168.1.2:50 <-- 168.95.1.2:80 <--TXQ1---- 192.168.2.254:2002 <-- 168.95.1.2:80
*                                                                                 
*	192.168.1.3:50 --> 168.95.1.3:80 ---TXQ2---> 192.168.2.254:2003 --> 168.95.1.3:80
*	192.168.1.3:50 <-- 168.95.1.3:80 <--TXQ2---- 192.168.2.254:2003 <-- 168.95.1.3:80
*                                                                                 
*	192.168.1.4:50 --> 168.95.1.4:80 ---TXQ3---> 192.168.2.254:2004 --> 168.95.1.4:80
*	192.168.1.4:50 <-- 168.95.1.4:80 <--TXQ3---- 192.168.2.254:2004 <-- 168.95.1.4:80
*             
* 2 TCP Frames <----> PPPoE + TCP Frames
*   LAN GMAC0 <--------------------------------> GMAC1 WAN
*	192.168.2.1:50 --> 168.95.2.1:80 ---TXQ0---> 192.168.2.254:2001 --> 168.95.2.1:80
*	192.168.2.1:50 <-- 168.95.2.1:80 <--TXQ0---- 192.168.2.254:2001 <-- 168.95.2.1:80
*                                                                                 
*	192.168.2.2:50 --> 168.95.2.2:80 ---TXQ1---> 192.168.2.254:2002 --> 168.95.2.2:80
*	192.168.2.2:50 <-- 168.95.2.2:80 <--TXQ1---- 192.168.2.254:2002 <-- 168.95.2.2:80
*                                                                                 
*	192.168.2.3:50 --> 168.95.2.3:80 ---TXQ2---> 192.168.2.254:2003 --> 168.95.2.3:80
*	192.168.2.3:50 <-- 168.95.2.3:80 <--TXQ2---- 192.168.2.254:2003 <-- 168.95.2.3:80
*                                                                                 
*	192.168.2.4:50 --> 168.95.2.4:80 ---TXQ3---> 192.168.2.254:2004 --> 168.95.2.4:80
*	192.168.2.4:50 <-- 168.95.2.4:80 <--TXQ3---- 192.168.2.254:2004 <-- 168.95.2.4:80
*             
* 3 TCP Frames <----> VLAN + PPPoE + TCP Frames
*   LAN GMAC0 <--------------------------------> GMAC1 WAN
*	192.168.3.1:50 --> 168.95.3.1:80 ---TXQ0---> 192.168.2.254:2001 --> 168.95.3.1:80
*	192.168.3.1:50 <-- 168.95.3.1:80 <--TXQ0---- 192.168.2.254:2001 <-- 168.95.3.1:80
*                                                                                 
*	192.168.3.2:50 --> 168.95.3.2:80 ---TXQ1---> 192.168.2.254:2002 --> 168.95.3.2:80
*	192.168.3.2:50 <-- 168.95.3.2:80 <--TXQ1---- 192.168.2.254:2002 <-- 168.95.3.2:80
*                                                                                 
*	192.168.3.3:50 --> 168.95.3.3:80 ---TXQ2---> 192.168.2.254:2003 --> 168.95.3.3:80
*	192.168.3.3:50 <-- 168.95.3.3:80 <--TXQ2---- 192.168.2.254:2003 <-- 168.95.3.3:80
*                                                                                 
*	192.168.3.4:50 --> 168.95.3.4:80 ---TXQ3---> 192.168.2.254:2004 --> 168.95.3.4:80
*	192.168.3.4:50 <-- 168.95.3.4:80 <--TXQ3---- 192.168.2.254:2004 <-- 168.95.3.4:80
*             
* 4 VLAN-A + TCP Frames <----> VLAN-B + PPPoE + TCP Frames
*   LAN GMAC0 <--------------------------------> GMAC1 WAN
*	192.168.4.1:50 --> 168.95.4.1:80 ---TXQ0---> 192.168.2.254:2001 --> 168.95.4.1:80
*	192.168.4.1:50 <-- 168.95.4.1:80 <--TXQ0---- 192.168.2.254:2001 <-- 168.95.4.1:80
*                                                                                 
*	192.168.4.2:50 --> 168.95.4.2:80 ---TXQ1---> 192.168.2.254:2002 --> 168.95.4.2:80
*	192.168.4.2:50 <-- 168.95.4.2:80 <--TXQ1---- 192.168.2.254:2002 <-- 168.95.4.2:80
*                                                                                 
*	192.168.4.3:50 --> 168.95.4.3:80 ---TXQ2---> 192.168.2.254:2003 --> 168.95.4.3:80
*	192.168.4.3:50 <-- 168.95.4.3:80 <--TXQ2---- 192.168.2.254:2003 <-- 168.95.4.3:80
*                                                                                 
*	192.168.4.4:50 --> 168.95.4.4:80 ---TXQ3---> 192.168.2.254:2004 --> 168.95.4.4:80
*	192.168.4.4:50 <-- 168.95.4.4:80 <--TXQ3---- 192.168.2.254:2004 <-- 168.95.4.4:80
*             
* 
*
*----------------------------------------------------------------------*/
#ifdef SL351x_NAT_TEST_BY_SMARTBITS
#define 	NAT_IPIV(a,b,c,d)			((a<<24)+(b<<16)+(c<<8)+d)
#define     NAT_TEST_CLIENT_IP 			NAT_IPIV(192,168,1,1)	
#define     NAT_TEST_SERVER_IP 			NAT_IPIV(168,95,1,1)
#define		NAT_TEST_LAN_IP				NAT_IPIV(192,168,1,254)
#define		NAT_TEST_WAN_IP				NAT_IPIV(192,168,2,254)
#define     NAT_TEST_MAP_PORT_BASE		2001
#define     NAT_TEST_SPORT				50
#define     NAT_TEST_DPORT				80
#define     NAT_TEST_PROTOCOL			6
u8			nat_test_lan_target_da[6]={0x00,0x11,0x22,0x33,0x44,0x55};
u8			nat_test_wan_target_da[6]={0x00,0xaa,0xbb,0xcc,0xdd,0xee};
u8			nat_test_lan_my_da[6]={0x00,0x11,0x11,0x11,0x11,0x11};
u8			nat_test_wan_my_da[6]={0x00,0x22,0x22,0x22,0x22,0x22};
static void nat_init_test_entry(void)
{
	int 				i, j ;
	NAT_HASH_ENTRY_T	*hash_entry;
	u32					sip, dip;
	u32					hash_data[HASH_MAX_DWORDS];
	NAT_CFG_T			*cfg;
	int					hash_index;
	
	cfg = (NAT_CFG_T *)&nat_cfg;
	hash_entry = (NAT_HASH_ENTRY_T *)&hash_data;
	hash_entry->key.Ethertype 	= 0;
	hash_entry->key.rule_id 	= 0;	
	hash_entry->key.ip_protocol = IPPROTO_TCP;
	hash_entry->key.reserved1 	= 0;
	hash_entry->key.reserved2 	= 0;
	// hash_entry->key.sip 		= NAT_TEST_CLIENT_IP;
	// hash_entry->key.dip 		= NAT_TEST_SERVER_IP;
	hash_entry->key.sport 		= htons(NAT_TEST_SPORT);
	hash_entry->key.dport 		= htons(NAT_TEST_DPORT);
	hash_entry->key.rule_id = cfg->tcp_udp_rule_id;
	hash_entry->action.dword = NAT_LAN2WAN_ACTIONS;
		
	sip = NAT_TEST_CLIENT_IP;
	dip = NAT_TEST_SERVER_IP;
	
	// Init TCP <------> TCP hash entries
	// LAN --> WAN
	// (1) TCP --> TCP
	// (2) TCP --> PPPoE + TCP
	// (3) TCP --> VLAN-B + PPPoE + TCP
	// (4) TCP + VLAN-A --> VLAN-B + PPPoE + TCP
	memcpy(hash_entry->param.da, nat_test_wan_target_da, 6);
	memcpy(hash_entry->param.sa, nat_test_wan_my_da, 6);
	hash_entry->key.port_id = cfg->lan_port;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		if (i < 2)
		{
			hash_entry->action.bits.dest_qid = i+2;
		}
		else
		{
			hash_entry->action.bits.dest_qid = i;
		}
		hash_entry->action.bits.dest_qid += (cfg->wan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;
		hash_entry->param.Sport = NAT_TEST_MAP_PORT_BASE+i;
		hash_entry->param.Dport = NAT_TEST_DPORT;
		for (j=0; j<4; j++)
		{
			hash_entry->key.sip = sip + i + j*0x100;
			hash_entry->key.dip = dip + i + j*0x100;
			hash_entry->param.Dip = hash_entry->key.dip;
			hash_entry->param.Sip = NAT_TEST_WAN_IP;
			switch (j)
			{
			case 0:
				hash_entry->action.bits.pppoe = 0;
				hash_entry->param.pppoe = 0;
				hash_entry->action.bits.vlan = 0;
				hash_entry->param.vlan = 0;
				break;
			case 1:
				hash_entry->action.bits.pppoe = 1;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 0;
				hash_entry->param.vlan = 0;
				break;
			case 2:
				hash_entry->action.bits.pppoe = 1;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 1;
				hash_entry->param.vlan = i+10;
				break;
			case 3:
				hash_entry->action.bits.pppoe = 1;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 1;
				hash_entry->param.vlan = i+10;
				break;
			}
			hash_entry->tmo.counter = hash_entry->tmo.interval = 0x7fff;
			hash_index = nat_build_keys(&hash_entry->key);
			nat_write_hash_entry(hash_index, hash_entry);
			hash_nat_enable_owner(hash_index);
			hash_validate_entry(hash_index); // Must last one, else HW Tx fast than SW 
		}
	}
	
		
	// WAN --> LAN
	hash_entry->key.port_id 	= cfg->wan_port;
	hash_entry->key.sport 		= htons(NAT_TEST_DPORT);
	hash_entry->key.dport 		= htons(NAT_TEST_DPORT);
	hash_entry->key.rule_id		= cfg->tcp_udp_rule_id;
	hash_entry->action.dword	= NAT_WAN2LAN_ACTIONS;
	hash_entry->key.sport		= htons(NAT_TEST_DPORT);
	memcpy(hash_entry->param.da, nat_test_lan_target_da, 6);
	memcpy(hash_entry->param.sa, nat_test_lan_my_da, 6);
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		hash_entry->key.dport = htons(NAT_TEST_MAP_PORT_BASE + i);  
		if (i < 2)
		{
			hash_entry->action.bits.dest_qid = i+2;
		}
		else
		{
			hash_entry->action.bits.dest_qid = i;
		}
		hash_entry->action.bits.dest_qid += (cfg->lan_port==0) ? TOE_GMAC0_HW_TXQ0_QID : TOE_GMAC1_HW_TXQ0_QID;
		hash_entry->param.Dport = NAT_TEST_SPORT;
		hash_entry->param.Sport = NAT_TEST_DPORT;
		hash_entry->param.da[5] = i;
		for (j=0; j<4; j++)
		{
			hash_entry->key.sip = (dip + i + j*0x100);
			hash_entry->key.dip = (NAT_TEST_WAN_IP);
			hash_entry->param.Sip = hash_entry->key.sip;
			hash_entry->param.Dip = sip + i + j*0x100;
			switch (j)
			{
			case 0:
				hash_entry->action.bits.pppoe = 0;
				hash_entry->param.pppoe = 0;
				hash_entry->action.bits.vlan = 0;
				hash_entry->param.vlan = 0;
				break;
			case 1:
				hash_entry->action.bits.pppoe = 2;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 0;
				hash_entry->param.vlan = 0;
				break;
			case 2:
				hash_entry->action.bits.pppoe = 2;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 2;
				hash_entry->param.vlan = i+5;
				break;
			case 3:
				hash_entry->action.bits.pppoe = 1;
				hash_entry->param.pppoe = i+1;
				hash_entry->action.bits.vlan = 1;
				hash_entry->param.vlan = i+5;
				break;
			}
			hash_entry->tmo.counter = hash_entry->tmo.interval = 0x7fff;
			hash_index = nat_build_keys(&hash_entry->key);
			nat_write_hash_entry(hash_index, hash_entry);
			hash_nat_enable_owner(hash_index);
			hash_validate_entry(hash_index); // Must last one, else HW Tx fast than SW 
		}
	}
}
#endif	// SL351x_NAT_TEST_BY_SMARTBITS

#endif // CONFIG_SL351x_NAT

