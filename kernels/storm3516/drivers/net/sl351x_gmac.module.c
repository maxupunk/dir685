/************************************************************************** 
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.
*--------------------------------------------------------------------------
* Name			: sl351x_gmac.c
* Description	: 
*		Ethernet device driver for Storlink SL351x FPGA
*
* History
*
*	Date		Writer		Description
*	-----------	-----------	-------------------------------------------------
*	08/22/2005	Gary Chen	Create and implement
*   27/10/2005  CH Hsu      Porting to Linux   
*
****************************************************************************/
#include <linux/config.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/compiler.h>
#include <linux/pci.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/rtnetlink.h>
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
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/mtd/kvctl.h>
#include <linux/sysctl_storlink.h>
#include "../../net/bridge/br_private.h"

//joel add for the RTL8366SR switch
#define ALPHA_RTL8366SR_MODULE
#ifdef ALPHA_RTL8366SR_MODULE
typedef void (*igmp_funp)(struct sk_buff *,int);
igmp_funp igmp_snoop_input;
#endif
//#define RTL8366SR
#ifdef RTL8366SR
#include "rtl8366rb/rtl8368s_asicdrv.h"
#include "rtl8366rb/rtl8366rb_api.h"
#include "rtl8366rb/rtl8366rb_api_ext.h"
//#define rtl8366sr_printk(argv ...) printk(argv)
#define rtl8366sr_printk(argv ...) 
extern int32 rtl8366rb_getPHYLinkStatus(uint32 phy, uint32 *linkStatus);
extern int32 rtl8366rb_getPartnerAbility(uint32 phy, rtl8366rb_phyAbility_t* ability);
void gmac_get_rtl8366sr_status(struct net_device *dev);
int32 rtl8366s_getAsicPortLinkState(enum PORTID port, enum PORTLINKSPEED *speed, enum PORTLINKDUPLEXMODE *duplex,uint32 *link, uint32 *txPause,uint32 *rxPause,uint32 *nWay);
#define RTL8366_LINK_INTERRUPT 1
#define RTL8366_LINK_INTERRUPT_PIN     15
//joel for igmp snoop in rtl8366sr
//hendry add for enable/disable snooping
#define ALPHA_IGMP_SNOOP
#ifdef ALPHA_IGMP_SNOOP
int g_igmp_snoop_initialized = 0;
int g_igmp_snoop_enable = 1;
extern void init_igmp_snoop();
extern int igmp_snoop_write_proc(struct file *file, const char __user *buffer, unsigned long count, void *read_data);  
extern int igmp_snoop_read_proc(char *buffer,char **buffer_location,off_t offset, int buffer_length, int *eof, void *data);
extern void igmp_snoop_input(struct sk_buff *skb);
extern void igmp_snoop(struct sk_buff *skb, int portId);
#endif
#endif


#define	 MIDWAY 
#define	 SL_LEPUS
//joel mark
//#define VITESSE_G5SWITCH	1

#ifndef CONFIG_SL351x_RXTOE
//#define CONFIG_SL351x_RXTOE	1
#endif
#undef CONFIG_SL351x_RXTOE

#include <linux/sysctl_storlink.h>
#include <asm/arch/sl2312.h>
#include <asm/arch/sl351x_gmac.h>
#include <asm/arch/sl351x_hash_cfg.h>
#include <asm/arch/sl351x_nat_cfg.h>

#ifdef CONFIG_SL351x_RXTOE
#include <asm/arch/sl351x_toe.h>
#include <net/tcp.h>
#include <linux/tcp.h>
#include <linux/ip.h>
#endif
// #define SL351x_TEST_WORKAROUND
#ifdef CONFIG_RTL8366SR_PHY
#undef VITESSE_G5SWITCH
#define CONFIG_SL351x_RTLDMZ	1
//#define SKIP_NAT_CFG			1		
#include <asm/arch/sl351x_rt8366.h>
#endif
 
#ifdef CONFIG_SL351x_RTLDMZ
#define DUAL_BAND_VIRTUAL_INTERFACE	1	//for open virtual interface
#include <linux/if_vlan.h>
#endif

#ifndef CONFIG_SL351X_BR
#define DO_HW_CHKSUM					1
#endif
#define CONFIG_SL_NAPI					1
//#undef CONFIG_SL_NAPI

#define GMAX_TX_INTR_DISABLED			1
#define ENABLE_TSO						1
#define GMAC_USE_TXQ0					1
// #define NAT_WORKAROUND_BY_RESET_GMAC	1
// #define HW_RXBUF_BY_KMALLOC			1
//#define _DUMP_TX_TCP_CONTENT	1
#define	br_if_ioctl						1
#define GMAC_LEN_1_2_ISSUE				1
#undef  GMAC_DEBUG	
//#define GMAC_DEBUG					1	//defined at sl351x_gmac.h
#define	INTERRUPT_SELECT				1
#define PHY_WORKAROUND					1
//#define BRIDGE_CHARIOT_TEST			1
#define DEBUG_MEMORY_LEAKAGE			1
//#define STORLINK_PHY					1
//#define GMAC_DEBUG_U					1	//defined at sl351x_gmac.h

#define GMAC_EXISTED_FLAG			0x5566abcd
#define CONFIG_MAC_NUM				GMAC_NUM
#define GMAC0_BASE					TOE_GMAC0_BASE
#define GMAC1_BASE					TOE_GMAC1_BASE
#define PAUSE_SET_HW_FREEQ			(TOE_HW_FREEQ_DESC_NUM / 2)
#define PAUSE_REL_HW_FREEQ			((TOE_HW_FREEQ_DESC_NUM / 2) + 10)
#define DEFAULT_RXQ_MAX_CNT			1024
#ifdef	L2_jumbo_frame
#define TCPHDRLEN(tcp_hdr)  ((ntohs(*((__u16 *)tcp_hdr + 6)) >> 12) & 0x000F)
#endif

/* define chip information */
#define DRV_NAME					"Storm Semiconductors,Inc. "
#define DRV_VERSION					"0.1.7"
#define SL351x_DRIVER_NAME  		DRV_NAME "SL351x Giga Ethernet driver " DRV_VERSION

#define toe_gmac_enable_interrupt(irq)	enable_irq(irq)
#define toe_gmac_disable_interrupt(irq)	disable_irq(irq)

#ifdef SL351x_GMAC_WORKAROUND
#define GMAC_SHORT_FRAME_THRESHOLD		10
static struct timer_list gmac_workround_timer_obj;
void sl351x_poll_gmac_hanged_status(u32 data);
#ifdef CONFIG_SL351x_NAT
	void sl351x_nat_workaround_init(void);
	#ifndef NAT_WORKAROUND_BY_RESET_GMAC
		static void sl351x_nat_workaround_handler(void);
	#endif
#endif
#endif

#ifdef GMAC_LEN_1_2_ISSUE
	#define _DEBUG_PREFETCH_NUM	256
	int	_debug_prefetch_cnt;
	char _debug_prefetch_buf[_DEBUG_PREFETCH_NUM][4] __attribute__((aligned(4)));
#endif

#ifdef CONFIG_SL351X_IPSEC
#define IPSEC_OUTBOUND_QID	0		// must match with sl351x_ipsec.h
#define IPSEC_INBOUND_QID	1		// must match with sl351x_ipsec.h
#endif

#ifdef DUAL_BAND_VIRTUAL_INTERFACE
struct gmac_conf VLAN_conf[] = { 
	{ (struct net_device *)0,0x7F,1 }, 
	{ (struct net_device *)0,0x80,2 } 
	};
#define NUM_VLAN_IF	(sizeof(VLAN_conf)/sizeof(struct gmac_conf))
static struct net_device	*GMACVIF0;
static struct net_device	*GMACVIF1;
int gmac0_running = 0;
int gmac0_vif0_down = 0,gmac0_vif1_down = 0;
#define	LAN_VLAN_ID					2
#define DMZ_VLAN_ID					3
#endif	//DUAL_BAND_VIRTUAL_INTERFACE

/*************************************************************
 *         Global Variable
 *************************************************************/
static int	gmac_initialized = 0;
#ifdef STORLINK_PHY		
	static int link_status = 0;
#endif
TOE_INFO_T toe_private_data;
spinlock_t gmac_fq_lock;
unsigned int FLAG_SWITCH;

#ifdef GMAC_DEBUG_U	
int gmac_dump_rxpkt;
int gmac_dump_txpkt;
#endif
#ifdef STORLINK_PHY
static unsigned int     	next_tick = 3 * HZ;
#else		
static unsigned int     	next_tick = 3 * HZ;		
#endif
#ifdef DUAL_BAND_VIRTUAL_INTERFACE
static unsigned char    	eth_mac[3][6]= {{0x00,0x11,0x11,0x87,0x87,0x87}, {0x00,0x22,0x22,0xab,0xab,0xab}, {0x00,0x51,0x51,0x57,0x87,0x87}};
#else
static unsigned char    	eth_mac[CONFIG_MAC_NUM][6]= {{0x00,0x11,0x11,0x87,0x87,0x87}, {0x00,0x22,0x22,0xab,0xab,0xab}};
#endif
#undef CONFIG_SL351x_RXTOE
extern NAT_CFG_T nat_cfg;

/************************************************/
/*                 function declare             */
/************************************************/
static int gmac_set_mac_address(struct net_device *dev, void *addr);
static unsigned int gmac_get_phy_vendor(int phy_addr);
static void gmac_set_phy_status(struct net_device *dev);
void gmac_get_phy_status(struct net_device *dev);
static int gmac_netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
static void gmac_tx_timeout(struct net_device *dev);
static int gmac_change_mtu(struct net_device *dev, int new_mtu);
static int gmac_phy_thread (void *data);
struct net_device_stats * gmac_get_stats(struct net_device *dev);
static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev);
static void gmac_set_rx_mode(struct net_device *dev);
static irqreturn_t toe_gmac_interrupt (int irq, void *dev_instance, struct pt_regs *regs);
static inline void toe_gmac_handle_default_rxq(struct net_device *dev, GMAC_INFO_T *tp);

unsigned int mii_read(unsigned char phyad,unsigned char regad);
void mii_write(unsigned char phyad,unsigned char regad,unsigned int value);
void mac_init_drv(void);

static void toe_init_free_queue(void);
static void toe_init_swtx_queue(void);
static void toe_init_default_queue(void);
#ifdef CONFIG_SL351x_RXTOE
static void toe_init_interrupt_queue(void);
#endif
static void toe_init_interrupt_config(void);
static void toe_gmac_sw_reset(void);
static int toe_gmac_init_chip(struct net_device *dev);
static void toe_gmac_enable_tx_rx(struct net_device* dev);
static void toe_gmac_disable_tx_rx(struct net_device *dev);
static void toe_gmac_hw_start(struct net_device *dev);
static void toe_gmac_hw_stop(struct net_device *dev);
static int toe_gmac_clear_counter(struct net_device *dev);
static void toe_init_gmac(struct net_device *dev);
static inline void toe_gmac_tx_complete(GMAC_INFO_T *tp, unsigned int tx_qid, struct net_device *dev, int interrupt);
#ifdef CONFIG_SL_NAPI
static inline int gmac_rx_poll(struct net_device *dev, int *budget);
// static void toe_gmac_disable_rx(struct net_device *dev);
// static void toe_gmac_enable_rx(struct net_device *dev);
#endif

u32 mac_read_dma_reg(int mac, unsigned int offset);
void mac_write_dma_reg(int mac, unsigned int offset, u32 data);
void mac_stop_txdma(struct net_device *dev);
void mac_get_sw_tx_weight(struct net_device *dev, char *weight);
void mac_set_sw_tx_weight(struct net_device *dev, char *weight);
void mac_get_hw_tx_weight(struct net_device *dev, char *weight);
void mac_set_hw_tx_weight(struct net_device *dev, char *weight);
void toe_gmac_fill_free_q(void);
static void gmac_reset_task(struct net_device *dev);
void dm_byte(u32 location, int length);
unsigned int Giga_switch=0;
#ifdef VITESSE_G5SWITCH
extern int Get_Set_port_status();
extern int SPI_default(void);
extern unsigned int SPI_get_identifier(void);
void gmac_get_switch_status(struct net_device *dev);
unsigned int Giga_switch = 0;
unsigned int switch_port_no = 0;
unsigned int ever_dwon = 0;
unsigned int c_GPIO = 0;
#endif

#ifdef CONFIG_RTL8366SR_PHY
extern int rtl8366sr_phy_setup(int ethUnit);
extern int rtl8366sr_phy_is_up(int ethUnit);
void gmac_get_switch_status(struct net_device *dev);
unsigned int Giga_switch = 0;
unsigned int ever_dwon = 0;
#endif	

/************************************************/
/*            GMAC function declare             */
/************************************************/
static int gmac_open (struct net_device *dev);
static int gmac_close (struct net_device *dev);
static void gmac_cleanup_module(void);
static void gmac_get_mac_address(void);

#ifdef CONFIG_SL351x_NAT
static void toe_init_hwtx_queue(void);
extern void sl351x_nat_init(void);
#ifdef CONFIG_SL351x_RTLDMZ	
extern void sl351x_nat_input(struct sk_buff *skb, int port, void *l3off, void *l4off, unsigned short rx_vid);
#else
extern void sl351x_nat_input(struct sk_buff *skb, int port, void *l3off, void *l4off);
#endif		
extern int sl351x_nat_output(struct sk_buff *skb, int port);
extern int sl351x_nat_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
#else
extern int sl351x_gmac_ioctl(struct net_device *dev, struct ifreq *rq, int cmd);
#endif

#ifdef CONFIG_SL351X_BR
static void toe_init_hwtx_queue(void);
extern void sl351x_br_init(void);
extern void sl351x_br_input(struct sk_buff *skb, int port);
#endif

#ifdef CONFIG_SL351x_RXTOE
extern void set_toeq_hdr(struct toe_conn* connection, TOE_INFO_T* toe, struct net_device *dev);
extern void sl351x_toe_init(void);
extern void toe_gmac_handle_toeq(struct net_device *dev, GMAC_INFO_T* tp, __u32 status);
extern struct toe_conn* init_toeq(int ipver, void* iph, struct tcphdr* tcp_hdr, TOE_INFO_T* toe, unsigned char* l2hdr);
#endif


#ifdef CONFIG_SL351X_IPSEC
extern int sl351x_ipsec_init(void);
extern void ipsec_init_class_queue(void);
extern int ipsec_handle_class_queue(struct net_device *, GMAC_INFO_T *, int budget);
#endif

int mac_set_rule_reg(int mac, int rule, int enabled, u32 reg0, u32 reg1, u32 reg2);
void mac_set_rule_enable_bit(int mac, int rule, int data);
int mac_set_rule_action(int mac, int rule, int data);
int mac_get_MRxCRx(int mac, int rule, int ctrlreg);
void mac_set_MRxCRx(int mac, int rule, int ctrlreg, u32 data);


/*----------------------------------------------------------------------
*	Ethernet Driver init
*----------------------------------------------------------------------*/

static int __init gmac_init_module(void)
{
	GMAC_INFO_T 		*tp;
	struct net_device	*dev;
	int 		i,j;
	unsigned int	chip_id,val_ns;
#ifdef VITESSE_G5SWITCH	
	unsigned int val_1;
#endif	 

#ifdef CONFIG_SL3516_ASIC
{
    unsigned int    val;
    /* set GMAC global register */
    val = readl(GMAC_GLOBAL_BASE_ADDR+0x10); 
    val = val | 0x005f0000;
    writel(val,GMAC_GLOBAL_BASE_ADDR+0x10); //Boot loader had writen it.
//    writel(0xb737b737,GMAC_GLOBAL_BASE_ADDR+0x1c); //For Socket Board
    writel(0x77777777,GMAC_GLOBAL_BASE_ADDR+0x20);
//    writel(0xa737b747,GMAC_GLOBAL_BASE_ADDR+0x1c);//For Mounting Board

	//debug_Aaron
    //writel(0xa7f0a7f0,GMAC_GLOBAL_BASE_ADDR+0x1c);//For Mounting Board
    writel(0xa7f0b7f0,GMAC_GLOBAL_BASE_ADDR+0x1c);//For Mounting Board
#ifdef	STORLINK_PHY
	writel(0xa7f09978,GMAC_GLOBAL_BASE_ADDR+0x1c);//For storlink phy setting
#endif	    

    writel(0x77777777,GMAC_GLOBAL_BASE_ADDR+0x24);
	writel(0x09200030,GMAC_GLOBAL_BASE_ADDR+0x2C);
	val = readl(GMAC_GLOBAL_BASE_ADDR+0x04);
	if((val&(1<<20))==0){           // GMAC1 enable
 		val = readl(GMAC_GLOBAL_BASE_ADDR+0x30);
		val = (val & 0xe7ffffff) | 0x08000000;
		writel(val,GMAC_GLOBAL_BASE_ADDR+0x30);
	}
	}    
#endif
	
#ifdef CONFIG_RTL8366SR_PHY
	writel(0x488cb7f0,GMAC_GLOBAL_BASE_ADDR+0x1c);	//For RTL8211B PHY Board
	Giga_switch = rtl8366sr_phy_setup(0);	
#endif	

	val_ns = readl(GMAC_GLOBAL_BASE_ADDR+0x04);
	val_ns = val_ns & (1<<30); 
	chip_id = readl(GMAC_GLOBAL_BASE_ADDR+0x0);
	
#ifdef VITESSE_G5SWITCH	
	if ((chip_id == 0x3516C3) && (val_ns == 0x40000000))
		c_GPIO = 1;	
	else
		c_GPIO = 0;			

	if (c_GPIO)
	{
		val_1 = readl(GMAC_GLOBAL_BASE_ADDR+0x30);
		val_1 ^= (1 << 4);
		writel(val_1,GMAC_GLOBAL_BASE_ADDR+0x30);
	}
	Giga_switch = SPI_get_identifier();
	if(Giga_switch)
		switch_port_no = SPI_default();
	if (c_GPIO)
	{
		val_1 = readl(GMAC_GLOBAL_BASE_ADDR+0x30);	
		val_1 |= (1 << 4);
		writel(val_1,GMAC_GLOBAL_BASE_ADDR+0x30);
	}	
#endif
	
	if ((chip_id == 0x3512C1) || (chip_id ==0x3512C2))
	{
		writel(0x5787a5f0,GMAC_GLOBAL_BASE_ADDR+0x1c);//For 3512 Switch Board 
		writel(0x55557777,GMAC_GLOBAL_BASE_ADDR+0x20);//For 3512 Switch Board
#ifdef	STORLINK_PHY
	     writel(0x57879978,GMAC_GLOBAL_BASE_ADDR+0x1c);//For storlink phy setting on Switch board 	
#endif		 		 
	}
	
//#endif

#ifdef GMAC_DEBUG_U	
	gmac_dump_rxpkt = 0;
	gmac_dump_txpkt = 0;
#endif	

	mac_init_drv();
	
	printk (KERN_INFO SL351x_DRIVER_NAME " built at %s %s\n", __DATE__, __TIME__);

//	init_waitqueue_entry(&wait, current);

#ifdef DUAL_BAND_VIRTUAL_INTERFACE	
	for(i=0,j=0 ; i < 3 ; i++,j++)
#else	
	for(i=0,j=0 ; i < CONFIG_MAC_NUM ; j++)
#endif	
	{	
		i=j;
#ifndef STORLINK_PHY 			// 3512 switch board with storlink phy not swap eth0/1 for debug pupose
#ifndef CONFIG_RTL8366SR_PHY	// RTL PHY do not swap eth0/1 
		if(Giga_switch){		// if gswitch present, swap eth0/1
			if(j==0)
				i=1;
			else if(j==1)
				i=0;
		}
#endif	//CONFIG_RTL8366SR_PHY	
#endif		
#ifdef DUAL_BAND_VIRTUAL_INTERFACE	
		if(i == 1)
		    tp = (GMAC_INFO_T *)&toe_private_data.gmac[1];
		else 
		    tp = (GMAC_INFO_T *)&toe_private_data.gmac[0];
#else		    
		tp = (GMAC_INFO_T *)&toe_private_data.gmac[i];
#endif		
		if (i != 2)
		tp->dev = NULL;
		
#ifndef DUAL_BAND_VIRTUAL_INTERFACE				
		if (tp->existed != GMAC_EXISTED_FLAG) continue;
#endif		
		
#ifndef DUAL_BAND_VIRTUAL_INTERFACE		
		dev = alloc_etherdev(0);
#else		
		dev = alloc_etherdev(sizeof (*tp));
#endif		
		
		if (dev == NULL)
		{
			printk (KERN_ERR "Can't allocate ethernet device #%d .\n",i);
			return -ENOMEM;
		}
#ifdef DUAL_BAND_VIRTUAL_INTERFACE			
		if(i==0)
		{		
			GMACVIF0 = dev;
			GMACVIF0->priv = tp;
		}
		if(i==2)
		{
			GMACVIF1 = dev;
			GMACVIF1->priv = tp;
		}
		if (i==1)
			dev->priv=tp;
			
		tp->dev = dev;
#else			
		dev->priv=tp;
		tp->dev = dev;
			
#endif	//DUAL_BAND_VIRTUAL_INTERFACE	
		SET_MODULE_OWNER(dev);

		// spin_lock_init(&tp->lock);
		spin_lock_init(&gmac_fq_lock);
		
	    dev->open = gmac_open;
	    dev->stop = gmac_close;
		dev->base_addr = tp->base_addr;
		dev->irq = tp->irq;
		dev->hard_start_xmit = gmac_start_xmit;
		dev->get_stats = gmac_get_stats;
		dev->set_multicast_list = gmac_set_rx_mode;
		dev->set_mac_address = gmac_set_mac_address;
		dev->do_ioctl = gmac_netdev_ioctl;
		dev->tx_timeout = gmac_tx_timeout;
		dev->watchdog_timeo = GMAC_DEV_TX_TIMEOUT;
#ifdef	L2_jumbo_frame
		dev->mtu = 2018; //2002  ,2018
#else
		dev->change_mtu = gmac_change_mtu;
#endif		

		if (tp->port_id == 0)
			dev->tx_queue_len = TOE_GMAC0_SWTXQ_DESC_NUM;
		else
			dev->tx_queue_len = TOE_GMAC1_SWTXQ_DESC_NUM;
			
#ifdef DO_HW_CHKSUM		
		dev->features |= NETIF_F_SG|NETIF_F_HW_CSUM;
#ifdef ENABLE_TSO
		dev->features |= NETIF_F_TSO;
#endif
#endif
		dev->features |= NETIF_F_HWNAT;
#ifdef CONFIG_SL_NAPI		
        dev->poll = gmac_rx_poll;
        dev->weight = 64;
#endif
		
		/* Configure the timeout task */
		INIT_WORK(&tp->tx_timeout_task,
			(void (*)(void *))gmac_reset_task, dev);
			
		init_waitqueue_head (&tp->thr_wait);
    	init_completion(&tp->thr_exited);	

		if (register_netdev(dev))
		{
			gmac_cleanup_module();
			return(-1);
		}

	}

	return (0);
}

/*----------------------------------------------------------------------
*	gmac_cleanup_module
*----------------------------------------------------------------------*/

static void gmac_cleanup_module(void)
{
    int i;

#ifdef SL351x_GMAC_WORKAROUND
	del_timer(&gmac_workround_timer_obj);
#endif

    for (i=0;i<CONFIG_MAC_NUM;i++) 
    {
    	if (toe_private_data.gmac[i].dev)
    	{
        	unregister_netdev(toe_private_data.gmac[i].dev);
        	toe_private_data.gmac[i].dev = NULL;
        }
    }    
	return ;
}

module_init(gmac_init_module);
module_exit(gmac_cleanup_module);


/*----------------------------------------------------------------------
*	gmac_read_reg
*----------------------------------------------------------------------*/
static inline unsigned int gmac_read_reg(unsigned int base, unsigned int offset)
//static unsigned int gmac_read_reg(unsigned int base, unsigned int offset)
{
    volatile unsigned int reg_val;

    reg_val = readl(base + offset);
	return (reg_val);
}

/*----------------------------------------------------------------------
*	gmac_write_reg
*----------------------------------------------------------------------*/
static inline void gmac_write_reg(unsigned int base, unsigned int offset,unsigned int data,unsigned int bit_mask)
//static void gmac_write_reg(unsigned int base, unsigned int offset,unsigned int data,unsigned int bit_mask)
{
	volatile unsigned int reg_val;
    unsigned int *addr;

	reg_val = ( gmac_read_reg(base, offset) & (~bit_mask) ) | (data & bit_mask);
	addr = (unsigned int *)(base + offset);
    writel(reg_val,addr);
	return;
}

/*----------------------------------------------------------------------
*	mac_init_drv
*----------------------------------------------------------------------*/
void mac_init_drv(void)
{
	TOE_INFO_T			*toe;
	int					i,ret;
	QUEUE_THRESHOLD_T	threshold;
	u32					*destp;
	unsigned int		chip_id,chip_version;
	
	chip_id = readl(GMAC_GLOBAL_BASE_ADDR+0x0);
	chip_version = chip_id & 0x1 ;
	
	if (!gmac_initialized)
	{
		gmac_initialized = 1;
		
		// clear non TOE Queue Header Area
		destp = (u32 *)TOE_NONTOE_QUE_HDR_BASE;
		for (; destp < (u32 *)NONTOE_Q_HDR_AREA_END; destp++)
			*destp = 0x00;

		// clear TOE Queue Header Area
		destp = (u32 *)TOE_TOE_QUE_HDR_BASE;
		for (; destp < (u32 *)TOE_Q_HDR_AREA_END; destp++)
			*destp = 0x00;
		
		// init private data
		toe = (TOE_INFO_T *)&toe_private_data;
		memset((void *)toe, 0, sizeof(TOE_INFO_T));
//joel		
#ifdef ALPHA_RTL8366SR_MODULE
		printk("GMAC_GLOBAL_BASE_ADDR+0x1c = 2 0x77777777 \n");
    	writel(0x77777777,GMAC_GLOBAL_BASE_ADDR+0x1c);//For rtl8366s Switch Board 
		printk(" NEW RTL8366RB RRRRRRRRRRRRRRRBBBBBBBBBBBBBBBBB init... \n");
		toe->gmac[0].base_addr = GMAC0_BASE;	
		toe->gmac[1].base_addr = GMAC1_BASE;	
		toe->gmac[0].dma_base_addr = TOE_GMAC0_DMA_BASE;
		toe->gmac[1].dma_base_addr = TOE_GMAC1_DMA_BASE;
        toe->gmac[0].auto_nego_cfg = 0;
        toe->gmac[1].auto_nego_cfg = 1;
        toe->gmac[0].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[1].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[0].full_duplex_cfg = 1;
        toe->gmac[1].full_duplex_cfg = 1;
        toe->gmac[0].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[1].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[0].port_id = GMAC_PORT0;
        toe->gmac[1].port_id = GMAC_PORT1;
        toe->gmac[0].phy_addr = 1;
        toe->gmac[1].phy_addr = 2;
        toe->gmac[0].irq =1;
        toe->gmac[1].irq =2;
        toe->gmac[0].mac_addr1 = &eth_mac[0][0];
        toe->gmac[1].mac_addr1 = &eth_mac[1][0];
        //toe->gmac[0].existed = GMAC_EXISTED_FLAG;
        //toe->gmac[1].existed = GMAC_EXISTED_FLAG;
        for (i=0; i<CONFIG_MAC_NUM; i++)
		{
			unsigned int data;
			gmac_write_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2, 0x55aa55aa, 0xffffffff);
			data = gmac_read_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2);
			if (data == 0x55aa55aa)
			{	
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;
			}
		}
#elif defined(RTL8366SR)
		printk("GMAC_GLOBAL_BASE_ADDR+0x1c = 2 0x77777777 \n");
    	writel(0x77777777,GMAC_GLOBAL_BASE_ADDR+0x1c);//For rtl8366s Switch Board 
		printk(" NEW RTL8366RB RRRRRRRRRRRRRRRBBBBBBBBBBBBBBBBB init... \n");
		smi_reset(0,0);
		//wait chip reset ...
		mdelay(100);
		for(i=0;i<3;i++)
		{
			//smi_reset(0, 0);			
			if((ret = rtl8366rb_initChip())==0)
				break;
			//mdelay(10);	
			printk("init fail,do again\n");
		}
			
		if ( ret != 0)		        
			printk("rtl8366s_initChip init failed!!\n");		    
		else		        
			printk(" rtl8366s_initChip init success!!\n");			

		//set green ethernet
	    if(0==rtl8366rb_setGreenEthernet(1,1))
	    	printk("set green ethernet success!!!\n");
	    else
	    	printk("set green ethernet fail!!!\n");
		
		//mdelay(50);
        if(0 == rtl8368s_setAsicMacForceLink(5,1,2, 1, 1, 1,1))
		    printk("Set Mac 5 Force Link to 1000M Full duplex success!!!\n");
        else
	        printk("Set Mac 5 Force Link to 1000M Full duplex fail  !!!\n");

		if(rtl8368s_setAsicReg(0xc,0x240)==0)
			printk("set 0xc to 0x240 success\n");
		else
			printk("set 0xc to 0x340 fail\n");

		if(rtl8368s_setAsicReg(0xd,0x240)==0)
			printk("set 0xd to 0x240 success\n");
		else
			printk("set 0xd to 0x240 fail\n");
		
#ifdef ALPHA_IGMP_SNOOP	    
		//hendry
		init_igmp_snoop();
#endif		
	    
#if 0
		toe->gmac[1].base_addr = GMAC1_BASE;	
		toe->gmac[1].dma_base_addr = TOE_GMAC1_DMA_BASE;
        toe->gmac[1].auto_nego_cfg = 1;
        toe->gmac[1].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[1].full_duplex_cfg = 1;
        toe->gmac[1].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[1].port_id = GMAC_PORT1;
        toe->gmac[1].phy_addr = 2;
        toe->gmac[1].irq =2;
        toe->gmac[1].mac_addr1 = &eth_mac[1][0];
        //toe->gmac[0].existed = GMAC_EXISTED_FLAG;
        //toe->gmac[1].existed = GMAC_EXISTED_FLAG;
        for (i=0; i<1; i++)
		{
			unsigned int data, phy_vendor;
			gmac_write_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2, 0x55aa55aa, 0xffffffff);
			data = gmac_read_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2);
			if (data == 0x55aa55aa)
			{	
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;
			}
		}
#else	    
		toe->gmac[0].base_addr = GMAC0_BASE;	
		toe->gmac[1].base_addr = GMAC1_BASE;	
		toe->gmac[0].dma_base_addr = TOE_GMAC0_DMA_BASE;
		toe->gmac[1].dma_base_addr = TOE_GMAC1_DMA_BASE;
        toe->gmac[0].auto_nego_cfg = 0;
        toe->gmac[1].auto_nego_cfg = 1;
        toe->gmac[0].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[1].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[0].full_duplex_cfg = 1;
        toe->gmac[1].full_duplex_cfg = 1;
        toe->gmac[0].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[1].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[0].port_id = GMAC_PORT0;
        toe->gmac[1].port_id = GMAC_PORT1;
        toe->gmac[0].phy_addr = 1;
        toe->gmac[1].phy_addr = 2;
        toe->gmac[0].irq =1;
        toe->gmac[1].irq =2;
        toe->gmac[0].mac_addr1 = &eth_mac[0][0];
        toe->gmac[1].mac_addr1 = &eth_mac[1][0];
        //toe->gmac[0].existed = GMAC_EXISTED_FLAG;
        //toe->gmac[1].existed = GMAC_EXISTED_FLAG;
        for (i=0; i<CONFIG_MAC_NUM; i++)
		{
			unsigned int data;
			gmac_write_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2, 0x55aa55aa, 0xffffffff);
			data = gmac_read_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2);
			if (data == 0x55aa55aa)
			{	
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;
			}
		}
#endif		
#else
		toe->gmac[0].base_addr = GMAC0_BASE;
		toe->gmac[1].base_addr = GMAC1_BASE;
		toe->gmac[0].dma_base_addr = TOE_GMAC0_DMA_BASE;
		toe->gmac[1].dma_base_addr = TOE_GMAC1_DMA_BASE;
        toe->gmac[0].auto_nego_cfg = 1;
        toe->gmac[1].auto_nego_cfg = 1;
#ifdef CONFIG_SL3516_ASIC
        toe->gmac[0].speed_cfg = GMAC_SPEED_1000;
        toe->gmac[1].speed_cfg = GMAC_SPEED_1000;
#else
		toe->gmac[0].speed_cfg = GMAC_SPEED_100;
        toe->gmac[1].speed_cfg = GMAC_SPEED_100;
#endif
        toe->gmac[0].full_duplex_cfg = 1;
        toe->gmac[1].full_duplex_cfg = 1;
#ifdef CONFIG_SL3516_ASIC
        toe->gmac[0].phy_mode = GMAC_PHY_RGMII_1000;
        toe->gmac[1].phy_mode = GMAC_PHY_RGMII_1000;
#else
		toe->gmac[0].phy_mode = GMAC_PHY_RGMII_100;
        toe->gmac[1].phy_mode = GMAC_PHY_RGMII_100;
#endif
        toe->gmac[0].port_id = GMAC_PORT0;
        toe->gmac[1].port_id = GMAC_PORT1;
#ifdef CONFIG_RTL8366SR_PHY
		toe->gmac[0].phy_addr = 0x0;	//depend on hardware layout, GMAC 0 --> RTL8366 Switch
        toe->gmac[1].phy_addr = 0x1;	//depend on hardware layout, GMAC 1 --> RTL PHY
#else        
        toe->gmac[0].phy_addr = 0x1;
        toe->gmac[1].phy_addr = 2;
#endif       
//      toe->gmac[0].irq = SL2312_INTER
		toe->gmac[0].irq =1;
//      toe->gmac[1].irq = SL2312_INTERRUPT_GMAC1;
		toe->gmac[1].irq =2;
        toe->gmac[0].mac_addr1 = &eth_mac[0][0];
        toe->gmac[1].mac_addr1 = &eth_mac[1][0];
#ifdef DUAL_BAND_VIRTUAL_INTERFACE        
        toe->gmac[0].mac_addr2 = &eth_mac[2][0];
#endif		
		for (i=0; i<CONFIG_MAC_NUM; i++)
		{
			unsigned int data, phy_vendor;
			gmac_write_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2, 0x55aa55aa, 0xffffffff);
			data = gmac_read_reg(toe->gmac[i].base_addr, GMAC_STA_ADD2);
			if (data == 0x55aa55aa)
			{
#ifdef CONFIG_RTL8366SR_PHY
				if(Giga_switch && (i==0)){
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;	
				}
				if(i==1){
					phy_vendor = gmac_get_phy_vendor(toe->gmac[i].phy_addr);
					if (phy_vendor != 0 && phy_vendor != 0xffffffff){
						toe->gmac[i].existed = GMAC_EXISTED_FLAG;
					}
				}						
#endif								
#ifdef VITESSE_G5SWITCH
				if(Giga_switch && (i==1)){
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;
					break;
				}
#endif
				phy_vendor = gmac_get_phy_vendor(toe->gmac[i].phy_addr);
				if (phy_vendor != 0 && phy_vendor != 0xffffffff)
					toe->gmac[i].existed = GMAC_EXISTED_FLAG;
			}
		}
#endif		
		// Write GLOBAL_QUEUE_THRESHOLD_REG
		threshold.bits32 = 0;
		threshold.bits.swfq_empty = (TOE_SW_FREEQ_DESC_NUM > 256) ? 255 :
		                                        TOE_SW_FREEQ_DESC_NUM/2;
		threshold.bits.hwfq_empty = (TOE_HW_FREEQ_DESC_NUM > 256) ? 256/4 :
		                                        TOE_HW_FREEQ_DESC_NUM/4;
		threshold.bits.toe_class = (TOE_TOE_DESC_NUM > 256) ? 256/4 : 
		                                        TOE_TOE_DESC_NUM/4;
		threshold.bits.intrq = (TOE_INTR_DESC_NUM > 256) ? 256/4 :
		                                        TOE_INTR_DESC_NUM/4;
		writel(threshold.bits32, TOE_GLOBAL_BASE + GLOBAL_QUEUE_THRESHOLD_REG);
		
		FLAG_SWITCH = 0;
		toe_gmac_sw_reset();
		toe_init_free_queue();
		toe_init_swtx_queue();
#ifdef CONFIG_SL351x_NAT
		toe_init_hwtx_queue();
#endif
#ifdef CONFIG_SL351X_BR
		toe_init_hwtx_queue();
#endif
		toe_init_default_queue();
#ifdef CONFIG_SL351x_RXTOE
		toe_init_interrupt_queue();
#endif
		toe_init_interrupt_config();

// for sl351x_ipsec (VPN)
#ifdef CONFIG_SL351X_IPSEC
		ipsec_init_class_queue();
		storlink_ctl.napi = 1;
#endif

#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351x_RXTOE) || defined(CONFIG_SL351X_BR) ||  defined(CONFIG_SL351X_IPSEC)
		sl351x_hash_init();
#else
	{
		volatile u32 *dp1, *dp2, dword;

		dp1 = (volatile u32 *) TOE_V_BIT_BASE;
		dp2 = (volatile u32 *) TOE_A_BIT_BASE;
	
		for (i=0; i<HASH_TOTAL_ENTRIES/32; i++)
		{
			*dp1++ = 0;
			dword = *dp2++;	// read-clear
		}
	}
#endif
	}
#ifdef SL351x_GMAC_WORKAROUND
#ifdef CONFIG_SL351x_NAT
	sl351x_nat_workaround_init();
#endif
	init_timer(&gmac_workround_timer_obj);
	gmac_workround_timer_obj.expires = jiffies + 2;
	gmac_workround_timer_obj.data = (unsigned long)&gmac_workround_timer_obj;
	gmac_workround_timer_obj.function = (void *)&sl351x_poll_gmac_hanged_status;
	add_timer(&gmac_workround_timer_obj);
#endif
}

/*----------------------------------------------------------------------
*	toe_init_free_queue
*	(1) Initialize the Free Queue Descriptor Base Address & size
*		Register: TOE_GLOBAL_BASE + 0x0004
*	(2) Initialize DMA Read/Write pointer for 
*		SW Free Queue and HW Free Queue
*	(3)	Initialize DMA Descriptors for
*		SW Free Queue and HW Free Queue, 
*----------------------------------------------------------------------*/
static void toe_init_free_queue(void)
{
	int 				i;
	TOE_INFO_T			*toe;
	DMA_RWPTR_T			rwptr_reg;
//	unsigned int 		rwptr_addr;
	unsigned int		desc_buf;
	GMAC_RXDESC_T		*sw_desc_ptr;
	struct sk_buff 		*skb;
#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
	GMAC_RXDESC_T		*desc_ptr;
	unsigned int		buf_ptr;
#endif
	
	toe = (TOE_INFO_T *)&toe_private_data;
	desc_buf = (unsigned int)DMA_MALLOC((TOE_SW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T)),
						(dma_addr_t *)&toe->sw_freeq_desc_base_dma) ;
	sw_desc_ptr = (GMAC_RXDESC_T *)desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return;
	}
	memset((void *)desc_buf, 0, TOE_SW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T));
	
	// DMA Queue Base & Size
	writel((toe->sw_freeq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_SW_FREEQ_DESC_POWER,
			TOE_GLOBAL_BASE + GLOBAL_SW_FREEQ_BASE_SIZE_REG);
			
	// init descriptor base
	toe->swfq_desc_base = desc_buf;
	
	// SW Free Queue Read/Write Pointer
	rwptr_reg.bits.wptr = TOE_SW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	toe->fq_rx_rwptr.bits32 = rwptr_reg.bits32;
	writel(rwptr_reg.bits32, TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
	
	// SW Free Queue Descriptors
	for (i=0; i<TOE_SW_FREEQ_DESC_NUM; i++)
	{
		sw_desc_ptr->word0.bits.buffer_size = SW_RX_BUF_SIZE;
		sw_desc_ptr->word1.bits.sw_id = i;	// used to locate skb
		if ( (skb = dev_alloc_skb(SW_RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
		{
			printk("%s::skb buffer allocation fail !\n",__func__); while(1);
		}
		REG32(skb->data) = (unsigned int)skb;
		skb_reserve(skb, SKB_RESERVE_BYTES);
		// toe->rx_skb[i] = skb;
		sw_desc_ptr->word2.buf_adr = (unsigned int)__pa(skb->data);
//   		consistent_sync((unsigned int)desc_ptr, sizeof(GMAC_RXDESC_T), PCI_DMA_TODEVICE);
   		sw_desc_ptr++;
	}

#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
	if (sizeof(skb->cb) < 64)
	{
			printk("==> %s:: sk structure is incorrect -->Change to cb[64] !\n",__func__); while(1);
	}
	// init hardware free queues
	desc_buf = (unsigned int)DMA_MALLOC((TOE_HW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T)),
						(dma_addr_t *)&toe->hw_freeq_desc_base_dma) ;
	desc_ptr = (GMAC_RXDESC_T *)desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return;
	}
	memset((void *)desc_buf, 0, TOE_HW_FREEQ_DESC_NUM * sizeof(GMAC_RXDESC_T));
	
	// DMA Queue Base & Size
	writel((toe->hw_freeq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_HW_FREEQ_DESC_POWER,
			TOE_GLOBAL_BASE + GLOBAL_HW_FREEQ_BASE_SIZE_REG);
			
	// init descriptor base
	toe->hwfq_desc_base = desc_buf;
	
	// HW Free Queue Read/Write Pointer
	rwptr_reg.bits.wptr = TOE_HW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	writel(rwptr_reg.bits32, TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);
#ifndef HW_RXBUF_BY_KMALLOC	
	buf_ptr = (unsigned int)DMA_MALLOC(TOE_HW_FREEQ_DESC_NUM * HW_RX_BUF_SIZE,
						(dma_addr_t *)&toe->hwfq_buf_base_dma);
#else						
	buf_ptr = (unsigned int)kmalloc(TOE_HW_FREEQ_DESC_NUM * HW_RX_BUF_SIZE, GFP_KERNEL);
	toe->hwfq_buf_base_dma = __pa(buf_ptr);
#endif
	if (!buf_ptr)
	{
		printk("===> %s::Failed to allocate HW TxQ Buffers!\n",__func__);
		while(1);	// could not be happened, if happened, adjust the buffer descriptor number
		return;
	}
	
	toe->hwfq_buf_base = buf_ptr;
	toe->hwfq_buf_end_dma = toe->hwfq_buf_base_dma + (TOE_HW_FREEQ_DESC_NUM * HW_RX_BUF_SIZE);
	buf_ptr = (unsigned int)toe->hwfq_buf_base_dma;
	for (i=0; i<TOE_HW_FREEQ_DESC_NUM; i++)
	{
		desc_ptr->word0.bits.buffer_size = HW_RX_BUF_SIZE;
		desc_ptr->word1.bits.sw_id = i;
		desc_ptr->word2.buf_adr = (unsigned int)buf_ptr;
//   		consistent_sync((unsigned int)desc_ptr, sizeof(GMAC_RXDESC_T), PCI_DMA_TODEVICE);
   		// consistent_sync((unsigned int)buf_ptr, HW_RX_BUF_SIZE, PCI_DMA_TODEVICE);
   		desc_ptr++;
   		buf_ptr += HW_RX_BUF_SIZE;
	}
#else
	// DMA Queue Base & Size
	writel((0) | TOE_SW_FREEQ_DESC_POWER,
			TOE_GLOBAL_BASE + GLOBAL_HW_FREEQ_BASE_SIZE_REG);
	rwptr_reg.bits.wptr = TOE_HW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	writel(rwptr_reg.bits32, TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);

#endif
}
/*----------------------------------------------------------------------
*	toe_init_swtx_queue
*	(2) Initialize the GMAC 0/1 SW TXQ Queue Descriptor Base Address & sizeup
*		GMAC_SW_TX_QUEUE_BASE_REG(0x0050)
*	(2) Initialize DMA Read/Write pointer for 
*		GMAC 0/1 SW TX Q0-5
*----------------------------------------------------------------------*/
static void toe_init_swtx_queue(void)
{
	int 				i;
	TOE_INFO_T			*toe;
	DMA_RWPTR_T			rwptr_reg;
	unsigned int 		rwptr_addr;
	unsigned int		desc_buf;

	
	toe = (TOE_INFO_T *)&toe_private_data;
	
	// GMAC-0, SW-TXQ
	// The GMAC-0 and GMAC-0 maybe have different descriptor number
	// so, not use for instruction
	desc_buf = (unsigned int)DMA_MALLOC((TOE_GMAC0_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T)),
						(dma_addr_t *)&toe->gmac[0].swtxq_desc_base_dma) ;
	toe->gmac[0].swtxq_desc_base = desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_buf, 0,	TOE_GMAC0_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T));
	writel((toe->gmac[0].swtxq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_GMAC0_SWTXQ_DESC_POWER,
			TOE_GMAC0_DMA_BASE+ GMAC_SW_TX_QUEUE_BASE_REG);
	
	// GMAC0 SW TX Q0-Q5
	rwptr_reg.bits.wptr = 0;
	rwptr_reg.bits.rptr = 0;
	rwptr_addr = TOE_GMAC0_DMA_BASE + GMAC_SW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_SW_TXQ_NUM; i++)
	{
		toe->gmac[0].swtxq[i].rwptr_reg = rwptr_addr;
		toe->gmac[0].swtxq[i].desc_base = desc_buf;
		toe->gmac[0].swtxq[i].total_desc_num = TOE_GMAC0_SWTXQ_DESC_NUM;
		desc_buf += TOE_GMAC0_SWTXQ_DESC_NUM * sizeof(GMAC_TXDESC_T);
		writel(rwptr_reg.bits32, rwptr_addr);
		rwptr_addr+=4;
	}
		
	// GMAC-1, SW-TXQ
	desc_buf = (unsigned int)DMA_MALLOC((TOE_GMAC1_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T)),
						(dma_addr_t *)&toe->gmac[1].swtxq_desc_base_dma) ;
	toe->gmac[1].swtxq_desc_base = desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_buf, 0,	TOE_GMAC1_SWTXQ_DESC_NUM * TOE_SW_TXQ_NUM * sizeof(GMAC_TXDESC_T));
	writel((toe->gmac[1].swtxq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_GMAC1_SWTXQ_DESC_POWER,
			TOE_GMAC1_DMA_BASE+ GMAC_SW_TX_QUEUE_BASE_REG);
	
	
	// GMAC1 SW TX Q0-Q5
	rwptr_reg.bits.wptr = 0;
	rwptr_reg.bits.rptr = 0;
	rwptr_addr = TOE_GMAC1_DMA_BASE + GMAC_SW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_SW_TXQ_NUM; i++)
	{
		toe->gmac[1].swtxq[i].rwptr_reg = rwptr_addr;
		toe->gmac[1].swtxq[i].desc_base = desc_buf;
		toe->gmac[1].swtxq[i].total_desc_num = TOE_GMAC1_SWTXQ_DESC_NUM;
		desc_buf += TOE_GMAC1_SWTXQ_DESC_NUM * sizeof(GMAC_TXDESC_T);
		writel(rwptr_reg.bits32, rwptr_addr);
		rwptr_addr+=4;
	}
}

/*----------------------------------------------------------------------
*	toe_init_hwtx_queue
*	(2) Initialize the GMAC 0/1 HW TXQ Queue Descriptor Base Address & size
*		GMAC_HW_TX_QUEUE_BASE_REG(0x0054)
*	(2) Initialize DMA Read/Write pointer for 
*		GMAC 0/1 HW TX Q0-5
*----------------------------------------------------------------------*/
#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
static void toe_init_hwtx_queue(void)
{
	int 				i;
	TOE_INFO_T			*toe;
	DMA_RWPTR_T			rwptr_reg;
	unsigned int 		rwptr_addr;
	unsigned int		desc_buf;
	
	toe = (TOE_INFO_T *)&toe_private_data;
	// GMAC-0, HW-TXQ
	// The GMAC-0 and GMAC-0 maybe have different descriptor number
	// so, not use for instruction
	desc_buf = (unsigned int)DMA_MALLOC((TOE_GMAC0_HWTXQ_DESC_NUM * TOE_HW_TXQ_NUM * sizeof(GMAC_TXDESC_T)),
						(dma_addr_t *)&toe->gmac[0].hwtxq_desc_base_dma) ;
	toe->gmac[0].hwtxq_desc_base = desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_buf, 0,	TOE_GMAC0_HWTXQ_DESC_NUM * TOE_HW_TXQ_NUM * sizeof(GMAC_TXDESC_T));
	writel((toe->gmac[0].hwtxq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_GMAC0_HWTXQ_DESC_POWER,
			TOE_GMAC0_DMA_BASE+ GMAC_HW_TX_QUEUE_BASE_REG);
	
	// GMAC0 HW TX Q0-Q5
	rwptr_reg.bits.wptr = 0;
	rwptr_reg.bits.rptr = 0;
	rwptr_addr = TOE_GMAC0_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		toe->gmac[0].hwtxq[i].desc_base = desc_buf;
		desc_buf += TOE_GMAC0_HWTXQ_DESC_NUM * sizeof(GMAC_TXDESC_T);
		writel(rwptr_reg.bits32, rwptr_addr);
		rwptr_addr+=4;
	}
		
	// GMAC-1, HW-TXQ
	desc_buf = (unsigned int)DMA_MALLOC((TOE_GMAC1_HWTXQ_DESC_NUM * TOE_HW_TXQ_NUM * sizeof(GMAC_TXDESC_T)),
						(dma_addr_t *)&toe->gmac[1].hwtxq_desc_base_dma) ;
	toe->gmac[1].hwtxq_desc_base = desc_buf;
	if (!desc_buf)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_buf, 0,	TOE_GMAC1_HWTXQ_DESC_NUM * TOE_HW_TXQ_NUM * sizeof(GMAC_TXDESC_T));
	writel((toe->gmac[1].hwtxq_desc_base_dma & DMA_Q_BASE_MASK) | TOE_GMAC1_HWTXQ_DESC_POWER,
			TOE_GMAC1_DMA_BASE+ GMAC_HW_TX_QUEUE_BASE_REG);
	
	// GMAC1 HW TX Q0-Q5
	rwptr_reg.bits.wptr = 0;
	rwptr_reg.bits.rptr = 0;
	rwptr_addr = TOE_GMAC1_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		toe->gmac[1].hwtxq[i].desc_base = desc_buf;
		desc_buf += TOE_GMAC1_HWTXQ_DESC_NUM * sizeof(GMAC_TXDESC_T);
		writel(rwptr_reg.bits32, rwptr_addr);
		rwptr_addr+=4;
	}
}
#endif

/*----------------------------------------------------------------------
*	toe_init_default_queue
*	(1) Initialize the default 0/1 Queue Header
*		Register: TOE_DEFAULT_Q0_HDR_BASE (0x60002000)
*				  TOE_DEFAULT_Q1_HDR_BASE (0x60002008)
*	(2)	Initialize Descriptors of Default Queue 0/1
*----------------------------------------------------------------------*/
static void toe_init_default_queue(void)
{
	TOE_INFO_T				*toe;
	volatile NONTOE_QHDR_T	*qhdr;
	GMAC_RXDESC_T			*desc_ptr;
	DMA_SKB_SIZE_T			skb_size;
	
	toe = (TOE_INFO_T *)&toe_private_data;
	desc_ptr = (GMAC_RXDESC_T *)DMA_MALLOC((TOE_DEFAULT_Q0_DESC_NUM * sizeof(GMAC_RXDESC_T)),
											(dma_addr_t *)&toe->gmac[0].default_desc_base_dma);
	if (!desc_ptr)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_ptr, 0, TOE_DEFAULT_Q0_DESC_NUM * sizeof(GMAC_RXDESC_T));
	toe->gmac[0].default_desc_base = (unsigned int)desc_ptr;				
	toe->gmac[0].default_desc_num = TOE_DEFAULT_Q0_DESC_NUM;
	qhdr = (volatile NONTOE_QHDR_T *)TOE_DEFAULT_Q0_HDR_BASE;
	qhdr->word0.base_size = ((unsigned int)toe->gmac[0].default_desc_base_dma & NONTOE_QHDR0_BASE_MASK) | TOE_DEFAULT_Q0_DESC_POWER;
	qhdr->word1.bits32 = 0;
	toe->gmac[0].rx_rwptr.bits32 = 0;
	toe->gmac[0].default_qhdr = (NONTOE_QHDR_T *)qhdr;
	desc_ptr = (GMAC_RXDESC_T *)DMA_MALLOC((TOE_DEFAULT_Q1_DESC_NUM * sizeof(GMAC_RXDESC_T)),
											(dma_addr_t *)&toe->gmac[1].default_desc_base_dma);
	if (!desc_ptr)
	{
		printk("%s::DMA_MALLOC fail !\n",__func__);
		return	;
	}
	memset((void *)desc_ptr, 0, TOE_DEFAULT_Q1_DESC_NUM * sizeof(GMAC_RXDESC_T));
	toe->gmac[1].default_desc_base = (unsigned int)desc_ptr;				
	toe->gmac[1].default_desc_num = TOE_DEFAULT_Q1_DESC_NUM;
	qhdr = (volatile NONTOE_QHDR_T *)TOE_DEFAULT_Q1_HDR_BASE;
	qhdr->word0.base_size = ((unsigned int)toe->gmac[1].default_desc_base_dma & NONTOE_QHDR0_BASE_MASK) | TOE_DEFAULT_Q1_DESC_POWER;
	qhdr->word1.bits32 = 0;
	toe->gmac[1].rx_rwptr.bits32 = 0;
	toe->gmac[1].default_qhdr = (NONTOE_QHDR_T *)qhdr;
	
	skb_size.bits.hw_skb_size = HW_RX_BUF_SIZE;
	skb_size.bits.sw_skb_size = SW_RX_BUF_SIZE;
	writel(skb_size.bits32, TOE_GLOBAL_BASE + GLOBAL_DMA_SKB_SIZE_REG);
}

/*----------------------------------------------------------------------
*	toe_init_interrupt_queue
*	(1) Initialize the Interrupt Queue Header
*		Register: TOE_INTR_Q_HDR_BASE (0x60002080)
*	(2)	Initialize Descriptors of Interrupt Queues
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_RXTOE
static void toe_init_interrupt_queue(void)
{
	TOE_INFO_T				*toe;
	volatile NONTOE_QHDR_T	*qhdr;
	INTR_QHDR_T				*desc_ptr;
	// unsigned int			desc_buf_addr;
	int						i;
	
	toe = (TOE_INFO_T *)&toe_private_data;
	desc_ptr = (INTR_QHDR_T *)DMA_MALLOC((TOE_INTR_QUEUE_NUM * TOE_INTR_DESC_NUM * sizeof(INTR_QHDR_T)),
											(dma_addr_t *)&toe->intr_desc_base_dma);
	if (!desc_ptr)
	{
		printk("%s::DMA_MALLOC interrupt queue fail !\n",__func__); 
		return	;
	}
	/*
	desc_buf_addr = (unsigned int)DMA_MALLOC((TOE_INTR_DESC_NUM * sizeof(TOE_QHDR_T)),
												(dma_addr_t *)&toe->intr_buf_base_dma);
	if (!desc_buf_addr)
	{
		printk("%s::DMA_MALLOC interrupt desc fail !\n",__func__); 
		return	;
	}*/
	printk("#### %s::Intr Q desc %x\n", __func__, (u32)desc_ptr);

	memset((void *)desc_ptr, 0, TOE_INTR_QUEUE_NUM * TOE_INTR_DESC_NUM * sizeof(INTR_QHDR_T));
//	memset((void *)desc_buf_addr, 0, TOE_INTR_DESC_NUM * sizeof(TOE_QHDR_T));
	toe->intr_desc_base = (unsigned int)desc_ptr;				
	toe->intr_desc_num = TOE_INTR_DESC_NUM;
	
	qhdr = (volatile NONTOE_QHDR_T *)TOE_INTR_Q_HDR_BASE;
//	intrq = (INTRQ_INFO_T*) &toe->intrq[0];
	for (i=0; i<TOE_INTR_QUEUE_NUM; i++, qhdr++)
	{
		qhdr->word0.base_size = ((unsigned int)toe->intr_desc_base_dma & NONTOE_QHDR0_BASE_MASK) | TOE_INTR_DESC_POWER;
		qhdr->word1.bits32 = 0;
		desc_ptr += TOE_INTR_DESC_NUM;
	}
}

#endif

/*----------------------------------------------------------------------
*	toe_init_interrupt_config
*	Interrupt Select Registers are used to map interrupt to int0 or int1
*	Int0 and int1 are wired to CPU 0/1 GMAC 0/1
* 	Interrupt Device Inteface data are used to pass device info to
*		upper device deiver or store status/statistics
*	ISR handler
*		(1) If status bit ON but masked, the prinf error message (bug issue)
*		(2) If select bits are for me, handle it, else skip to let 
*			the other ISR handles it.
*  Notes:
*		GMACx init routine (for eCOS) or open routine (for Linux)
*       enable the interrupt bits only which are selected for him.
*
*	Default Setting:
*		GMAC0 intr bits ------>	int0 ----> eth0
*		GMAC1 intr bits ------> int1 ----> eth1
*		TOE intr -------------> int0 ----> eth0
*		Classification Intr --> int0 ----> eth0
*		Default Q0 -----------> int0 ----> eth0
*		Default Q1 -----------> int1 ----> eth1
*----------------------------------------------------------------------*/
static void toe_init_interrupt_config(void)
{
	// clear all status bits
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_0_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_1_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_2_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_3_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_4_REG);
	
	// Init select registers
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_0_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_1_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_2_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_3_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_4_REG);
	
	// disable all interrupt
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_0_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_2_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_3_REG);
	writel(0, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_4_REG);
}

/*----------------------------------------------------------------------
*	toe_init_gmac
*----------------------------------------------------------------------*/
static void toe_init_gmac(struct net_device *dev)
{
	GMAC_INFO_T		*tp = dev->priv;
	TOE_INFO_T		*toe;
	u32 			data;
	
	if (!gmac_initialized)
		return ;
		
	if (!tp->existed)
		return;

	tp->dev = dev;
	tp->flow_control_enable = 1;
	tp->pre_phy_status = LINK_DOWN;
	tp->full_duplex_status = tp->full_duplex_cfg;
	tp->speed_status = tp->speed_status;

   /* get mac address from FLASH */
    gmac_get_mac_address();	
	
    /* set PHY register to start autonegition process */
    gmac_set_phy_status(dev);

	/* GMAC initialization */
	if ( toe_gmac_init_chip(dev) ) 
	{
		printk ("GMAC %d init fail\n", tp->port_id);
	}	

    /* clear statistic counter */
    toe_gmac_clear_counter(dev);
	
	memset((void *)&tp->ifStatics, 0, sizeof(struct net_device_stats));

	/* -----------------------------------------------------------
	Enable GMAC interrupt & disable loopback 
	Notes:
		GMACx init routine (for eCOS) or open routine (for Linux)
		enable the interrupt bits only which are selected for him.
	--------------------------------------------------------------*/
	toe = (TOE_INFO_T *)&toe_private_data;
	
	// Enable Interrupt Bits
	if (tp->port_id == 0)
	{
		tp->intr0_selected =	GMAC0_TXDERR_INT_BIT	 | GMAC0_TXPERR_INT_BIT		|
	                         	GMAC0_RXDERR_INT_BIT	 | GMAC0_RXPERR_INT_BIT		|	
	                            GMAC0_SWTQ05_FIN_INT_BIT | GMAC0_SWTQ05_EOF_INT_BIT |
	                            GMAC0_SWTQ04_FIN_INT_BIT | GMAC0_SWTQ04_EOF_INT_BIT |
	                            GMAC0_SWTQ03_FIN_INT_BIT | GMAC0_SWTQ03_EOF_INT_BIT |
	                            GMAC0_SWTQ02_FIN_INT_BIT | GMAC0_SWTQ02_EOF_INT_BIT |
	                            GMAC0_SWTQ01_FIN_INT_BIT | GMAC0_SWTQ01_EOF_INT_BIT |
	                            GMAC0_SWTQ00_FIN_INT_BIT | GMAC0_SWTQ00_EOF_INT_BIT;

#ifdef GMAX_TX_INTR_DISABLED
	    tp->intr0_enabled =		0;
#else
	    tp->intr0_enabled =		GMAC0_SWTQ00_FIN_INT_BIT | GMAC0_SWTQ00_EOF_INT_BIT;
#endif

#ifdef CONFIG_SL351X_IPSEC
		if (Giga_switch) {
			tp->intr1_selected =	TOE_IQ_ALL_BITS			 | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)	|
		    						GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
									GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT |
	    							DEFAULT_Q0_INT_BIT;
			tp->intr1_enabled  =	DEFAULT_Q0_INT_BIT		 | TOE_IQ_ALL_BITS |
									CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
		}
		else {
			tp->intr1_selected =	TOE_IQ_ALL_BITS			 | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)	|
		    						GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
									GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT |
	    							DEFAULT_Q0_INT_BIT;
			tp->intr1_enabled  =	DEFAULT_Q0_INT_BIT		 | TOE_IQ_ALL_BITS |
									CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
		}
#else
	    tp->intr1_selected =	TOE_IQ_ALL_BITS			 | TOE_CLASS_RX_INT_BITS	|
	    						GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
	    						GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT |
	    						DEFAULT_Q0_INT_BIT;
	    tp->intr1_enabled = 	DEFAULT_Q0_INT_BIT | TOE_IQ_ALL_BITS;
#endif
	    tp->intr2_selected = 	0xffffffff;	 // TOE Queue 32-63 FUUL Intr
	    tp->intr2_enabled = 	0xffffffff;
	    tp->intr3_selected = 	0xffffffff;	 // TOE Queue 0-31 FUUL Intr
	    tp->intr3_enabled = 	0xffffffff;
	    tp->intr4_selected = 	GMAC0_INT_BITS | CLASS_RX_FULL_INT_BITS | 
	    						HWFQ_EMPTY_INT_BIT | SWFQ_EMPTY_INT_BIT;
	    tp->intr4_enabled = 	GMAC0_INT_BITS | SWFQ_EMPTY_INT_BIT;
	    
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_0_REG) & ~tp->intr0_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_0_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_1_REG) & ~tp->intr1_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_1_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_2_REG) & ~tp->intr2_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_2_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_3_REG) & ~tp->intr3_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_3_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_4_REG) & ~tp->intr4_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_4_REG);
	}
	else
	{
		tp->intr0_selected =	GMAC1_TXDERR_INT_BIT	 | GMAC1_TXPERR_INT_BIT		|
	                         	GMAC1_RXDERR_INT_BIT	 | GMAC1_RXPERR_INT_BIT		|	
	                            GMAC1_SWTQ15_FIN_INT_BIT | GMAC1_SWTQ15_EOF_INT_BIT |
	                            GMAC1_SWTQ14_FIN_INT_BIT | GMAC1_SWTQ14_EOF_INT_BIT |
	                            GMAC1_SWTQ13_FIN_INT_BIT | GMAC1_SWTQ13_EOF_INT_BIT |
	                            GMAC1_SWTQ12_FIN_INT_BIT | GMAC1_SWTQ12_EOF_INT_BIT |
	                            GMAC1_SWTQ11_FIN_INT_BIT | GMAC1_SWTQ11_EOF_INT_BIT |
	                            GMAC1_SWTQ10_FIN_INT_BIT | GMAC1_SWTQ10_EOF_INT_BIT;
#ifdef GMAX_TX_INTR_DISABLED
	    tp->intr0_enabled =		0;
#else
	    tp->intr0_enabled =		GMAC1_SWTQ10_FIN_INT_BIT | GMAC1_SWTQ10_EOF_INT_BIT;
#endif
	    
#ifdef CONFIG_SL351X_IPSEC
		if (Giga_switch) {
			tp->intr1_selected =	DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
			tp->intr1_enabled  =	DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
		}
		else {
			tp->intr1_selected =	DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
			tp->intr1_enabled  =	DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
		}
#else
	    tp->intr1_selected =	DEFAULT_Q1_INT_BIT;
	    tp->intr1_enabled = 	DEFAULT_Q1_INT_BIT;
#endif
		tp->intr2_selected = 	0;	 // TOE Queue 32-63 FUUL Intr
	    tp->intr2_enabled = 	0;
	    tp->intr3_selected = 	0;	 // TOE Queue 0-31 FUUL Intr
	    tp->intr3_enabled = 	0;
	    tp->intr4_selected = 	GMAC1_INT_BITS;
	    tp->intr4_enabled = 	GMAC1_INT_BITS;
	    
	    if (toe->gmac[0].existed != GMAC_EXISTED_FLAG)
	    {
#ifdef CONFIG_SL351X_IPSEC
			if (Giga_switch) {
				tp->intr1_selected	|=	TOE_IQ_ALL_BITS	 | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)	|
		    							GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
										GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT;
				tp->intr1_enabled	|=	TOE_IQ_ALL_BITS | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
			}
			else {
				tp->intr1_selected	|=	TOE_IQ_ALL_BITS	 | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)	|
		    							GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
										GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT;
				tp->intr1_enabled	|=	TOE_IQ_ALL_BITS | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
			}
#else
			tp->intr1_selected	|= 	TOE_IQ_ALL_BITS | TOE_CLASS_RX_INT_BITS	|
	    						  	GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT |
	    						  	GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT;
	    	tp->intr1_enabled	|= 	TOE_IQ_ALL_BITS;
#endif
	    	tp->intr2_selected	|= 	0xffffffff;	 // TOE Queue 32-63 FUUL Intr
	    	tp->intr2_enabled	|= 	0xffffffff;
	    	tp->intr3_selected	|= 	0xffffffff;	 // TOE Queue 0-31 FUUL Intr
	    	tp->intr3_enabled	|= 	0xffffffff;
	    	tp->intr4_selected 	|= 	CLASS_RX_FULL_INT_BITS | 
	    							HWFQ_EMPTY_INT_BIT | SWFQ_EMPTY_INT_BIT;
	    	tp->intr4_enabled	|= 	SWFQ_EMPTY_INT_BIT;
		}
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_0_REG) | tp->intr0_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_0_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_1_REG) | tp->intr1_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_1_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_2_REG) | tp->intr2_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_2_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_3_REG) | tp->intr3_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_3_REG);
	    data = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_4_REG) | tp->intr4_selected;
	    writel(data, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_SELECT_4_REG);
	}
	
	// enable only selected bits
	gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_0_REG, 
					tp->intr0_enabled, tp->intr0_selected);
	gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_1_REG, 
					tp->intr1_enabled, tp->intr1_selected);
	gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_2_REG, 
					tp->intr2_enabled, tp->intr2_selected);
	gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_3_REG, 
					tp->intr3_enabled, tp->intr3_selected);
	gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_4_REG, 
					tp->intr4_enabled, tp->intr4_selected);

    /* start DMA process */
	toe_gmac_hw_start(dev);

    /* enable tx/rx register */    
    toe_gmac_enable_tx_rx(dev);

//	toe_gmac_enable_interrupt(tp->irq);
	
    return ;
}	


/*----------------------------------------------------------------------
* toe_gmac_sw_reset
*----------------------------------------------------------------------*/
static void toe_gmac_sw_reset(void)
{
	unsigned int	reg_val;
	reg_val = readl(GMAC_GLOBAL_BASE_ADDR+GLOBAL_RESET_REG) | 0x00000060;   /* GMAC0 S/W reset */
    writel(reg_val,GMAC_GLOBAL_BASE_ADDR+GLOBAL_RESET_REG);
    udelay(100);
    return;
}

/*----------------------------------------------------------------------
*	toe_gmac_init_chip
*----------------------------------------------------------------------*/
static int toe_gmac_init_chip(struct net_device *dev)
{
	GMAC_INFO_T 	*tp = dev->priv;
	GMAC_CONFIG2_T	config2_val;
	GMAC_CONFIG0_T	config0,config0_mask;
	GMAC_CONFIG1_T	config1;
	#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
	GMAC_CONFIG3_T	config3_val;
	#endif
	GMAC_TX_WCR0_T	hw_weigh;
	GMAC_TX_WCR1_T	sw_weigh;
	struct sockaddr sock;
#ifdef DUAL_BAND_VIRTUAL_INTERFACE  
	struct sockaddr sock_2;
#endif		
	// GMAC_AHB_WEIGHT_T	ahb_weight, ahb_weight_mask;


	/* set station MAC address1 and address2 */
	memcpy(&sock.sa_data[0],&eth_mac[tp->port_id][0],6);
	gmac_set_mac_address(dev,(void *)&sock);

	/* set RX_FLTR register to receive all multicast packet */
	gmac_write_reg(tp->base_addr, GMAC_RX_FLTR, 0x00000007,0x0000001f);
	//    gmac_write_reg(tp->base_addr, GMAC_RX_FLTR, 0x00000007,0x0000001f);
	//gmac_write_reg(tp->base_addr, GMAC_RX_FLTR,0x00000007,0x0000001f);

	/* set per packet buffer size */
	//	config1.bits32 = 0x002004;	//next version
	/* set flow control threshold */
	config1.bits32 = 0;
	config1.bits.set_threshold = 32 / 2;
	config1.bits.rel_threshold = 32 / 4 * 3;
	gmac_write_reg(tp->base_addr, GMAC_CONFIG1, config1.bits32, 0xffffffff);

	/* set flow control threshold */
	config2_val.bits32 = 0;
	config2_val.bits.set_threshold = TOE_SW_FREEQ_DESC_NUM/2;
	config2_val.bits.rel_threshold = TOE_SW_FREEQ_DESC_NUM*3/4;
	gmac_write_reg(tp->base_addr, GMAC_CONFIG2, config2_val.bits32,0xffffffff);

	#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
	/* set HW free queue flow control threshold */
	config3_val.bits32 = 0;
	config3_val.bits.set_threshold = PAUSE_SET_HW_FREEQ;
	config3_val.bits.rel_threshold = PAUSE_REL_HW_FREEQ;
	gmac_write_reg(tp->base_addr, GMAC_CONFIG3, config3_val.bits32,0xffffffff);
	#endif
	/* set_mcast_filter mask*/
	//	gmac_write_reg(tp->base_addr,GMAC_MCAST_FIL0,0x0,0xffffffff);
	//  gmac_write_reg(tp->base_addr,GMAC_MCAST_FIL1,0x0,0xffffffff);

	/* disable TX/RX and disable internal loop back */
	config0.bits32 = 0;
	config0_mask.bits32 = 0;

	//debug_Aaron
#ifdef	L2_jumbo_frame
	config0.bits.max_len = 5;
#else
	config0.bits.max_len = 2;
#endif

	if (tp->flow_control_enable==1)
	{
		config0.bits.tx_fc_en = 1; /* enable tx flow control */
		config0.bits.rx_fc_en = 1; /* enable rx flow control */
		printk("Enable MAC Flow Control...\n");
	}
	else
	{
		config0.bits.tx_fc_en = 0; /* disable tx flow control */
		config0.bits.rx_fc_en = 0; /* disable rx flow control */
		printk("Disable MAC Flow Control...\n");
	}
	config0.bits.dis_rx = 1;  /* disable rx */
	config0.bits.dis_tx = 1;  /* disable tx */
	config0.bits.loop_back = 0; /* enable/disable GMAC loopback */
	config0.bits.rx_err_detect = 1;
	config0.bits.rgmii_en = 0;
	config0.bits.rgmm_edge = 1;
	config0.bits.rxc_inv = 0;
	config0.bits.ipv4_rx_chksum = 1;  /* enable H/W to check ip checksum */
	config0.bits.ipv6_rx_chksum = 1;  /* enable H/W to check ip checksum */
	config0.bits.port0_chk_hwq = 1;	// GaryChen 3/24/2006 2:26PM
	config0.bits.port1_chk_hwq = 1;	// GaryChen 3/24/2006 2:26PM
	config0.bits.port0_chk_toeq = 1;
	config0.bits.port1_chk_toeq = 1;
	config0.bits.port0_chk_classq = 1;
	config0.bits.port1_chk_classq = 1;

	config0_mask.bits.max_len = 7;
	config0_mask.bits.tx_fc_en = 1;
	config0_mask.bits.rx_fc_en = 1;
	config0_mask.bits.dis_rx = 1;
	config0_mask.bits.dis_tx = 1;
	config0_mask.bits.loop_back = 1;
	config0_mask.bits.rgmii_en = 1;
	config0_mask.bits.rgmm_edge = 1;
	config0_mask.bits.rxc_inv = 1;
	config0_mask.bits.ipv4_rx_chksum = 1;
	config0_mask.bits.ipv6_rx_chksum = 1;
	config0_mask.bits.port0_chk_hwq = 1;
	config0_mask.bits.port1_chk_hwq = 1;
	config0_mask.bits.port0_chk_toeq = 1;
	config0_mask.bits.port1_chk_toeq = 1;
	config0_mask.bits.port0_chk_classq = 1;
	config0_mask.bits.port1_chk_classq = 1;
	config0_mask.bits.rx_err_detect = 1;

	#if 0
	config0.bits.dis_rx = 1;  /* disable rx */
	config0.bits.dis_tx = 1;  /* disable tx */
	config0.bits.loop_back = 0; /* enable/disable GMAC loopback */
	config0.bits.txc_inv = 0;
	config0.bits.rgmii_en = 0;
	config0.bits.rgmm_edge = 1;
	config0.bits.rxc_inv = 1;
	config0.bits.ipv4_tss_rx_en = 1;  /* enable H/W to check ip checksum */
	config0.bits.ipv6_tss_rx_en = 1;  /* enable H/W to check ip checksum */

	config0_mask.bits.max_len = 3;
	config0_mask.bits.tx_fc_en = 1;
	config0_mask.bits.rx_fc_en = 1;
	config0_mask.bits.dis_rx = 1;
	config0_mask.bits.dis_tx = 1;
	config0_mask.bits.loop_back = 1;
	config0_mask.bits.rgmii_en = 1;
	config0_mask.bits.rgmm_edge = 1;
	config0_mask.bits.txc_inv = 1;
	config0_mask.bits.rxc_inv = 1;
	config0_mask.bits.ipv4_tss_rx_en = 1;
	config0_mask.bits.ipv6_tss_rx_en = 1;
	#endif

	gmac_write_reg(tp->base_addr, GMAC_CONFIG0, config0.bits32,config0_mask.bits32);

	#if 1
	hw_weigh.bits32 = 0;
	hw_weigh.bits.hw_tq3 = 1;
	hw_weigh.bits.hw_tq2 = 1;
	hw_weigh.bits.hw_tq1 = 1;
	hw_weigh.bits.hw_tq0 = 1;
	gmac_write_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_0_REG, hw_weigh.bits32, 0xffffffff);

	sw_weigh.bits32 = 0;
	sw_weigh.bits.sw_tq5 = 1;
	sw_weigh.bits.sw_tq4 = 1;
	sw_weigh.bits.sw_tq3 = 1;
	sw_weigh.bits.sw_tq2 = 1;
	sw_weigh.bits.sw_tq1 = 1;
	sw_weigh.bits.sw_tq0 = 1;
	gmac_write_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_1_REG, sw_weigh.bits32, 0xffffffff);
	#endif

	#if 0
	ahb_weight.bits32 = 0;
	ahb_weight_mask.bits32 = 0;
	ahb_weight.bits.rx_weight = 1;
	ahb_weight.bits.tx_weight = 1;
	ahb_weight.bits.hash_weight = 1;
	ahb_weight.bits.pre_req = 0x1f;
	ahb_weight.bits.tqDV_threshold = 0;
	ahb_weight_mask.bits.rx_weight = 0x1f;
	ahb_weight_mask.bits.tx_weight = 0x1f;
	ahb_weight_mask.bits.hash_weight = 0x1f;
	ahb_weight_mask.bits.pre_req = 0x1f;
	ahb_weight_mask.bits.tqDV_threshold = 0x1f;
	gmac_write_reg(tp->dma_base_addr, GMAC_AHB_WEIGHT_REG, ahb_weight.bits32, ahb_weight_mask.bits32);
	#endif

	#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351x_RXTOE) || defined(CONFIG_SL351X_BR) || defined(CONFIG_SL351X_IPSEC)
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR0, IPPROTO_TCP, 0xffffffff);
	#endif

	#if defined(CONFIG_SL351x_NAT) || defined(CONFIG_SL351X_BR)
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR1, IPPROTO_UDP, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR2, IPPROTO_GRE, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR3, 0xff, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR4, 0xff, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR5, 0xff, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR6, 0xff, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR7, 0xff, 0xffffffff);

#ifdef CONFIG_SL351x_NAT
	sl351x_nat_init();
#endif
#ifdef CONFIG_SL351X_BR
	sl351x_br_init();
#endif	
	#endif

#ifdef CONFIG_SL351X_IPSEC
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR3, IPPROTO_ESP, 0xffffffff);
	gmac_write_reg(tp->dma_base_addr, GMAC_SPR4, IPPROTO_AH, 0xffffffff);
#endif

	#ifdef CONFIG_SL351x_RXTOE
	/* setup matching rule to TOE */
	sl351x_toe_init();
	#endif

	#ifdef CONFIG_SL351X_IPSEC
	sl351x_ipsec_init();
	// debug message
	// printk("Done Initializaion of IPSEC in GMAC\n");
	#endif


	return (0);
}

/*----------------------------------------------------------------------
*	toe_gmac_enable_tx_rx
*----------------------------------------------------------------------*/
static void toe_gmac_enable_tx_rx(struct net_device *dev)
{
	GMAC_INFO_T		*tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 0;  /* enable rx */
    config0.bits.dis_tx = 0;  /* enable tx */
    config0_mask.bits.dis_rx = 1;
    config0_mask.bits.dis_tx = 1;
    gmac_write_reg(tp->base_addr, GMAC_CONFIG0, config0.bits32,config0_mask.bits32);
}
/*----------------------------------------------------------------------
*	toe_gmac_disable_rx
*----------------------------------------------------------------------*/
#if 0
static void toe_gmac_disable_rx(struct net_device *dev)
{
	GMAC_INFO_T		*tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 1;  /* disable rx */
//    config0.bits.dis_tx = 1;  /* disable tx */
    config0_mask.bits.dis_rx = 1;
//     config0_mask.bits.dis_tx = 1;
    gmac_write_reg(tp->base_addr, GMAC_CONFIG0, config0.bits32,config0_mask.bits32);
}
#endif
/*----------------------------------------------------------------------
*	toe_gmac_enable_rx
*----------------------------------------------------------------------*/
#if 0
static void toe_gmac_enable_rx(struct net_device *dev)
{
	GMAC_INFO_T		*tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 0;  /* enable rx */
//    config0.bits.dis_tx = 0;  /* enable tx */
    config0_mask.bits.dis_rx = 1;
//    config0_mask.bits.dis_tx = 1;
    gmac_write_reg(tp->base_addr, GMAC_CONFIG0, config0.bits32,config0_mask.bits32);
}
#endif
/*----------------------------------------------------------------------
*	toe_gmac_disable_tx_rx
*----------------------------------------------------------------------*/
static void toe_gmac_disable_tx_rx(struct net_device *dev)
{
	GMAC_INFO_T		*tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;

    /* enable TX/RX */
    config0.bits32 = 0;
    config0_mask.bits32 = 0;
    config0.bits.dis_rx = 1;  /* disable rx */
    config0.bits.dis_tx = 1;  /* disable tx */
    config0_mask.bits.dis_rx = 1;
    config0_mask.bits.dis_tx = 1;
    gmac_write_reg(tp->base_addr, GMAC_CONFIG0, config0.bits32,config0_mask.bits32);
}
    
/*----------------------------------------------------------------------
*	toe_gmac_hw_start
*----------------------------------------------------------------------*/
static void toe_gmac_hw_start(struct net_device *dev)
{
	GMAC_INFO_T				*tp = (GMAC_INFO_T *)dev->priv;
	GMAC_DMA_CTRL_T			dma_ctrl, dma_ctrl_mask;
	
					
    /* program dma control register */	
	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.rd_enable = 1;    
	dma_ctrl.bits.td_enable = 1;    
	dma_ctrl.bits.loopback = 0;    
	dma_ctrl.bits.drop_small_ack = 0;    
	dma_ctrl.bits.rd_prot = 0;    
	dma_ctrl.bits.rd_burst_size = 3;    
	dma_ctrl.bits.rd_insert_bytes = RX_INSERT_BYTES;
	dma_ctrl.bits.rd_bus = 3;    
	dma_ctrl.bits.td_prot = 0;    
	dma_ctrl.bits.td_burst_size = 3;    
	dma_ctrl.bits.td_bus = 3;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.rd_enable = 1;    
	dma_ctrl_mask.bits.td_enable = 1;    
	dma_ctrl_mask.bits.loopback = 1;    
	dma_ctrl_mask.bits.drop_small_ack = 1;    
	dma_ctrl_mask.bits.rd_prot = 3;    
	dma_ctrl_mask.bits.rd_burst_size = 3;    
	dma_ctrl_mask.bits.rd_insert_bytes = 3;    
	dma_ctrl_mask.bits.rd_bus = 3;    
	dma_ctrl_mask.bits.td_prot = 0x0f;    
	dma_ctrl_mask.bits.td_burst_size = 3;    
	dma_ctrl_mask.bits.td_bus = 3;    

	gmac_write_reg(tp->dma_base_addr, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
	
    return;	
}	

/*----------------------------------------------------------------------
*	toe_gmac_hw_stop
*----------------------------------------------------------------------*/
static void toe_gmac_hw_stop(struct net_device *dev)
{
	GMAC_INFO_T			*tp = (GMAC_INFO_T *)dev->priv;
	GMAC_DMA_CTRL_T		dma_ctrl, dma_ctrl_mask;

    /* program dma control register */	
	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.rd_enable = 0;    
	dma_ctrl.bits.td_enable = 0;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.rd_enable = 1;    
	dma_ctrl_mask.bits.td_enable = 1;    

	gmac_write_reg(tp->dma_base_addr, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
}

/*----------------------------------------------------------------------
*	toe_gmac_clear_counter
*----------------------------------------------------------------------*/
static int toe_gmac_clear_counter (struct net_device *dev)
{
	GMAC_INFO_T	*tp = (GMAC_INFO_T *)dev->priv;

    /* clear counter */
    gmac_read_reg(tp->base_addr, GMAC_IN_DISCARDS);
    gmac_read_reg(tp->base_addr, GMAC_IN_ERRORS); 
    gmac_read_reg(tp->base_addr, GMAC_IN_MCAST); 
    gmac_read_reg(tp->base_addr, GMAC_IN_BCAST); 
    gmac_read_reg(tp->base_addr, GMAC_IN_MAC1); 
    gmac_read_reg(tp->base_addr, GMAC_IN_MAC2); 
		tp->ifStatics.tx_bytes = 0;
		tp->ifStatics.tx_packets = 0;
		tp->ifStatics.tx_errors = 0;
		tp->ifStatics.rx_bytes = 0;
		tp->ifStatics.rx_packets = 0;
		tp->ifStatics.rx_errors = 0;
		tp->ifStatics.rx_dropped = 0;    
	return (0);    
}
   			

/*----------------------------------------------------------------------
*	toe_gmac_tx_complete
*----------------------------------------------------------------------*/
static inline void toe_gmac_tx_complete(GMAC_INFO_T *tp, unsigned int tx_qid,
   										struct net_device *dev, int interrupt)
{
	volatile GMAC_TXDESC_T	*curr_desc;
	GMAC_TXDESC_0_T			word0;
	GMAC_TXDESC_1_T			word1;
	unsigned int			desc_count;
//	struct net_device_stats *isPtr = (struct net_device_stats *)&tp->ifStatics;
	GMAC_SWTXQ_T			*swtxq;
	DMA_RWPTR_T				rwptr;

	/* get tx H/W completed descriptor virtual address */
	/* check tx status and accumulate tx statistics */
	swtxq = &tp->swtxq[tx_qid];
#ifdef GMAC_DEBUG
	swtxq->intr_cnt++;
#endif	
	for (;;)
	{ 
		rwptr.bits32 = readl(swtxq->rwptr_reg);
		if (rwptr.bits.rptr == swtxq->finished_idx)
			break;
    	curr_desc = (volatile GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
//   		consistent_sync((void *)curr_desc, sizeof(GMAC_TXDESC_T), PCI_DMA_FROMDEVICE);
		word0.bits32 = curr_desc->word0.bits32;
		word1.bits32 = curr_desc->word1.bits32;
			
		if (word0.bits.status_tx_ok)
		{
			tp->ifStatics.tx_bytes += word1.bits.byte_count;
			desc_count = word0.bits.desc_count;
#ifdef GMAC_DEBUG
			if (desc_count==0)
			{
				printk("%s::Desc 0x%x = 0x%x, desc_count=%d\n",__func__, (u32)curr_desc, word0.bits32, desc_count);
				while(1);
			}
#endif			
			while (--desc_count)
			{
				word0.bits.status_tx_ok = 0;
				curr_desc->word0.bits32 = word0.bits32;
				swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, swtxq->total_desc_num);
				curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
				word0.bits32 = curr_desc->word0.bits32;
#ifdef _DUMP_TX_TCP_CONTENT
				if (curr_desc->word0.bits.buffer_size < 16)
				{
					int a;
					char *datap;
					printk("\t Tx Finished Desc 0x%x Len %d Addr 0x%08x: ", (u32)curr_desc, curr_desc->word0.bits.buffer_size, curr_desc->word2.buf_adr);
					datap = (char *)__va(curr_desc->word2.buf_adr);
					for (a=0; a<8 && a<curr_desc->word0.bits.buffer_size; a++, datap++)
					{
						printk("0x%02x ", *datap);
					}
					printk("\n");
				}
#endif
			}
				
			word0.bits.status_tx_ok = 0;
			if (swtxq->tx_skb[swtxq->finished_idx])
			{
//				if (interrupt)
//					dev_kfree_skb_irq(swtxq->tx_skb[swtxq->finished_idx]);
			 if (swtxq->tx_skb[swtxq->finished_idx]->destructor)	//Modify by Wen for VPN
                 dev_kfree_skb_any(swtxq->tx_skb[swtxq->finished_idx]);
             else
                 dev_kfree_skb(swtxq->tx_skb[swtxq->finished_idx]);
//				else
//					dev_kfree_skb(swtxq->tx_skb[swtxq->finished_idx]);
				swtxq->tx_skb[swtxq->finished_idx] = NULL;
			}
			curr_desc->word0.bits32 = word0.bits32;
  			swtxq->curr_finished_desc = (GMAC_TXDESC_T *)curr_desc;
#ifdef GMAC_DEBUG  			
 			swtxq->total_finished++;
#endif 			
  			tp->ifStatics.tx_packets++;
			swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, swtxq->total_desc_num);
		}
		else
		{
			// tp->ifStatics.tx_errors++;
			// printk("%s::Tx Descriptor is !!!\n",__func__);
			// wait ready by breaking
			break;
		}
	}
	
	if (netif_queue_stopped(dev))
	{
		netif_wake_queue(dev);
	}
}

/*----------------------------------------------------------------------
*	gmac_start_xmit
*----------------------------------------------------------------------*/
static int gmac_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	GMAC_INFO_T 			*tp= dev->priv;
//	unsigned int			tx_qid;
    DMA_RWPTR_T				rwptr;
	volatile GMAC_TXDESC_T	*curr_desc;
	int 					snd_pages = skb_shinfo(skb)->nr_frags + 1;  /* get number of descriptor */
	int 					frag_id = 0;
	int 					len, total_len = skb->len;
	struct net_device_stats *isPtr = (struct net_device_stats *)&tp->ifStatics;
	unsigned int			free_desc;
	GMAC_SWTXQ_T			*swtxq;
	register unsigned long	word0, word1, word2, word3;
	unsigned short			wptr, rptr;
	struct net_bridge_fdb_entry *br_fdb;
	struct net_bridge_port *br_port;
#ifdef	L2_jumbo_frame	
	struct tcphdr	*tcp_hdr;
	struct iphdr	*ip_hdr;
	struct ethhdr 	*eth;
#endif	
	
#ifdef	L2_jumbo_frame
	int header_len = skb->len;
	//struct iphdr	*ip_hdr;
    //struct tcphdr	*tcp_hdr;
    int             tcp_hdr_len;
    unsigned char 	*ptr;
    int             data_len,a;
    unsigned int    val;
#endif
#ifdef CONFIG_SL351x_RTLDMZ	
	struct vlan_ethhdr *veth;
	unsigned short veth_TCI;
	unsigned short		tx_vid_skb = 0;
#endif	
#ifdef GMAC_LEN_1_2_ISSUE
	int						total_pages;
	total_pages = snd_pages;
#endif
	
	if (skb == NULL) {
		printk("%s:: skb == NULL\n",__func__);
		return 0;
	}
		

#ifdef CONFIG_SL351x_RTLDMZ	
	veth = (struct vlan_ethhdr *)(skb->data);
	if (tp->port_id == 0)
	{
		if (veth->h_vlan_proto != __constant_htons(ETH_P_8021Q)) 
		{
			veth_TCI = VLAN_DEV_INFO(dev)->vlan_id;
			tx_vid_skb = ((skb->cb[62] & 0x00FF) | ((skb->cb[63]& 0x0F00) >> 8));
#ifdef DUAL_BAND_VIRTUAL_INTERFACE					
			if (dev == GMACVIF0)	//to lan interface
			{
				dev = GMACVIF0;
				veth_TCI = LAN_VLAN_ID;
				tp = GMACVIF0->priv;
			}
			if (dev == GMACVIF1)	//to dmz interface
			{
				dev = GMACVIF1;
				veth_TCI = DMZ_VLAN_ID;
				tp = GMACVIF1->priv;
			}
#else 				
				veth_TCI = LAN_VLAN_ID; 				
#endif	 		
			skb = __vlan_put_tag(skb, veth_TCI);
			if (!skb) {
				isPtr->tx_dropped++;
				return 0;
			}
				total_len = skb->len;
		}
		
	}
#endif	//CONFIG_SL351x_RTLDMZ	

	isPtr = (struct net_device_stats *)&tp->ifStatics;
#if 1
	if (skb->len >= 0x10000)
	{
//		spin_unlock(&tp->tx_mutex);
		isPtr->tx_dropped++;
		printk("%s::[GMAC %d] skb->len %d >= 64K\n", __func__, tp->port_id, skb->len);
		netif_stop_queue(dev);
		return 1;
    }
#endif
	//igmp snoop test,have joint send out to LAN ,that must be set to port 5 cpu port.

#ifdef ALPHA_RTL8366SR_MODULE				
				if(igmp_snoop_input 
					   && tp->port_id == GMAC_PORT0 /*LAN */
					   && (skb->data[0]&0x01) == 0x01 
					   && ((struct ethhdr*)skb->data)->h_proto == htons(0x0800))
				{
					igmp_snoop_input(skb,5);
				}
#endif

	
#ifdef ALPHA_IGMP_SNOOP
#if 1 //hendry for enable/disable igmp
	if(g_igmp_snoop_enable)
#endif
	{
		#if 1//joel	
		if(tp->port_id == GMAC_PORT0 /*LAN */
			   && skb->data[0]&0x01== 0x01 
			   && ((struct ethhdr*)skb->data)->h_proto == htons(0x0800))
		{
			igmp_snoop(skb,5);
		}
		#endif	
	}
#endif
	

#ifdef GMAC_USE_TXQ0
	#define tx_qid 	0
#endif
	
	swtxq = &tp->swtxq[tx_qid];
	
//	spin_lock(&tp->tx_mutex);	
    rwptr.bits32 = readl(swtxq->rwptr_reg);
	wptr = rwptr.bits.wptr;
	rptr = rwptr.bits.rptr;
	// check finished desc or empty BD 
	// cannot check by read ptr of RW PTR register, 
	// because the HW complete to send but the SW may NOT handle it
#ifndef	GMAX_TX_INTR_DISABLED
	if (wptr >= swtxq->finished_idx)
		free_desc = swtxq->total_desc_num - wptr - 1 + swtxq->finished_idx;
	else 
		free_desc = swtxq->finished_idx - wptr - 1;
	
	if (free_desc < snd_pages)
	{
//		spin_unlock(&tp->tx_mutex);
		isPtr->tx_dropped++;
//		printk("GMAC %d No available descriptor!\n", tp->port_id);
		netif_stop_queue(dev);
		return 1;
    }
#else
	toe_gmac_tx_complete(tp, tx_qid, dev, 0);
	if (wptr >= swtxq->finished_idx)
		free_desc = swtxq->total_desc_num - wptr - 1 + swtxq->finished_idx;
	else 
		free_desc = swtxq->finished_idx - wptr - 1;
	if (free_desc < snd_pages)
	{
//		spin_unlock(&tp->tx_mutex);
		isPtr->tx_dropped++;
//		printk("GMAC %d No available descriptor!\n", tp->port_id);
		netif_stop_queue(dev);
		return 1;
    }

#if 0		
	printk("1: free_desc=%d, wptr=%d, finished_idx=%d\n", free_desc, wptr, swtxq->finished_idx);
	if ((free_desc < (snd_pages << 2)) || 
	    (free_desc < (swtxq->total_desc_num >> 2)))
	{
		printk("2: free_desc = %d\n", free_desc);
		toe_gmac_tx_complete(tp, tx_qid, dev, 0);
		rwptr.bits32 = readl(swtxq->rwptr_reg);
		wptr = rwptr.bits.wptr;
		if (wptr>= swtxq->finished_idx)
			free_desc = swtxq->total_desc_num - wptr -1 + swtxq->finished_idx;
		else
			free_desc = swtxq->finished_idx - wptr - 1;
	}
#endif	
#endif

#ifdef	L2_jumbo_frame
		if ( skb->nh.iph && ((skb->nh.iph->protocol & 0x00ff)  == IPPROTO_TCP))
		{
				ip_hdr = (struct iphdr*)(skb->nh.iph);
				tcp_hdr = (struct tcphdr*)(skb->h.th);
				tcp_hdr_len = TCPHDRLEN(tcp_hdr) * 4;
				tcp_hdr_len = TCPHDRLEN(tcp_hdr) * 4;
				
				if ((skb->h.th->syn) && (tcp_hdr_len > 20))
				{
					ptr = (unsigned char *)(tcp_hdr+1); 
					if ((ptr[0] == 0x02) && (ptr[1] == 0x04) && (ptr[2] == 0x07) && (ptr[3] == 0xba)) // 0x07 aa=2016-54=1962  ,0x07ba=2032-54=1978
					{
						ptr[2]=0x20;	//23
						ptr[3]=0x00;   	//00
					}
				}
		}
//		if ((ip_hdr->protocol & 0x00ff) != IPPROTO_TCP)
//		if ((tcp_hdr_len > 20) && (skb->h.th->syn))
#endif

    while (snd_pages)
    {
    	char *pkt_datap;
    	curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + wptr;
//		consistent_sync((void *)curr_desc, sizeof(GMAC_TXDESC_T), PCI_DMA_FROMDEVICE);
#if 0
//#if (GMAC_DEBUG==1)
    	// if curr_desc->word2.buf_adr !=0 means that the ISR does NOT handle it
    	// if (curr_desc->word2.buf_adr)
    	if (swtxq->tx_skb[wptr])
    	{
    		printk("Error! Stop due to TX descriptor's buffer is not freed!\n");
    		while(1);
    		dev_kfree_skb(swtxq->tx_skb[wptr]);
    		swtxq->tx_skb[wptr] = NULL;
		}
#endif

		if (frag_id == 0) 
		{
#if 0		
			int i;
			pkt_datap = skb->data;
			len = total_len;
			for (i=0; i<skb_shinfo(skb)->nr_frags; i++)
			{
				skb_frag_t* frag = &skb_shinfo(skb)->frags[i];
				len -= frag->size;
			}
#else
			pkt_datap = skb->data;
			len = total_len - skb->data_len;
#endif			
		}
		else 
		{
			skb_frag_t* frag = &skb_shinfo(skb)->frags[frag_id-1];
			pkt_datap = page_address(frag->page) + frag->page_offset;
			len = frag->size;
			if (len > total_len)
			{
				printk("Fatal Error! Send Frag size %d > Total Size %d!!\n", 
					len, total_len);
			}
		}
			
		/* set TX descriptor */
		/* copy packet to descriptor buffer address */
		// curr_desc->word0.bits32 = len;    /* total frame byte count */
		word0 = len;
#ifdef	L2_jumbo_frame
		word3 = (dev->mtu+14) | EOFIE_BIT;  //2016 ,2032
#else
#ifdef CONFIG_SL2312_MPAGE
		/* if this skb is with fast_sock on; i.e, FTP data, 
		   use path MTU instead, jeanson */
		if (skb->sk)
		{
			/* if the destination is known; normally yes */
			if (skb->sk->fast_sock && skb->dst)
				word3 = (dst_mtu(skb->dst)+14) | EOFIE_BIT;
			else
				word3 = (dev->mtu+14) | EOFIE_BIT;
		}else
		{
			word3 = (dev->mtu+14) | EOFIE_BIT;
		}
#else
		word3 = (dev->mtu+14) | EOFIE_BIT;
#endif 	/* CONFIG_SL2312_MPAGE */
#endif

#ifdef DO_HW_CHKSUM	
#ifdef	L2_jumbo_frame	
		if (total_len >= (dev->mtu+14) && (skb->nh.iph->protocol == 0x011) && skb->nh.iph && (skb->nh.iph->frag_off & __constant_htons(0x3fff)))
#else
#ifndef CONFIG_SL351x_RTLDMZ	
		if (total_len <= 1514 && skb->nh.iph && (skb->nh.iph->frag_off & __constant_htons(0x3fff)))
#else
		if ((tp->port_id == 0) && (total_len >= 1518))		
			word1  = total_len | TSS_IP_CHKSUM_BIT  | TSS_IPV6_ENABLE_BIT | TSS_MTU_ENABLE_BIT;
		
		else if ((tp->port_id == 1) && (total_len >= 1514))	
#endif					
#endif	//L2_jumbo_frame
		
			word1  = total_len | TSS_IP_CHKSUM_BIT  | TSS_IPV6_ENABLE_BIT |	TSS_MTU_ENABLE_BIT;
		
#ifdef CONFIG_SL351X_IPSEC
		else if ((__u8)(skb->cb[28]) == 1)	// special packet handling for HW IPsec-VPN packets
			word1 = total_len |
				TSS_IP_CHKSUM_BIT |
				TSS_IPV6_ENABLE_BIT |
				TSS_MTU_ENABLE_BIT;
#endif 
		else
		{
#ifdef CONFIG_SL351x_RTLDMZ	
			    if (total_len <=64 && (tp->port_id == 0)) {
			    	if(memcmp(&skb->data[6],&dev->dev_addr[0],ETH_ALEN)!=0)
			    	{
						if( ((br_port = rcu_dereference(skb->dev->br_port)) != NULL) &&
							likely( (br_fdb = br_fdb_get(br_port->br, &skb->data[6])) != NULL ) &&
							likely( !(br_fdb->is_local) ) )
							
							word1 = total_len | TSS_MTU_ENABLE_BIT;
						
						else
							word1 = total_len | 
								TSS_UDP_CHKSUM_BIT |
								TSS_TCP_CHKSUM_BIT |
								TSS_IP_CHKSUM_BIT  |
								TSS_IPV6_ENABLE_BIT |
								TSS_MTU_ENABLE_BIT;
					}
					else
						word1 = total_len | 
							TSS_UDP_CHKSUM_BIT |
							TSS_TCP_CHKSUM_BIT |
							TSS_IP_CHKSUM_BIT | 
							TSS_IPV6_ENABLE_BIT |
							TSS_MTU_ENABLE_BIT;
									
				if(frag_id==0)
					word1 |= TSS_IP_FIXED_LEN_BIT;
			}
			else if (total_len <=60 && (tp->port_id == 1)) {
#else			
			if (total_len <= 60){
#endif	//CONFIG_SL351x_RTLDMZ				
								
				if(memcmp(&skb->data[6],&dev->dev_addr[0],ETH_ALEN)!=0)
				{
						if( ((br_port = rcu_dereference(skb->dev->br_port)) != NULL) &&
							likely( (br_fdb = br_fdb_get(br_port->br, &skb->data[6])) != NULL ) &&
							likely( !(br_fdb->is_local) ) )
							
							word1 = total_len | TSS_MTU_ENABLE_BIT;
					
						else
							word1 = total_len | 
								TSS_UDP_CHKSUM_BIT |
								TSS_TCP_CHKSUM_BIT |
								TSS_IP_CHKSUM_BIT  |
								TSS_IPV6_ENABLE_BIT |
								TSS_MTU_ENABLE_BIT;
				}
				else
					word1 = total_len | 
						TSS_UDP_CHKSUM_BIT |
						TSS_TCP_CHKSUM_BIT |
						TSS_IP_CHKSUM_BIT | 
						TSS_IPV6_ENABLE_BIT |
						TSS_MTU_ENABLE_BIT;
				
				// Tell GMAC not to modify ip_hdr->tot_len 
				if(frag_id==0)
					word1 |= TSS_IP_FIXED_LEN_BIT;
			}
			else
				word1 = total_len | 
					TSS_UDP_CHKSUM_BIT |
					TSS_TCP_CHKSUM_BIT |
					TSS_IP_CHKSUM_BIT  |
					TSS_IPV6_ENABLE_BIT |
					TSS_MTU_ENABLE_BIT;
		}
#else	
		word1 = total_len | TSS_MTU_ENABLE_BIT;
		
#endif	//DO_HW_CHKSUM
		word2 = (unsigned long)__pa(pkt_datap);
	
		if (frag_id == 0)
		{
			word3 |= SOF_BIT;	// SOF
		}
			
		if (snd_pages == 1)
		{
			word3 |= EOF_BIT;	// EOF
			swtxq->tx_skb[wptr] = skb;
#ifdef CONFIG_SL351x_NAT
#ifdef SKIP_NAT_CFG	
				nat_cfg.enabled = 1;	
#endif
			if (nat_cfg.enabled && sl351x_nat_output(skb, tp->port_id))
				word1 |= TSS_IP_FIXED_LEN_BIT;
#endif
#ifdef CONFIG_SL351X_BR
				word1 |= TSS_IP_FIXED_LEN_BIT;
#endif
		}
		else
			swtxq->tx_skb[wptr] = NULL;
		// word1 |= TSS_IP_FIXED_LEN_BIT;
#if 1
#ifdef CONFIG_SL351x_RXTOE
		// check if this frame has the mission to enable toe hash entry..
		// if rx_max_pktsize ==0, do not enable RXTOE
		if (TCP_SKB_CB(skb)->connection && storlink_ctl.rx_max_pktsize) {
			set_toeq_hdr(TCP_SKB_CB(skb)->connection, &toe_private_data, dev);
		}
#endif
#endif

#ifdef GMAC_DEBUG_U
		if (gmac_dump_txpkt)
		{
			printk("******** GMAC %d Tx (Size = %d): skb->data =0x%p *********\n",tp->port_id, total_len, skb->data);
			 dm_byte((u32)skb->head, (total_len > 256) ? 256 : total_len);
			 printk("\n");
		}
#endif	

#ifdef _DUMP_TX_TCP_CONTENT
		if (len < 16 && frag_id && skb->h.th && (skb->h.th->source == __constant_htons(445) || skb->h.th->source == __constant_htons(139)))
		{
			int a;
			char *datap;
			printk("Tx Desc 0x%x Frag %d Len %d [IP-ID 0x%x] 0x%08x: ", (u32)curr_desc, frag_id, len, htons(skb->nh.iph->id), (u32)pkt_datap);
			datap = (char *)pkt_datap;
			for (a=0; a<8 && a<len; a++, datap++)
			{
				printk("0x%02x ", *datap);
			}
			printk("\n");
		}
#endif

#ifdef GMAC_LEN_1_2_ISSUE
		if ((total_pages!=snd_pages) && (len == 1 || len == 2 ) && ((u32)pkt_datap & 0x03))
		{
			memcpy((void *)&_debug_prefetch_buf[_debug_prefetch_cnt][0], pkt_datap, len);
			pkt_datap = (char *)&_debug_prefetch_buf[_debug_prefetch_cnt][0];
			word2 = (unsigned long)__pa(pkt_datap);
			_debug_prefetch_cnt++;
			if (_debug_prefetch_cnt >= _DEBUG_PREFETCH_NUM)
				_debug_prefetch_cnt = 0;
		}
#endif
		
		consistent_sync((void *)pkt_datap, len, PCI_DMA_TODEVICE);
		wmb();
		curr_desc->word0.bits32 = word0;
		curr_desc->word1.bits32 = word1;
		curr_desc->word2.bits32 = word2;
		curr_desc->word3.bits32 = word3;
		swtxq->curr_tx_desc = (GMAC_TXDESC_T *)curr_desc;
		consistent_sync((void *)curr_desc, sizeof(GMAC_TXDESC_T), PCI_DMA_TODEVICE);
#ifdef _DUMP_TX_TCP_CONTENT
		if (len < 16 && frag_id && skb->h.th && (skb->h.th->source == __constant_htons(445) || skb->h.th->source == __constant_htons(139)))
		{
			int a;
			char *datap;
			printk("\t 0x%08x: ", (u32)pkt_datap);
			datap = (char *)pkt_datap;
			for (a=0; a<8 && a<len; a++, datap++)
			{
				printk("0x%02x ", *datap);
			}
			printk("\n");
		}
#endif
		free_desc--;
		wmb();
		rwptr.bits32 = readl(swtxq->rwptr_reg);
		wptr = RWPTR_ADVANCE_ONE(wptr, swtxq->total_desc_num);
		frag_id++;
		snd_pages--;
	}	

#ifdef GMAC_DEBUG	
    swtxq->total_sent++;
#endif
	rwptr.bits32 = readl(swtxq->rwptr_reg);
	SET_WPTR(swtxq->rwptr_reg, wptr);
	dev->trans_start = jiffies;

//#ifdef	GMAX_TX_INTR_DISABLED
//		toe_gmac_tx_complete(tp, tx_qid, dev, 0);
//#endif	
	return (0); 
}

/*----------------------------------------------------------------------
* gmac_set_mac_address
*----------------------------------------------------------------------*/
static int gmac_set_mac_address(struct net_device *dev, void *addr)
{
	GMAC_INFO_T		*tp= dev->priv;
	struct sockaddr *sock;
	unsigned int    reg_val;
    unsigned int    i;
#ifdef DUAL_BAND_VIRTUAL_INTERFACE    
	struct sockaddr *sock_2;
#endif

	sock = (struct sockaddr *) addr;
	
#ifdef DUAL_BAND_VIRTUAL_INTERFACE	
	if(tp->port_id==0)
	{
		if(strstr(dev->name,"eth0"))
		{
			for (i = 0; i < 6; i++)
			{
				dev->dev_addr[i] = sock->sa_data[i];
			}

    		reg_val = dev->dev_addr[0] + (dev->dev_addr[1]<<8) + (dev->dev_addr[2]<<16) + (dev->dev_addr[3]<<24);
    		gmac_write_reg(tp->base_addr,GMAC_STA_ADD0,reg_val,0xffffffff);
    		reg_val = dev->dev_addr[4] + (dev->dev_addr[5]<<8);
    		gmac_write_reg(tp->base_addr,GMAC_STA_ADD1,reg_val,0x0000ffff);
			memcpy(&eth_mac[tp->port_id][0],&dev->dev_addr[0],6);
		}
		if(strstr(dev->name,"eth2"))
		{
			sock_2 = (struct sockaddr *) addr;
			for (i = 0; i < 6; i++)
			{
				dev->dev_addr[i] = sock_2->sa_data[i];
			}

    		reg_val = (dev->dev_addr[0]<<16) + (dev->dev_addr[1]<<24);
    		gmac_write_reg(tp->base_addr,GMAC_STA_ADD1,reg_val,0xffff0000);
    		reg_val = dev->dev_addr[2] + (dev->dev_addr[3]<<8) + (dev->dev_addr[4]<<16) + (dev->dev_addr[5]<<24);
    		gmac_write_reg(tp->base_addr,GMAC_STA_ADD2,reg_val,0xffffffff);
			memcpy(&eth_mac[tp->port_id][0],&dev->dev_addr[0],6);
		}			
	}
	else
	{
		for (i = 0; i < 6; i++)
		{
			dev->dev_addr[i] = sock->sa_data[i];
		}

    	reg_val = dev->dev_addr[0] + (dev->dev_addr[1]<<8) + (dev->dev_addr[2]<<16) + (dev->dev_addr[3]<<24);
    	gmac_write_reg(tp->base_addr,GMAC_STA_ADD0,reg_val,0xffffffff);
    	reg_val = dev->dev_addr[4] + (dev->dev_addr[5]<<8);
    	gmac_write_reg(tp->base_addr,GMAC_STA_ADD1,reg_val,0x0000ffff);
		memcpy(&eth_mac[tp->port_id][0],&dev->dev_addr[0],6);
	}	
#else 
	for (i = 0; i < 6; i++)
	{
		dev->dev_addr[i] = sock->sa_data[i];
	}
    reg_val = dev->dev_addr[0] + (dev->dev_addr[1]<<8) + (dev->dev_addr[2]<<16) + (dev->dev_addr[3]<<24);
    gmac_write_reg(tp->base_addr,GMAC_STA_ADD0,reg_val,0xffffffff);
    reg_val = dev->dev_addr[4] + (dev->dev_addr[5]<<8);
    gmac_write_reg(tp->base_addr,GMAC_STA_ADD1,reg_val,0x0000ffff);
	memcpy(&eth_mac[tp->port_id][0],&dev->dev_addr[0],6);	
#endif	//DUAL_BAND_VIRTUAL_INTERFACE
        	
    printk("Storlink %s address = ",dev->name);
    printk("%02X",dev->dev_addr[0]); 
    printk("%02X",dev->dev_addr[1]);
    printk("%02X",dev->dev_addr[2]);
    printk("%02X",dev->dev_addr[3]);
    printk("%02X",dev->dev_addr[4]);
    printk("%02X\n",dev->dev_addr[5]);

    return (0);
}

/*----------------------------------------------------------------------
* gmac_get_mac_address
*	get mac address from FLASH
*----------------------------------------------------------------------*/
static void gmac_get_mac_address(void)
{
#ifdef CONFIG_MTD	
	extern int get_vlaninfo(vlaninfo* vlan);
#ifdef DUAL_BAND_VIRTUAL_INTERFACE  	
    static vlaninfo    vlan[3];
#else
	static vlaninfo    vlan[2];    
#endif	

    if (get_vlaninfo(&vlan[0]))
    {
        memcpy((void *)&eth_mac[0][0],vlan[0].mac,6);
        memcpy((void *)&eth_mac[1][0],vlan[1].mac,6);
#ifdef DUAL_BAND_VIRTUAL_INTERFACE   
        memcpy((void *)&eth_mac[2][0],vlan[2].mac,6);
#endif        
    }
#else
    unsigned int reg_val;
    
    reg_val = readl(IO_ADDRESS(SL2312_SECURITY_BASE)+0xac);
    eth_mac[0][4] = (reg_val & 0xff00) >> 8;
    eth_mac[0][5] = reg_val & 0x00ff;
    reg_val = readl(IO_ADDRESS(SL2312_SECURITY_BASE)+0xac);
    eth_mac[1][4] = (reg_val & 0xff00) >> 8;
    eth_mac[1][5] = reg_val & 0x00ff;        
#endif
    
    return;
}

/*----------------------------------------------------------------------
* mac_stop_txdma
*----------------------------------------------------------------------*/
void mac_stop_txdma(struct net_device *dev)
{
	GMAC_INFO_T				*tp = (GMAC_INFO_T *)dev->priv;
	GMAC_DMA_CTRL_T			dma_ctrl, dma_ctrl_mask;
	GMAC_TXDMA_FIRST_DESC_T	txdma_busy;

	// wait idle
	do
	{
		txdma_busy.bits32 = gmac_read_reg(tp->dma_base_addr, GMAC_DMA_TX_FIRST_DESC_REG);
	} while (txdma_busy.bits.td_busy);
	
    /* program dma control register */	
	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.rd_enable = 0;    
	dma_ctrl.bits.td_enable = 0;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.rd_enable = 1;    
	dma_ctrl_mask.bits.td_enable = 1;    

	gmac_write_reg(tp->dma_base_addr, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
}

/*----------------------------------------------------------------------
* mac_start_txdma
*----------------------------------------------------------------------*/
void mac_start_txdma(struct net_device *dev)
{
	GMAC_INFO_T			*tp = (GMAC_INFO_T *)dev->priv;
	GMAC_DMA_CTRL_T		dma_ctrl, dma_ctrl_mask;
	
    /* program dma control register */	
	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.rd_enable = 1;    
	dma_ctrl.bits.td_enable = 1;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.rd_enable = 1;    
	dma_ctrl_mask.bits.td_enable = 1;    

	gmac_write_reg(tp->dma_base_addr, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
}


/*----------------------------------------------------------------------
* gmac_get_stats 
*----------------------------------------------------------------------*/

struct net_device_stats * gmac_get_stats(struct net_device *dev)
{
    GMAC_INFO_T *tp = (GMAC_INFO_T *)dev->priv;
    // unsigned int        flags;
    unsigned int        pkt_drop;
    unsigned int        pkt_error;

    if (netif_running(dev))
    {
        /* read H/W counter */
        // spin_lock_irqsave(&tp->lock,flags);
        pkt_drop = gmac_read_reg(tp->base_addr,GMAC_IN_DISCARDS);
        pkt_error = gmac_read_reg(tp->base_addr,GMAC_IN_ERRORS);
        tp->ifStatics.rx_dropped = tp->ifStatics.rx_dropped + pkt_drop;
        tp->ifStatics.rx_errors = tp->ifStatics.rx_errors + pkt_error;
        // spin_unlock_irqrestore(&tp->lock,flags);
    }
    return &tp->ifStatics;
}



/*----------------------------------------------------------------------
* mac_get_sw_tx_weight
*----------------------------------------------------------------------*/
void mac_get_sw_tx_weight(struct net_device *dev, char *weight)
{
	GMAC_TX_WCR1_T	sw_weigh;
    GMAC_INFO_T		*tp = (GMAC_INFO_T *)dev->priv;
	
	sw_weigh.bits32 = gmac_read_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_1_REG);
   	
	weight[0] = sw_weigh.bits.sw_tq0;
   	weight[1] = sw_weigh.bits.sw_tq1;
   	weight[2] = sw_weigh.bits.sw_tq2;
   	weight[3] = sw_weigh.bits.sw_tq3;
   	weight[4] = sw_weigh.bits.sw_tq4;
   	weight[5] = sw_weigh.bits.sw_tq5;
}

/*----------------------------------------------------------------------
* mac_set_sw_tx_weight
*----------------------------------------------------------------------*/
void mac_set_sw_tx_weight(struct net_device *dev, char *weight)
{
	GMAC_TX_WCR1_T	sw_weigh;
    GMAC_INFO_T		*tp = (GMAC_INFO_T *)dev->priv;
	
	sw_weigh.bits32 = 0;
	sw_weigh.bits.sw_tq0 = weight[0];
   	sw_weigh.bits.sw_tq1 = weight[1];
   	sw_weigh.bits.sw_tq2 = weight[2];
   	sw_weigh.bits.sw_tq3 = weight[3];
   	sw_weigh.bits.sw_tq4 = weight[4];
   	sw_weigh.bits.sw_tq5 = weight[5];
   	
	gmac_write_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_1_REG, sw_weigh.bits32, 0xffffffff);
}

/*----------------------------------------------------------------------
* mac_get_hw_tx_weight
*----------------------------------------------------------------------*/
void mac_get_hw_tx_weight(struct net_device *dev, char *weight)
{
	GMAC_TX_WCR0_T	hw_weigh;
    GMAC_INFO_T		*tp = (GMAC_INFO_T *)dev->priv;
	
	hw_weigh.bits32 = gmac_read_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_0_REG);
   	
	weight[0] = hw_weigh.bits.hw_tq0;
   	weight[1] = hw_weigh.bits.hw_tq1;
   	weight[2] = hw_weigh.bits.hw_tq2;
   	weight[3] = hw_weigh.bits.hw_tq3;
}

/*----------------------------------------------------------------------
* mac_set_hw_tx_weight
*----------------------------------------------------------------------*/
void mac_set_hw_tx_weight(struct net_device *dev, char *weight)
{
	GMAC_TX_WCR0_T	hw_weigh;
    GMAC_INFO_T		*tp = (GMAC_INFO_T *)dev->priv;
	
	hw_weigh.bits32 = 0;
	hw_weigh.bits.hw_tq0 = weight[0];
   	hw_weigh.bits.hw_tq1 = weight[1];
   	hw_weigh.bits.hw_tq2 = weight[2];
   	hw_weigh.bits.hw_tq3 = weight[3];
   	
	gmac_write_reg(tp->dma_base_addr, GMAC_TX_WEIGHTING_CTRL_0_REG, hw_weigh.bits32, 0xffffffff);
}

/*----------------------------------------------------------------------
* mac_start_tx_dma
*----------------------------------------------------------------------*/
int mac_start_tx_dma(int mac)
{
	GMAC_DMA_CTRL_T dma_ctrl, dma_ctrl_mask;

	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.td_enable = 1;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.td_enable = 1;    

	if (mac == 0)
    	gmac_write_reg(TOE_GMAC0_DMA_BASE, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
	else
    	gmac_write_reg(TOE_GMAC1_DMA_BASE, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
	return	1;
}

/*----------------------------------------------------------------------
* mac_stop_tx_dma
*----------------------------------------------------------------------*/
int mac_stop_tx_dma(int mac)
{
	GMAC_DMA_CTRL_T dma_ctrl, dma_ctrl_mask;

	dma_ctrl.bits32 = 0;
	dma_ctrl.bits.td_enable = 0;    
	
	dma_ctrl_mask.bits32 = 0;
	dma_ctrl_mask.bits.td_enable = 1;    

	if (mac == 0)
    	gmac_write_reg(TOE_GMAC0_DMA_BASE, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
	else
    	gmac_write_reg(TOE_GMAC1_DMA_BASE, GMAC_DMA_CTRL_REG, dma_ctrl.bits32, dma_ctrl_mask.bits32);
	return	1;
}

/*----------------------------------------------------------------------
* mac_read_reg(int mac, unsigned int offset)
*----------------------------------------------------------------------*/
unsigned int mac_read_reg(int mac, unsigned int offset)
{
	switch (mac)
	{
		case 0:
			return gmac_read_reg(TOE_GMAC0_BASE, offset);
		case 1:
			return gmac_read_reg(TOE_GMAC1_BASE, offset);
		default:
			return 0;
	}
}

/*----------------------------------------------------------------------
* mac_write_reg
*----------------------------------------------------------------------*/
void mac_write_reg(int mac, unsigned int offset, unsigned data)
{
	switch (mac)
	{
		case 0:
			gmac_write_reg(GMAC0_BASE, offset, data, 0xffffffff);
			break;
		case 1:
			gmac_write_reg(GMAC1_BASE, offset, data, 0xffffffff);
			break;
	}
}

/*----------------------------------------------------------------------
* mac_read_dma_reg(int mac, unsigned int offset)
*----------------------------------------------------------------------*/
u32 mac_read_dma_reg(int mac, unsigned int offset)
{
	switch (mac)
	{
		case 0:
			return gmac_read_reg(TOE_GMAC0_DMA_BASE, offset);
		case 1:
			return gmac_read_reg(TOE_GMAC1_DMA_BASE, offset);
		default:
			return 0;
	}
}

/*----------------------------------------------------------------------
* mac_write_dma_reg
*----------------------------------------------------------------------*/
void mac_write_dma_reg(int mac, unsigned int offset, u32 data)
{
	switch (mac)
	{
		case 0:
			gmac_write_reg(TOE_GMAC0_DMA_BASE, offset, data, 0xffffffff);
			break;
		case 1:
			gmac_write_reg(TOE_GMAC1_DMA_BASE, offset, data, 0xffffffff);
			break;
	}
}

/*----------------------------------------------------------------------
* ether_crc
*----------------------------------------------------------------------*/
static unsigned const ethernet_polynomial = 0x04c11db7U;
static unsigned int ether_crc (int length, unsigned char *data)
{
	int crc = -1;
	unsigned int i;
	unsigned int crc_val=0;

	while (--length >= 0) {
		unsigned char current_octet = *data++;
		int bit;
		for (bit = 0; bit < 8; bit++, current_octet >>= 1)
			crc = (crc << 1) ^ ((crc < 0) ^ (current_octet & 1) ?
			     ethernet_polynomial : 0);
	}
	crc = ~crc;
	for (i=0;i<32;i++)
	{
		crc_val = crc_val + (((crc << i) & 0x80000000) >> (31-i));
	}
	return crc_val;
}



/*----------------------------------------------------------------------
* mac_set_rx_mode
*----------------------------------------------------------------------*/
void mac_set_rx_mode(int pid, unsigned int data)
{
	unsigned int	base;	
	
	base = (pid == 0) ? GMAC0_BASE : GMAC1_BASE;

    gmac_write_reg(base, GMAC_RX_FLTR, data, 0x0000001f);
    return;
}

/*----------------------------------------------------------------------
* gmac_open
*----------------------------------------------------------------------*/
#ifdef RTL8366_LINK_INTERRUPT
#define GPIO_LEVEL_TRIG		1
#define GPIO_EDGE_TRIG		0
#define GPIO_LOW_ACTIVE		1
#define GPIO_HIGH_ACTIVE	0
#define GPIO_FALL_ACTIVE	1
#define GPIO_RISE_ACTIVE	0
#define GPIO_BOTH_EDGE		1
#define GPIO_SINGLE_EDGE	0 
#define PORT4_MASK   0x410
static int portMasktoPort(unsigned int portmask)
{
	int i;
	for(i=0;i<4;i++)
	{
		if(portmask&(1<<i))
			return i;	
	}
	return -1;
}
static void linkInterruptHandle(void) 
{
	unsigned int portmask=0,status;
	int port;
	GMAC_INFO_T  *tp;
	//check witch port
	rtl8366sr_printk("linkInterruptHandle\n");
	if(rtl8366rb_getPortInterruptStatus(&portmask)!=0)
		return;
	if(!(portmask & 0xf))/*port 0~3 LAN port*/
		return;
	port = portMasktoPort(portmask);
	rtl8366sr_printk("switch event %x port %d\n",portmask,port);
	if(port >=0 && rtl8366rb_getPHYLinkStatus(port,&status)==0)
	{
		printk("port %d link %s\n",port,status?"up":"down");
		//add by sam_pan
		if(port,status)
			key_handler(0x30); 
	}			
}
#endif
static int gmac_open (struct net_device *dev)
{
	GMAC_INFO_T  *tp = (GMAC_INFO_T *)dev->priv;
	int    					retval;
	TOE_INFO_T				*toe;
	toe = (TOE_INFO_T *)&toe_private_data;
#ifdef DUAL_BAND_VIRTUAL_INTERFACE  
	struct sockaddr sock_2;
#endif		
	
#ifdef DUAL_BAND_VIRTUAL_INTERFACE	
	if(tp->port_id == 0)
	{		
		if (dev == GMACVIF0)
		{
			if (gmac0_running == 1)
				return (0);
			retval = request_irq (dev->irq, toe_gmac_interrupt, SA_INTERRUPT, dev->name, dev);
			gmac0_running = 1;
			if (retval)
				return retval;
		}	
		if (dev == GMACVIF1)
		{
			memcpy(&sock_2.sa_data[0],&eth_mac[2][0],6);
			gmac_set_mac_address(dev,(void *)&sock_2);   
			gmac0_running = 0;
			return (0);
		}
	}
	else
	{
		retval = request_irq (dev->irq, toe_gmac_interrupt, SA_INTERRUPT, dev->name, dev);
		if (retval)
			return retval;
	}	
#else
	
    /* hook ISR */
	retval = request_irq (dev->irq, toe_gmac_interrupt, SA_INTERRUPT, dev->name, dev);
	if (retval)
		return retval;
#endif
	
	netif_start_queue (dev);
	
	toe_init_gmac(dev);
#ifdef RTL8366_LINK_INTERRUPT
	/*0x3f is all event,seem not work always send event when any port link*/
	if(tp->port_id == GMAC_PORT0)
	{
		unsigned int portmask;

		//request_gpio_irq(RTL8366_LINK_INTERRUPT_PIN,linkInterruptHandle,GPIO_EDGE_TRIG,GPIO_FALL_ACTIVE,GPIO_SINGLE_EDGE); 
		request_gpio_irq(RTL8366_LINK_INTERRUPT_PIN,linkInterruptHandle,GPIO_LEVEL_TRIG,GPIO_LOW_ACTIVE,GPIO_SINGLE_EDGE); 
		//must read status first..to clear....
		if(rtl8366rb_getPortInterruptStatus(&portmask)!=0)
			rtl8366sr_printk("%s return fail\n",__FUNCTION__);
		if(rtl8366rb_setPortInterruptControl(1/*low triger*/,0x0f)==0)
			rtl8366sr_printk("rtl8366s_setInterruptControl success\n");
		else
			rtl8366sr_printk("rtl8366s_setInterruptControl fail\n");
		//maybe the ether is link up,try syn one time...
		//gmac_get_rtl8366sr_status(dev);
		
	}
	//now the port do not have interrupt using polling
#endif	
#ifndef ALPHA_RTL8366SR_MODULE	 //sync thread let switch module do it
	if(tp->port_id == GMAC_PORT1 && tp->auto_nego_cfg)//if auto we need monitor PHY
	{
//		init_waitqueue_head (&tp->thr_wait);
//		init_completion(&tp->thr_exited);

		tp->time_to_die = 0;
		tp->thr_pid = kernel_thread (gmac_phy_thread, dev, CLONE_FS | CLONE_FILES);
		if (tp->thr_pid < 0)
		{
			printk (KERN_WARNING "%s: unable to start kernel thread\n",dev->name);
		}
		msleep(200);
	}
#endif
	tp->operation = 1;
//   	netif_start_queue (dev);

	return (0);
}

/*----------------------------------------------------------------------
* gmac_close
*----------------------------------------------------------------------*/
static int gmac_close(struct net_device *dev)
{
    TOE_INFO_T			*toe;
	GMAC_INFO_T 		*tp = dev->priv;
	unsigned int		ret;

#ifdef DUAL_BAND_VIRTUAL_INTERFACE		
	if (tp->port_id == 0) 
	{
		if (dev == GMACVIF0)
		{
			gmac0_vif0_down = 1;
			if (gmac0_vif1_down == 2)
				goto close;
			return (0);
		}
		if (dev == GMACVIF1)
		{
			gmac0_vif1_down = 2;
			return (0);
		}
	}
close:	
#endif	
	toe = (TOE_INFO_T *)&toe_private_data;

	tp->operation = 0;
    
    netif_stop_queue(dev);
    mdelay(20);
    
    /* stop tx/rx packet */
    toe_gmac_disable_tx_rx(dev);
    mdelay(20);
    
    /* stop the chip's Tx and Rx DMA processes */
	toe_gmac_hw_stop(dev);
	
	toe_gmac_disable_interrupt(tp->irq);

    /* disable interrupts by clearing the interrupt mask */
    synchronize_irq();
    free_irq(dev->irq,dev); 
    
#ifdef RTL8366_LINK_INTERRUPT
	/*0x3f is all event,seem not work always send event when any port link*/
	if(tp->port_id == GMAC_PORT1)
		free_gpio_irq(RTL8366_LINK_INTERRUPT_PIN);  
#else    
//joel
#ifndef RTL8366SR
	if(!FLAG_SWITCH)
#else	
	if(tp->auto_nego_cfg)
#endif	
	{
    	if (tp->thr_pid >= 0)
    	{
		    tp->time_to_die = 1;
    		wmb();
    		ret = kill_proc (tp->thr_pid, SIGTERM, 1);
    		if (ret)
    		{
    			printk (KERN_ERR "%s: unable to signal thread\n", dev->name);
    			return ret;
    		}
    		wait_for_completion (&tp->thr_exited);
    	}
    }
#ifdef DUAL_BAND_VIRTUAL_INTERFACE
	gmac0_vif0_down = 0;
	gmac0_vif1_down = 0;
#endif	
#endif//marco
    return (0);
}

/*----------------------------------------------------------------------
* toe_gmac_fill_free_q
* allocate buffers for free queue.
*----------------------------------------------------------------------*/
void toe_gmac_fill_free_q(void)
{
	struct sk_buff	*skb;
	volatile DMA_RWPTR_T	fq_rwptr;
	volatile GMAC_RXDESC_T	*fq_desc;
	unsigned long	flags;
#ifdef CONFIG_SL351X_IPSEC
	int						cq0_vacant_cnt, cq1_vacant_cnt;
	volatile DMA_RWPTR_T	cq0_rwptr, cq1_rwptr;
#endif		
#ifdef DEBUG_MEMORY_LEAKAGE	
	unsigned short max_cnt = TOE_SW_FREEQ_DESC_NUM;
	volatile DMA_RWPTR_T	dfq0_rwptr, dfq1_rwptr;
	volatile NONTOE_QHDR_T	*qhdr;
	int	q0_vacant_cnt, q1_vacant_cnt, min_vacant_cnt, fq_cnt;
#else
	unsigned short max_cnt = TOE_SW_FREEQ_DESC_NUM >> 1;
#endif

	spin_lock_irqsave(&gmac_fq_lock, flags);
	fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);

#ifdef DEBUG_MEMORY_LEAKAGE
	// number of buffers in software free queue.
	fq_cnt = (fq_rwptr.bits.wptr - fq_rwptr.bits.rptr + TOE_SW_FREEQ_DESC_NUM) & 
		(TOE_SW_FREEQ_DESC_NUM - 1);
	rmb();
	// number of buffers in default queue 0.
	qhdr = (NONTOE_QHDR_T*)TOE_DEFAULT_Q0_HDR_BASE;
	dfq0_rwptr.bits32 = qhdr->word1.bits32;
	q0_vacant_cnt = (TOE_DEFAULT_Q0_DESC_NUM - 1 + qhdr->word1.bits.rptr -
		qhdr->word1.bits.wptr) & (TOE_DEFAULT_Q0_DESC_NUM - 1);
	// number of buffers in default queue 1.
	qhdr = (NONTOE_QHDR_T*)TOE_DEFAULT_Q1_HDR_BASE;
	dfq1_rwptr.bits32 = qhdr->word1.bits32;
	q1_vacant_cnt = (TOE_DEFAULT_Q1_DESC_NUM - 1 + qhdr->word1.bits.rptr -
		qhdr->word1.bits.wptr) & (TOE_DEFAULT_Q1_DESC_NUM - 1);

#ifdef CONFIG_SL351X_IPSEC
	// number of buffer in classification queue for hardware accelerated IPSEC-VPN outbound
	qhdr = (NONTOE_QHDR_T*)(TOE_CLASS_Q_HDR_BASE + 0x8 * 0);
	cq0_rwptr.bits32 = qhdr->word1.bits32;
	cq0_vacant_cnt = (TOE_CLASS_DESC_NUM - 1 + qhdr->word1.bits.rptr - qhdr->word1.bits.wptr) & 
		(TOE_CLASS_DESC_NUM - 1);

	// number of buffer in classification queue for hardware accelerated IPSEC-VPN inbound
	qhdr = (NONTOE_QHDR_T*)(TOE_CLASS_Q_HDR_BASE + 0x8 * 1);
	cq1_rwptr.bits32 = qhdr->word1.bits32;
	cq1_vacant_cnt = (TOE_CLASS_DESC_NUM - 1 + qhdr->word1.bits.rptr - qhdr->word1.bits.wptr) & 
		(TOE_CLASS_DESC_NUM - 1);

//	printk("%s::cq0 vacant cnt = %d, cq1 vacant cnt = %d\n",__func__,cq0_vacant_cnt,cq1_vacant_cnt);
	min_vacant_cnt = ((q0_vacant_cnt < q1_vacant_cnt) ? q0_vacant_cnt : q1_vacant_cnt);
	min_vacant_cnt = ((min_vacant_cnt < cq0_vacant_cnt) ? min_vacant_cnt : cq0_vacant_cnt);
	min_vacant_cnt = ((min_vacant_cnt < cq1_vacant_cnt) ? min_vacant_cnt : cq1_vacant_cnt);
#else

	min_vacant_cnt = ((q0_vacant_cnt < q1_vacant_cnt) ? q0_vacant_cnt : q1_vacant_cnt);
#endif
	if(min_vacant_cnt>fq_cnt)
		min_vacant_cnt -= fq_cnt;
	//printk("%s::q0 %d, q1 %d, max %x, fq %x\n",
	//	__func__, q0_buf_cnt, q1_buf_cnt, max_buf_cnt, fq_vacant_cnt);
	//max_buf_cnt = ((max_buf_cnt < fq_vacant_cnt) ? max_buf_cnt : fq_vacant_cnt);
	max_cnt = ((max_cnt < min_vacant_cnt) ? max_cnt : min_vacant_cnt);
#endif
	while ((max_cnt--) && ((unsigned short)RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr,
			TOE_SW_FREEQ_DESC_NUM) != fq_rwptr.bits.rptr)) {
		if ((skb = dev_alloc_skb(SW_RX_BUF_SIZE)) == NULL) 
		{
			printk("%s::skb allocation fail!\n", __func__);
			//while(1);
			break;
		}
		REG32(skb->data) = (unsigned int)skb;
		skb_reserve(skb, SKB_RESERVE_BYTES);
		// fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
		fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr,TOE_SW_FREEQ_DESC_NUM);
		fq_desc = (GMAC_RXDESC_T*)toe_private_data.swfq_desc_base+fq_rwptr.bits.wptr;
		fq_desc->word2.buf_adr = (unsigned int)__pa(skb->data);
#ifdef DEBUG_MEMORY_LEAKAGE		
		consistent_sync((void*)fq_desc, sizeof(GMAC_RXDESC_T), PCI_DMA_TODEVICE);
		wmb();
		readl((GMAC_RXDESC_T*)toe_private_data.swfq_desc_base+fq_rwptr.bits.wptr);
#endif			
		SET_WPTR(TOE_GLOBAL_BASE+GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
		wmb();
		toe_private_data.fq_rx_rwptr.bits32 = fq_rwptr.bits32;
	}
	spin_unlock_irqrestore(&gmac_fq_lock, flags);
}
// EXPORT_SYMBOL(toe_gmac_fill_free_q);

/*----------------------------------------------------------------------
* toe_gmac_interrupt
*----------------------------------------------------------------------*/
static irqreturn_t toe_gmac_interrupt (int irq, void *dev_instance, struct pt_regs *regs)
{
	struct net_device   *dev = (struct net_device *)dev_instance;
	TOE_INFO_T			*toe;
	GMAC_INFO_T 		*tp = (GMAC_INFO_T *)dev->priv;
	unsigned int		status0;
	unsigned int		status1;
	unsigned int		status2;
	unsigned int		status3;
	unsigned int		status4;
	unsigned int		data32;
	
//	struct net_device_stats *isPtr = (struct net_device_stats *)&tp->ifStatics;
	toe = (TOE_INFO_T *)&toe_private_data;
//	handle NAPI	

	storlink_ctl.napi = 1;

#ifdef CONFIG_SL_NAPI 
if (storlink_ctl.napi == 1) 
{	  
/* disable GMAC interrupt */
   	toe_gmac_disable_interrupt(tp->irq);
    
//	isPtr->interrupts++;
	/* read Interrupt status */
	status0 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_0_REG);
	status1 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_1_REG);
	status2 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_2_REG);
	status3 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_3_REG);
	status4 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_4_REG);
	// prompt warning if status bit ON but not enabled
#if 0	
	if (status0 & ~tp->intr0_enabled)
		printk("Intr 0 Status error. status = 0x%X, enable = 0x%X\n", 
				status0, tp->intr0_enabled);
	if (status1 & ~tp->intr1_enabled)
		printk("Intr 1 Status error. status = 0x%X, enable = 0x%X\n", 
				status1, tp->intr1_enabled);
	if (status2 & ~tp->intr2_enabled)
		printk("Intr 2 Status error. status = 0x%X, enable = 0x%X\n", 
				status2, tp->intr2_enabled);
	if (status3 & ~tp->intr3_enabled)
		printk("Intr 3 Status error. status = 0x%X, enable = 0x%X\n", 
				status3, tp->intr3_enabled);
	if (status4 & ~tp->intr4_enabled)
		printk("Intr 4 Status error. status = 0x%X, enable = 0x%X\n", 
				status4, tp->intr4_enabled);
#endif

	if (status0) 
		writel(status0 & tp->intr0_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_0_REG);
	if (status1) 
		writel(status1 & tp->intr1_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_1_REG);
	if (status2) 
		writel(status2 & tp->intr2_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_2_REG);
	if (status3) 
		writel(status3 & tp->intr3_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_3_REG);
	if (status4) 
		writel(status4 & tp->intr4_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_4_REG);
	
	/* handle freeq interrupt first */
	if (status4 & tp->intr4_enabled) {
		if ((status4 & SWFQ_EMPTY_INT_BIT) && (tp->intr4_enabled & SWFQ_EMPTY_INT_BIT))
		{
			// unsigned long data = REG32(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
			//gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_4_REG,
			//	tp->intr4_enabled & ~SWFQ_EMPTY_INT_BIT, SWFQ_EMPTY_INT_BIT);

//			if (toe->gmac[0].dev && netif_running(toe->gmac[0].dev))
//				toe_gmac_handle_default_rxq(toe->gmac[0].dev,&toe->gmac[0]);
//			if (toe->gmac[1].dev && netif_running(toe->gmac[1].dev))
//				toe_gmac_handle_default_rxq(toe->gmac[1].dev,&toe->gmac[1]);
			printk("\nNAPI free_q empty INT\n");
			//toe_gmac_fill_free_q();
			
			// disable freeq interrupt
			data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_4_REG);
			data32 &= ~SWFQ_EMPTY_INT_BIT;
			writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_4_REG);
			tp->sw_fq_empty_cnt++;
		}
	}

	// Interrupt Status 1
	if (status1 & tp->intr1_enabled)
	{
		#define G1_INTR0_BITS	(GMAC1_HWTQ13_EOF_INT_BIT | GMAC1_HWTQ12_EOF_INT_BIT | GMAC1_HWTQ11_EOF_INT_BIT | GMAC1_HWTQ10_EOF_INT_BIT)
		#define G0_INTR0_BITS	(GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT | GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT)
		// Handle GMAC 0/1 HW Tx queue 0-3 EOF events
		// Only count 
		// TOE, Classification, and default queues interrupts are handled by ISR
		// because they should pass packets to upper layer
		if (tp->port_id == 0)
		{
#ifndef	INTERRUPT_SELECT
			if (netif_running(dev) && (status1 & G0_INTR0_BITS) && (tp->intr1_enabled & G0_INTR0_BITS))
			{
				if (status1 & GMAC0_HWTQ03_EOF_INT_BIT)
					tp->hwtxq[3].eof_cnt++;
				if (status1 & GMAC0_HWTQ02_EOF_INT_BIT)
					tp->hwtxq[2].eof_cnt++;
				if (status1 & GMAC0_HWTQ01_EOF_INT_BIT)
					tp->hwtxq[1].eof_cnt++;
				if (status1 & GMAC0_HWTQ00_EOF_INT_BIT)
					tp->hwtxq[0].eof_cnt++;
			}
#endif	//INTERRUPT_SELECT			
			if (netif_running(dev) && (status1 & DEFAULT_Q0_INT_BIT) &&
					(tp->intr1_enabled & DEFAULT_Q0_INT_BIT))
			{
				tp->poll_bits |= DEFAULT_Q0_INT_BIT;
				if (likely(netif_rx_schedule_prep(dev)))
				{
					// disable default queue#0 interrupt
					data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
//					data32 &= ~(DEFAULT_Q0_INT_BIT|TOE_IQ_ALL_BITS|TOE_CLASS_RX_INT_BITS);
//					data32 &= ~tp->intr1_enabled;
					data32 &= ~DEFAULT_Q0_INT_BIT;
					writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
	       				__netif_rx_schedule(dev);
        			}
				else {
					/* poll has been scheduled  */
        			// disable default queue#0 interrupt
					data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					data32 &= ~DEFAULT_Q0_INT_BIT;
					writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
				}
			}
#ifdef CONFIG_SL351X_IPSEC
			if (Giga_switch==1)
			{
				if (netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)) &&
						(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)))
				{
					tp->poll_bits |= CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
					if (likely(netif_rx_schedule_prep(dev)))
					{
        				// disable class queue interrupt under this device
        				data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
	       				__netif_rx_schedule(dev);
        			}
					else {
        				// disable class queue interrupt under this device
        				data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					}
				}
			}
			else {
				if (netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)) &&
						(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)))
				{
					tp->poll_bits |= CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
					if (likely(netif_rx_schedule_prep(dev)))
		    		{
						// disable class queue interrupt under this device
        				data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
		       			__netif_rx_schedule(dev);
		    		}
					else {
        				// disable class queue interrupt under this device
        				data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					}
				}
			}
#endif
		}
		if (tp->port_id == 1)
		{
#ifndef	INTERRUPT_SELECT			
			if (netif_running(dev) && (status1 & G1_INTR0_BITS) && (tp->intr1_enabled & G1_INTR0_BITS))
			{
				if (status1 & GMAC1_HWTQ13_EOF_INT_BIT)
					tp->hwtxq[3].eof_cnt++;
				if (status1 & GMAC1_HWTQ12_EOF_INT_BIT)
					tp->hwtxq[2].eof_cnt++;
				if (status1 & GMAC1_HWTQ11_EOF_INT_BIT)
					tp->hwtxq[1].eof_cnt++;
				if (status1 & GMAC1_HWTQ10_EOF_INT_BIT)
					tp->hwtxq[0].eof_cnt++;
			}
#endif	//INTERRUPT_SELECT			

			if (netif_running(dev) && (status1 & DEFAULT_Q1_INT_BIT) && 
				(tp->intr1_enabled & DEFAULT_Q1_INT_BIT))
			{
//				tp->poll_bits |= status1;
				tp->poll_bits |= DEFAULT_Q1_INT_BIT;
				if (likely(netif_rx_schedule_prep(dev)))
        		{
					// disable default queue#1 rx interrupt
					data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					data32 &= ~DEFAULT_Q1_INT_BIT;
					writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					__netif_rx_schedule(dev);
        		}
				else {
					// disable default queue#1 rx interrupt
        			data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
        			data32 &= ~DEFAULT_Q1_INT_BIT;
					writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
				}
			}
#ifdef CONFIG_SL351X_IPSEC
			if (Giga_switch)
			{
				if (netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)) && 
					(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)))
				{
					tp->poll_bits |= CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
					if (likely(netif_rx_schedule_prep(dev)))
					{
						// disable class-Q on this interface
						data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						__netif_rx_schedule(dev);
					}
					else {
						// disable class-Q on this interface
						data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					}
				}
			}
			else
			{
				if (netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)) && 
					(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)))
				{
					tp->poll_bits |= CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
					if (likely(netif_rx_schedule_prep(dev)))
	        		{
						// disable class-Q on this interface
						data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						__netif_rx_schedule(dev);
	        		}
					else {
						// disable class-Q on this interface
						data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
						data32 &= ~CLASS_RX_INT_BIT(IPSEC_INBOUND_QID);
						writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
					}
				}
			}
#endif
		} 
	}//end of if (status1 & tp->intr1_enabled)	

	// Interrupt Status 0
	if (status0 & tp->intr0_enabled)
	{
		#define ERR_INTR_BITS	(GMAC0_TXDERR_INT_BIT | GMAC0_TXPERR_INT_BIT |	\
								 GMAC1_TXDERR_INT_BIT | GMAC1_TXPERR_INT_BIT |	\
								 GMAC0_RXDERR_INT_BIT | GMAC0_RXPERR_INT_BIT |	\
								 GMAC1_RXDERR_INT_BIT | GMAC1_RXPERR_INT_BIT)
		
#ifndef	INTERRUPT_SELECT
		if (status0 &  ERR_INTR_BITS)
		{
			if ((status0 & GMAC0_TXDERR_INT_BIT) && (tp->intr0_enabled & GMAC0_TXDERR_INT_BIT))
			{
				tp->txDerr_cnt[0]++;
				printk("GMAC0 TX AHB Bus Error!\n");
			}
			if ((status0 & GMAC0_TXPERR_INT_BIT) && (tp->intr0_enabled & GMAC0_TXPERR_INT_BIT))
			{
				tp->txPerr_cnt[0]++;
				printk("GMAC0 Tx Descriptor Protocol Error!\n");
			}
			if ((status0 & GMAC1_TXDERR_INT_BIT) && (tp->intr0_enabled & GMAC1_TXDERR_INT_BIT))
			{
				tp->txDerr_cnt[1]++;
				printk("GMAC1 Tx AHB Bus Error!\n");
			}
			if ((status0 & GMAC1_TXPERR_INT_BIT) && (tp->intr0_enabled & GMAC1_TXPERR_INT_BIT))
			{
				tp->txPerr_cnt[1]++;
				printk("GMAC1 Tx Descriptor Protocol Error!\n");
			}
			
			if ((status0 & GMAC0_RXDERR_INT_BIT) && (tp->intr0_enabled & GMAC0_RXDERR_INT_BIT))
			{
				tp->RxDerr_cnt[0]++;
				printk("GMAC0 Rx AHB Bus Error!\n");
			}
			if ((status0 & GMAC0_RXPERR_INT_BIT) && (tp->intr0_enabled & GMAC0_RXPERR_INT_BIT))
			{
				tp->RxPerr_cnt[0]++;
				printk("GMAC0 Rx Descriptor Protocol Error!\n");
			}
			if ((status0 & GMAC1_RXDERR_INT_BIT) && (tp->intr0_enabled & GMAC1_RXDERR_INT_BIT))
			{
				tp->RxDerr_cnt[1]++;
				printk("GMAC1 Rx AHB Bus Error!\n");
			}
			if ((status0 & GMAC1_RXPERR_INT_BIT) && (tp->intr0_enabled & GMAC1_RXPERR_INT_BIT))
			{
				tp->RxPerr_cnt[1]++;
				printk("GMAC1 Rx Descriptor Protocol Error!\n");
			}
		}
#endif	//INTERRUPT_SELECT			
#ifndef	GMAX_TX_INTR_DISABLED
		if (tp->port_id == 1 &&	netif_running(dev) && 
			(((status0 & GMAC1_SWTQ10_FIN_INT_BIT) && (tp->intr0_enabled & GMAC1_SWTQ10_FIN_INT_BIT))
			||
			((status0 & GMAC1_SWTQ10_EOF_INT_BIT) && (tp->intr0_enabled & GMAC1_SWTQ10_EOF_INT_BIT))))
		{
			toe_gmac_tx_complete(&toe_private_data.gmac[1], 0, dev, 1);
		}
		
		if (tp->port_id == 0 &&	netif_running(dev) && 
			(((status0 & GMAC0_SWTQ00_FIN_INT_BIT) && (tp->intr0_enabled & GMAC0_SWTQ00_FIN_INT_BIT))
			||
			((status0 & GMAC0_SWTQ00_EOF_INT_BIT) && (tp->intr0_enabled & GMAC0_SWTQ00_EOF_INT_BIT))))
		{
			toe_gmac_tx_complete(&toe_private_data.gmac[0], 0, dev, 1);
		}
#endif		
	}
	
	// Interrupt Status 4
#ifndef	INTERRUPT_SELECT	
	if (status4 & tp->intr4_enabled)
	{
		#define G1_INTR4_BITS		(0xff000000)
		#define G0_INTR4_BITS		(0x00ff0000)
		
		if (tp->port_id == 0)
		{
			if ((status4 & G0_INTR4_BITS) && (tp->intr4_enabled & G0_INTR4_BITS))
			{
				if (status4 & GMAC0_RESERVED_INT_BIT)
					printk("GMAC0_RESERVED_INT_BIT is ON\n");
				if (status4 & GMAC0_MIB_INT_BIT)
					tp->mib_full_cnt++;
				if (status4 & GMAC0_RX_PAUSE_ON_INT_BIT)
					tp->rx_pause_on_cnt++;
				if (status4 & GMAC0_TX_PAUSE_ON_INT_BIT)
					tp->tx_pause_on_cnt++;
				if (status4 & GMAC0_RX_PAUSE_OFF_INT_BIT)
					tp->rx_pause_off_cnt++;
				if (status4 & GMAC0_TX_PAUSE_OFF_INT_BIT)
					tp->rx_pause_off_cnt++;
				if (status4 & GMAC0_RX_OVERRUN_INT_BIT)
					tp->rx_overrun_cnt++;
				if (status4 & GMAC0_STATUS_CHANGE_INT_BIT)
					tp->status_changed_cnt++;
			}
		}
		if (tp->port_id == 1)
		{
			if ((status4 & G1_INTR4_BITS) && (tp->intr4_enabled & G1_INTR4_BITS))
			{
				if (status4 & GMAC1_RESERVED_INT_BIT)
					printk("GMAC1_RESERVED_INT_BIT is ON\n");
				if (status4 & GMAC1_MIB_INT_BIT)
					tp->mib_full_cnt++;
				if (status4 & GMAC1_RX_PAUSE_ON_INT_BIT)
				{
					printk("Gmac pause on\n");
					tp->rx_pause_on_cnt++;
				}
				if (status4 & GMAC1_TX_PAUSE_ON_INT_BIT)
				{
					printk("Gmac pause on\n");
					tp->tx_pause_on_cnt++;
				}
				if (status4 & GMAC1_RX_PAUSE_OFF_INT_BIT)
				{
					printk("Gmac pause off\n");
					tp->rx_pause_off_cnt++;
				}
				if (status4 & GMAC1_TX_PAUSE_OFF_INT_BIT)
				{
					printk("Gmac pause off\n");
					tp->rx_pause_off_cnt++;
				}
				if (status4 & GMAC1_RX_OVERRUN_INT_BIT)
				{
					//printk("Gmac Rx Overrun \n");
					tp->rx_overrun_cnt++;
				}
				if (status4 & GMAC1_STATUS_CHANGE_INT_BIT)
					tp->status_changed_cnt++;
			}
		}
	}
#endif //Interrupt Status 4 INTERRUPT_SELECT
	
	toe_gmac_enable_interrupt(tp->irq);
	return	IRQ_RETVAL(1);
}	//end of (if (storlink_ctl.napi == 1)) 
else
{
#endif	//endif CONFIG_SL_NAPI

	
	/* disable GMAC interrupt */
    toe_gmac_disable_interrupt(tp->irq);
    
//	isPtr->interrupts++;
	/* read Interrupt status */
	status0 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_0_REG);
	status1 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_1_REG);
	status2 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_2_REG);
	status3 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_3_REG);
	status4 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_4_REG);
	// prompt warning if status bit ON but not enabled
#if 0	
	if (status0 & ~tp->intr0_enabled)
		printk("Intr 0 Status error. status = 0x%X, enable = 0x%X\n", 
				status0, tp->intr0_enabled);
	if (status1 & ~tp->intr1_enabled)
		printk("Intr 1 Status error. status = 0x%X, enable = 0x%X\n", 
				status1, tp->intr1_enabled);
	if (status2 & ~tp->intr2_enabled)
		printk("Intr 2 Status error. status = 0x%X, enable = 0x%X\n", 
				status2, tp->intr2_enabled);
	if (status3 & ~tp->intr3_enabled)
		printk("Intr 3 Status error. status = 0x%X, enable = 0x%X\n", 
				status3, tp->intr3_enabled);
	if (status4 & ~tp->intr4_enabled)
		printk("Intr 4 Status error. status = 0x%X, enable = 0x%X\n", 
				status4, tp->intr4_enabled);
#endif

	if (status0) 
		writel(status0 & tp->intr0_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_0_REG);
	if (status1) 
		writel(status1 & tp->intr1_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_1_REG);
	if (status2) 
		writel(status2 & tp->intr2_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_2_REG);
	if (status3) 
		writel(status3 & tp->intr3_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_3_REG);
	if (status4) 
		writel(status4 & tp->intr4_enabled, TOE_GLOBAL_BASE+GLOBAL_INTERRUPT_STATUS_4_REG);
	
	/* handle freeq interrupt first */
	if (status4 & tp->intr4_enabled) {
		if ((status4 & SWFQ_EMPTY_INT_BIT) && (tp->intr4_enabled & SWFQ_EMPTY_INT_BIT))
		{
			// unsigned long data = REG32(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
			//gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_4_REG,
			//	tp->intr4_enabled & ~SWFQ_EMPTY_INT_BIT, SWFQ_EMPTY_INT_BIT);

			//gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_STATUS_4_REG,
			//	SWFQ_EMPTY_INT_BIT, SWFQ_EMPTY_INT_BIT);
//			printk("\nSWFQ_EMPTY_INT_BIT  freeq int\n");
			if (toe->gmac[0].dev && netif_running(toe->gmac[0].dev))
				toe_gmac_handle_default_rxq(toe->gmac[0].dev,&toe->gmac[0]);
			if (toe->gmac[1].dev && netif_running(toe->gmac[1].dev))
				toe_gmac_handle_default_rxq(toe->gmac[1].dev,&toe->gmac[1]);
			printk("\nfreeq int\n");
			//toe_gmac_fill_free_q();
			tp->sw_fq_empty_cnt++;

			gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_STATUS_4_REG, status4,
				SWFQ_EMPTY_INT_BIT);
		}
	}

	// Interrupt Status 1
	if (status1 & tp->intr1_enabled)
	{
		#define G1_INTR0_BITS	(GMAC1_HWTQ13_EOF_INT_BIT | GMAC1_HWTQ12_EOF_INT_BIT | GMAC1_HWTQ11_EOF_INT_BIT | GMAC1_HWTQ10_EOF_INT_BIT)
		#define G0_INTR0_BITS	(GMAC0_HWTQ03_EOF_INT_BIT | GMAC0_HWTQ02_EOF_INT_BIT | GMAC0_HWTQ01_EOF_INT_BIT | GMAC0_HWTQ00_EOF_INT_BIT)
		// Handle GMAC 0/1 HW Tx queue 0-3 EOF events
		// Only count 
		// TOE, Classification, and default queues interrupts are handled by ISR
		// because they should pass packets to upper layer
		if (tp->port_id == 0)
		{
#ifndef	INTERRUPT_SELECT
			if (netif_running(dev) && (status1 & G0_INTR0_BITS) && (tp->intr1_enabled & G0_INTR0_BITS))
			{
				if (status1 & GMAC0_HWTQ03_EOF_INT_BIT)
					tp->hwtxq[3].eof_cnt++;
				if (status1 & GMAC0_HWTQ02_EOF_INT_BIT)
					tp->hwtxq[2].eof_cnt++;
				if (status1 & GMAC0_HWTQ01_EOF_INT_BIT)
					tp->hwtxq[1].eof_cnt++;
				if (status1 & GMAC0_HWTQ00_EOF_INT_BIT)
					tp->hwtxq[0].eof_cnt++;
#endif	//INTERRUPT_SELECT
#ifndef	INTERRUPT_SELECT
			}
#endif	//INTERRUPT_SELECT
			if (netif_running(dev) && (status1 & DEFAULT_Q0_INT_BIT) && (tp->intr1_enabled & DEFAULT_Q0_INT_BIT))
			{
				tp->default_q_intr_cnt++;
				toe_gmac_handle_default_rxq(dev, tp);
			}
#ifdef CONFIG_SL351x_RXTOE
			if (netif_running(dev) && (status1 & TOE_IQ_ALL_BITS) && 
			    (tp->intr1_enabled & TOE_IQ_ALL_BITS)) {
				//printk("status %x, bits %x, slct %x\n", status1, TOE_IQ_ALL_BITS, tp->intr1_selected);
				toe_gmac_handle_toeq(dev, tp, status1);
				//toe_gmac_handle_toeq(dev, toe, tp, status1);
			}
#endif
#ifdef CONFIG_SL351X_IPSEC
			if (Giga_switch && netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)) &&
				(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID))) {
					ipsec_handle_class_queue(dev,tp,0);
			}
			else if ((Giga_switch==0) && netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)) &&
				(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID))) {
					ipsec_handle_class_queue(dev,tp,0);
			}
#endif
		}
		if (tp->port_id == 1)
		{
#ifndef	INTERRUPT_SELECT
			if (netif_running(dev) && (status1 & G1_INTR0_BITS) && (tp->intr1_enabled & G1_INTR0_BITS))
			{
				if (status1 & GMAC1_HWTQ13_EOF_INT_BIT)
					tp->hwtxq[3].eof_cnt++;
				if (status1 & GMAC1_HWTQ12_EOF_INT_BIT)
					tp->hwtxq[2].eof_cnt++;
				if (status1 & GMAC1_HWTQ11_EOF_INT_BIT)
					tp->hwtxq[1].eof_cnt++;
				if (status1 & GMAC1_HWTQ10_EOF_INT_BIT)
					tp->hwtxq[0].eof_cnt++;
#endif	//INTERRUPT_SELECT
#ifndef	INTERRUPT_SELECT
			}
#endif	//INTERRUPT_SELECT			
			if (netif_running(dev) && (status1 & DEFAULT_Q1_INT_BIT) && (tp->intr1_enabled & DEFAULT_Q1_INT_BIT))
			{
				tp->default_q_intr_cnt++;
				toe_gmac_handle_default_rxq(dev, tp);
			}
#ifdef CONFIG_SL351x_RXTOE
			if (netif_running(dev) && (status1 & TOE_IQ_ALL_BITS) && 
			    (tp->intr1_enabled & TOE_IQ_ALL_BITS)) {
				//printk("status %x, bits %x, slct %x\n", status1, TOE_IQ_ALL_BITS, tp->intr1_selected);
				toe_gmac_handle_toeq(dev, tp, status1);
				//toe_gmac_handle_toeq(dev, toe, tp, status1);
			}
#endif
#ifdef CONFIG_SL351X_IPSEC
			if (Giga_switch && netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)) &&
				(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID))) {
					ipsec_handle_class_queue(dev,tp,0);
			}
			else if ((Giga_switch==0) && netif_running(dev) && (status1 & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)) &&
				(tp->intr1_enabled & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID))) {
					ipsec_handle_class_queue(dev,tp,0);
			}
#endif
		} 
	}	
	

	// Interrupt Status 0
	if (status0 & tp->intr0_enabled)
	{

		#define ERR_INTR_BITS	(GMAC0_TXDERR_INT_BIT | GMAC0_TXPERR_INT_BIT |	\
								 GMAC1_TXDERR_INT_BIT | GMAC1_TXPERR_INT_BIT |	\
								 GMAC0_RXDERR_INT_BIT | GMAC0_RXPERR_INT_BIT |	\
								 GMAC1_RXDERR_INT_BIT | GMAC1_RXPERR_INT_BIT)
#ifndef	INTERRUPT_SELECT
		if (status0 &  ERR_INTR_BITS)
		{
			if ((status0 & GMAC0_TXDERR_INT_BIT) && (tp->intr0_enabled & GMAC0_TXDERR_INT_BIT))
			{
				tp->txDerr_cnt[0]++;
				printk("GMAC0 TX AHB Bus Error!\n");
			}
			if ((status0 & GMAC0_TXPERR_INT_BIT) && (tp->intr0_enabled & GMAC0_TXPERR_INT_BIT))
			{
				tp->txPerr_cnt[0]++;
				printk("GMAC0 Tx Descriptor Protocol Error!\n");
			}
			if ((status0 & GMAC1_TXDERR_INT_BIT) && (tp->intr0_enabled & GMAC1_TXDERR_INT_BIT))
			{
				tp->txDerr_cnt[1]++;
				printk("GMAC1 Tx AHB Bus Error!\n");
			}
			if ((status0 & GMAC1_TXPERR_INT_BIT) && (tp->intr0_enabled & GMAC1_TXPERR_INT_BIT))
			{
				tp->txPerr_cnt[1]++;
				printk("GMAC1 Tx Descriptor Protocol Error!\n");
			}
			
			if ((status0 & GMAC0_RXDERR_INT_BIT) && (tp->intr0_enabled & GMAC0_RXDERR_INT_BIT))
			{
				tp->RxDerr_cnt[0]++;
				printk("GMAC0 Rx AHB Bus Error!\n");
			}
			if ((status0 & GMAC0_RXPERR_INT_BIT) && (tp->intr0_enabled & GMAC0_RXPERR_INT_BIT))
			{
				tp->RxPerr_cnt[0]++;
				printk("GMAC0 Rx Descriptor Protocol Error!\n");
			}
			if ((status0 & GMAC1_RXDERR_INT_BIT) && (tp->intr0_enabled & GMAC1_RXDERR_INT_BIT))
			{
				tp->RxDerr_cnt[1]++;
				printk("GMAC1 Rx AHB Bus Error!\n");
			}
			if ((status0 & GMAC1_RXPERR_INT_BIT) && (tp->intr0_enabled & GMAC1_RXPERR_INT_BIT))
			{
				tp->RxPerr_cnt[1]++;
				printk("GMAC1 Rx Descriptor Protocol Error!\n");
			}
		}
#endif	//#ifndef INTERRUPT_SELECT	
#ifndef	GMAX_TX_INTR_DISABLED
		if (tp->port_id == 1 &&	netif_running(dev) && 
			(((status0 & GMAC1_SWTQ10_FIN_INT_BIT) && (tp->intr0_enabled & GMAC1_SWTQ10_FIN_INT_BIT))
			||
			((status0 & GMAC1_SWTQ10_EOF_INT_BIT) && (tp->intr0_enabled & GMAC1_SWTQ10_EOF_INT_BIT))))
		{
			toe_gmac_tx_complete(&toe_private_data.gmac[1], 0, dev, 1);
		}
		
		if (tp->port_id == 0 &&	netif_running(dev) && 
			(((status0 & GMAC0_SWTQ00_FIN_INT_BIT) && (tp->intr0_enabled & GMAC0_SWTQ00_FIN_INT_BIT))
			||
			((status0 & GMAC0_SWTQ00_EOF_INT_BIT) && (tp->intr0_enabled & GMAC0_SWTQ00_EOF_INT_BIT))))
		{
			toe_gmac_tx_complete(&toe_private_data.gmac[0], 0, dev, 1);
		}
#endif		
		// clear enabled status bits
	}
	// Interrupt Status 4
#ifndef	INTERRUPT_SELECT
	if (status4 & tp->intr4_enabled)
	{
		#define G1_INTR4_BITS		(0xff000000)
		#define G0_INTR4_BITS		(0x00ff0000)
		
		if (tp->port_id == 0)
		{
			if ((status4 & G0_INTR4_BITS) && (tp->intr4_enabled & G0_INTR4_BITS))
			{
				if (status4 & GMAC0_RESERVED_INT_BIT)
					printk("GMAC0_RESERVED_INT_BIT is ON\n");
				if (status4 & GMAC0_MIB_INT_BIT)
					tp->mib_full_cnt++;
				if (status4 & GMAC0_RX_PAUSE_ON_INT_BIT)
					tp->rx_pause_on_cnt++;
				if (status4 & GMAC0_TX_PAUSE_ON_INT_BIT)
					tp->tx_pause_on_cnt++;
				if (status4 & GMAC0_RX_PAUSE_OFF_INT_BIT)
					tp->rx_pause_off_cnt++;
				if (status4 & GMAC0_TX_PAUSE_OFF_INT_BIT)
					tp->rx_pause_off_cnt++;
				if (status4 & GMAC0_RX_OVERRUN_INT_BIT)
					tp->rx_overrun_cnt++;
				if (status4 & GMAC0_STATUS_CHANGE_INT_BIT)
					tp->status_changed_cnt++;
			}
		}
		if (tp->port_id == 1)
		{
			if ((status4 & G1_INTR4_BITS) && (tp->intr4_enabled & G1_INTR4_BITS))
			{
				if (status4 & GMAC1_RESERVED_INT_BIT)
					printk("GMAC1_RESERVED_INT_BIT is ON\n");
				if (status4 & GMAC1_MIB_INT_BIT)
					tp->mib_full_cnt++;
				if (status4 & GMAC1_RX_PAUSE_ON_INT_BIT)
				{
					//printk("Gmac pause on\n");
					tp->rx_pause_on_cnt++;
				}
				if (status4 & GMAC1_TX_PAUSE_ON_INT_BIT)
				{
					//printk("Gmac pause on\n");
					tp->tx_pause_on_cnt++;
				}
				if (status4 & GMAC1_RX_PAUSE_OFF_INT_BIT)
				{
					//printk("Gmac pause off\n");
					tp->rx_pause_off_cnt++;
				}
				if (status4 & GMAC1_TX_PAUSE_OFF_INT_BIT)
				{
					//printk("Gmac pause off\n");
					tp->rx_pause_off_cnt++;
				}
				if (status4 & GMAC1_RX_OVERRUN_INT_BIT)
				{
					//printk("Gmac Rx Overrun \n");
					tp->rx_overrun_cnt++;
				}
				if (status4 & GMAC1_STATUS_CHANGE_INT_BIT)
					tp->status_changed_cnt++;
			}
		}
#if 0
		if ((status4 & SWFQ_EMPTY_INT_BIT) && (tp->intr4_enabled & SWFQ_EMPTY_INT_BIT))
		{
			// unsigned long data = REG32(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
//			mac_stop_rxdma(tp->sc);
			gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_ENABLE_4_REG,
				tp->intr4_enabled & ~SWFQ_EMPTY_INT_BIT, SWFQ_EMPTY_INT_BIT);

			gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_STATUS_4_REG,
				SWFQ_EMPTY_INT_BIT, SWFQ_EMPTY_INT_BIT);
			toe_gmac_fill_free_q();
			tp->sw_fq_empty_cnt++;

			gmac_write_reg(TOE_GLOBAL_BASE, GLOBAL_INTERRUPT_STATUS_4_REG, status4,
				SWFQ_EMPTY_INT_BIT);
//#if 0
/*			if (netif_running(dev))
				toe_gmac_handle_default_rxq(dev, tp);
			printk("SWFQ_EMPTY_INT_BIT is ON!\n");	// should not be happened */
//#endif
		}
#endif
	}
#endif	//INTERRUPT_SELECT		 
	toe_gmac_enable_interrupt(tp->irq);
	//printk("gmac_interrupt complete!\n\n");
//	return IRQ_RETVAL(handled);
	return	IRQ_RETVAL(1);
#ifdef CONFIG_SL_NAPI  
}
#endif
}

/*----------------------------------------------------------------------
*	toe_gmac_handle_default_rxq
*	(1) Get rx Buffer for default Rx queue
*	(2) notify or call upper-routine to handle it
*	(3) get a new buffer and insert it into SW free queue
*	(4) Note: The SW free queue Read-Write Pointer should be locked when accessing
*----------------------------------------------------------------------*/
static inline void toe_gmac_handle_default_rxq(struct net_device *dev, GMAC_INFO_T *tp)
//static void toe_gmac_handle_default_rxq(struct net_device *dev, GMAC_INFO_T *tp)
{
	TOE_INFO_T			*toe;
    GMAC_RXDESC_T   	*curr_desc;
	struct sk_buff 		*skb;
    DMA_RWPTR_T			rwptr;
	unsigned int 		pkt_size;
	int					max_cnt;
	unsigned int        desc_count;
	unsigned int        good_frame, chksum_status, rx_status;
#ifdef CONFIG_SL351x_RTLDMZ		
	unsigned short		vid_skb = 0;
#endif		
	struct net_device_stats *isPtr = (struct net_device_stats *)&tp->ifStatics;
	
	rwptr.bits32 = readl(&tp->default_qhdr->word1);
#if 0
	if (rwptr.bits.rptr != tp->rx_rwptr.bits.rptr)
	{ 
		mac_stop_txdma((struct net_device *)tp->dev);
		printk("Default Queue HW RD ptr (0x%x) != SW RD Ptr (0x%x)\n",
				rwptr.bits32, tp->rx_rwptr.bits.rptr);
		while(1);
	}
#endif
	toe = (TOE_INFO_T *)&toe_private_data;
	max_cnt = DEFAULT_RXQ_MAX_CNT;
	while ((--max_cnt) && rwptr.bits.rptr != rwptr.bits.wptr)
//	while (rwptr.bits.rptr != rwptr.bits.wptr)
	{	       
    	curr_desc = (GMAC_RXDESC_T *)tp->default_desc_base + rwptr.bits.rptr;
//		consistent_sync(curr_desc, sizeof(GMAC_RXDESC_T), PCI_DMA_FROMDEVICE);
#ifdef GMAC_DEBUG
		tp->default_q_cnt++;
#endif
    	tp->rx_curr_desc = (unsigned int)curr_desc;
    	rx_status = curr_desc->word0.bits.status;
    	chksum_status = curr_desc->word0.bits.chksum_status;
#ifdef GMAC_DEBUG
    	tp->rx_status_cnt[rx_status]++;
    	tp->rx_chksum_cnt[chksum_status]++;
#endif
        pkt_size = curr_desc->word1.bits.byte_count;  /*total byte count in a frame*/
		desc_count = curr_desc->word0.bits.desc_count; /* get descriptor count per frame */
		good_frame=1;
		if ((curr_desc->word0.bits32 & (GMAC_RXDESC_0_T_derr | GMAC_RXDESC_0_T_perr))
			|| (pkt_size < 60)
		    || (chksum_status & 0x4)
			|| rx_status)
		{
			good_frame = 0;
#ifdef GMAC_DEBUG
			if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_derr)
				printk("%s::derr (GMAC-%d)!!!\n", __func__, tp->port_id);
			if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_perr)
				printk("%s::perr (GMAC-%d)!!!\n", __func__, tp->port_id);
#endif
			if (rx_status)
			{
				if (rx_status == 4 || rx_status == 7)
					isPtr->rx_crc_errors++;
//				printk("%s::Status=%d (GMAC-%d)!!!\n", __func__, rx_status, tp->port_id);
			}
#ifdef SL351x_GMAC_WORKAROUND
			else if (pkt_size < 60)
			{
				if (tp->short_frames_cnt < GMAC_SHORT_FRAME_THRESHOLD)
					tp->short_frames_cnt++;
				if (tp->short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
				{
					GMAC_CONFIG0_T config0;
					config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
					config0.bits.dis_rx = 1;
					writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
					config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
					config0.bits.dis_rx = 1;
					writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
				}
			}
#endif
//			if (chksum_status)
//				printk("%s::Checksum Status=%d (GMAC-%d)!!!\n", __func__, chksum_status, tp->port_id);
			skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
			dev_kfree_skb_irq(skb);
		}
		if (good_frame)
		{
#ifdef GMAC_DEBUG
			if (curr_desc->word0.bits.drop)
				printk("%s::Drop (GMAC-%d)!!!\n", __func__, tp->port_id);
#endif
//			if (chksum_status)
//				printk("%s::Checksum Status=%d (GMAC-%d)!!!\n", __func__, chksum_status, tp->port_id);
				
	    	/* get frame information from the first descriptor of the frame */
#ifdef SL351x_GMAC_WORKAROUND
			if (tp->short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
			{
				GMAC_CONFIG0_T config0;
				config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
				config0.bits.dis_rx = 0;
				writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
				config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
				config0.bits.dis_rx = 0;
				writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
			}
			tp->short_frames_cnt = 0;
#endif
			isPtr->rx_packets++;
			skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr - SKB_RESERVE_BYTES)));
			if (!skb)
			{
				printk("Fatal Error!!skb==NULL\n");
				goto next_rx;
			}
			tp->curr_rx_skb = skb;
			// consistent_sync((void *)__va(curr_desc->word2.buf_adr), pkt_size, PCI_DMA_FROMDEVICE);					
	
	//		curr_desc->word2.buf_adr = 0;
			
			skb_reserve (skb, RX_INSERT_BYTES);	/* 16 byte align the IP fields. */
			skb_put(skb, pkt_size);
			skb->dev = dev;
#if 0		//add by Wen		
			struct iphdr	*ip_hdr2 = (struct iphdr*)&(skb->data[curr_desc->word3.bits.l3_offset]);
			if ((ip_hdr2->tot_len + ENET_HEADER_SIZE) < 60)
				skb_trim(skb, ip_hdr2->tot_len + ENET_HEADER_SIZE);
#endif			
			if (chksum_status == RX_CHKSUM_IP_UDP_TCP_OK)
			{
				skb->ip_summed = CHECKSUM_UNNECESSARY;
#ifdef CONFIG_SL351x_NAT
				if (nat_cfg.enabled && curr_desc->word3.bits.l3_offset && curr_desc->word3.bits.l4_offset)
				{
					struct iphdr	*ip_hdr;
					ip_hdr = (struct iphdr *)&(skb->data[curr_desc->word3.bits.l3_offset]);
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
				}
#endif
#ifdef CONFIG_SL351X_BR
				sl351x_br_input(skb,tp->port_id);
#endif
				skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
#if 0
#ifdef CONFIG_SL351x_RXTOE
				if (storlink_ctl.rx_max_pktsize) {
					struct iphdr	*ip_hdr;
					struct tcphdr	*tcp_hdr;
					int ip_hdrlen;
 					
 					ip_hdr = (struct iphdr*)&(skb->data[0]);
					if ((skb->protocol == __constant_htons(ETH_P_IP)) && 
					   ((ip_hdr->protocol & 0x00ff) == IPPROTO_TCP)) {
						ip_hdrlen = ip_hdr->ihl << 2;
						tcp_hdr = (struct tcphdr*)&(skb->data[ip_hdrlen]);
						if (tcp_hdr->syn) {
							struct toe_conn* connection = init_toeq(ip_hdr->version,
									ip_hdr, tcp_hdr, toe, &(skb->data[0]) - 14);
							TCP_SKB_CB(skb)->connection = connection;
							//	hash_dump_entry(TCP_SKB_CB(skb)->connection->hash_entry_index);
							//		printk("%s::skb data %x, conn %x, mode %x\n",
							//			__func__, skb->data, connection, connection->mode);
						}
					}
				}
#endif
#endif
			}
			else if (chksum_status == RX_CHKSUM_IP_OK_ONLY)
			{
				skb->ip_summed = CHECKSUM_UNNECESSARY;
#ifdef CONFIG_SL351x_NAT
				if (nat_cfg.enabled && curr_desc->word3.bits.l3_offset && curr_desc->word3.bits.l4_offset)
				{
					struct iphdr		*ip_hdr;
					//struct tcphdr		*tcp_hdr;
					ip_hdr = (struct iphdr *)&(skb->data[curr_desc->word3.bits.l3_offset]);
					//tcp_hdr = (struct tcphdr *)&(skb->data[curr_desc->word3.bits.l4_offset]);
					if (ip_hdr->protocol == IPPROTO_UDP)
					{
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
					}
					else if (ip_hdr->protocol == IPPROTO_GRE)
					{
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
					}
				}
#endif		
#ifdef CONFIG_SL351X_BR
			sl351x_br_input(skb,tp->port_id); 
#endif	
				skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
			}
			else
			{
#ifdef CONFIG_SL351X_BR				
				sl351x_br_input(skb,tp->port_id);  
#endif					
				skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
			}

			netif_rx(skb);  /* socket rx */

			dev->last_rx = jiffies;

			isPtr->rx_bytes += pkt_size;
			
        }

next_rx:	    	
		// advance one for Rx default Q 0/1
		rwptr.bits.rptr = RWPTR_ADVANCE_ONE(rwptr.bits.rptr, tp->default_desc_num);
		SET_RPTR(&tp->default_qhdr->word1, rwptr.bits.rptr);
     	tp->rx_rwptr.bits32 = rwptr.bits32;
//		toe_gmac_fill_free_q();
	}
	toe_gmac_fill_free_q();
}

/*----------------------------------------------------------------------
* gmac_get_phy_vendor
*----------------------------------------------------------------------*/
static unsigned int gmac_get_phy_vendor(int phy_addr)
{
    unsigned int	reg_val;
    reg_val=(mii_read(phy_addr,0x02) << 16) + mii_read(phy_addr,0x03);
    return reg_val;
}    

/*----------------------------------------------------------------------
* gmac_set_phy_status
*----------------------------------------------------------------------*/
void gmac_set_phy_status(struct net_device *dev)
{
	GMAC_INFO_T *tp = dev->priv;
	GMAC_STATUS_T   status;
	unsigned int    reg_val, ability,wan_port_id;
	unsigned int    i = 0;

#if defined(RTL8366SR) || defined(ALPHA_RTL8366SR_MODULE) //joel
	if(tp->port_id == GMAC_PORT0/*4 port switch*/){
		GMAC_CONFIG0_T	config0,config0_mask;
		config0.bits32 = 0;
		config0_mask.bits32 = 0;
		config0.bits.tx_fc_en = 1; /* enable tx flow control */
		config0.bits.rx_fc_en = 1; /* enable rx flow control */
		config0_mask.bits.tx_fc_en = 1;
		config0_mask.bits.rx_fc_en = 1;
		gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
		printk("GMAC-%d Flow Control Enable.\n", tp->port_id);
		gmac_write_reg(tp->base_addr, GMAC_STATUS, 0x7d, 0x0000007f);
		toe_gmac_enable_tx_rx(dev);
		netif_wake_queue(dev);
		set_bit(__LINK_STATE_START, &dev->state);
		tp->pre_phy_status = LINK_UP;
		storlink_ctl.link[tp->port_id] = 1;
		return ;
	}
	else if(tp->port_id == GMAC_PORT1)/*PHY 4*/
	{
		GMAC_CONFIG0_T	config0,config0_mask;
		config0.bits32 = 0;
		config0_mask.bits32 = 0;
		config0.bits.tx_fc_en = 1; /* enable tx flow control */
		config0.bits.rx_fc_en = 1; /* enable rx flow control */
		config0_mask.bits.tx_fc_en = 1;
		config0_mask.bits.rx_fc_en = 1;
		gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
		printk("GMAC-%d Flow Control Enable.\n", tp->port_id);
		gmac_write_reg(tp->base_addr, GMAC_STATUS, 0x7d, 0x0000007f);
		toe_gmac_enable_tx_rx(dev);
		netif_wake_queue(dev);
		set_bit(__LINK_STATE_START, &dev->state);
		tp->pre_phy_status = LINK_UP;
		return ;
	}
#endif	
#ifdef VITESSE_G5SWITCH
	if((tp->port_id == GMAC_PORT1)&&(Giga_switch==1)){
		gmac_get_switch_status(dev);
		gmac_write_reg(tp->base_addr, GMAC_STATUS, 0x7d, 0x0000007f);
//		SPI_write(2,0,0x10,rcv_mask);			// Enable Receive
		return ;
	}
#endif

#ifdef STORLINK_PHY  
	if (tp->port_id == 0)
	{
		mii_write(tp->phy_addr,0x0,0x8000);
		mii_write(tp->phy_addr,16,0x8000);
		mii_write(tp->phy_addr,17,0x4040);
		mii_write(tp->phy_addr,18,0x1a38);
		mii_write(tp->phy_addr,16,0x4000);
		mii_write(tp->phy_addr,22,0x02d4);
		mii_write(tp->phy_addr,23,0x02d4);
		mii_write(tp->phy_addr,16,0x0000);
		printk("=> STORM PHY Initial.\n");
	}
#endif

	reg_val = gmac_get_phy_vendor(tp->phy_addr);
	printk("GMAC-%d Addr %d Vendor ID: 0x%08x\n", tp->port_id, tp->phy_addr, reg_val);
	switch (tp->phy_mode)
	{
		case GMAC_PHY_GMII:
		mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
		#ifdef CONFIG_SL3516_ASIC
		mii_write(tp->phy_addr,0x09,0x0300); /* advertise 1000M full/half duplex */
		#else
		mii_write(tp->phy_addr,0x09,0x0000); /* advertise no 1000M full/half duplex */
		#endif
		break;
		case GMAC_PHY_RGMII_100:
		#ifdef STORLINK_PHY 		
		mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
		#else		
		mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
		#endif		
		mii_write(tp->phy_addr,0x09,0x0000); /* advertise no 1000M */
		break;
		case GMAC_PHY_RGMII_1000:
		#ifdef STORLINK_PHY 
		mii_write(tp->phy_addr,0x04,0x01e1); /* advertisement STORM phy to 100M full duplex, pause capable on */
		#else		
		mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
		#endif		
		#ifdef CONFIG_SL3516_ASIC
		#ifdef STORLINK_PHY 		
		mii_write(tp->phy_addr,0x09,0x0f00); /* advertise STORM phy to 1000M full/half duplex */
		#else			
		mii_write(tp->phy_addr,0x09,0x0300); /* advertise 1000M full/half duplex */
		#endif		
		#else
		mii_write(tp->phy_addr,0x09,0x0000); /* advertise no 1000M full/half duplex */
		#endif
		break;
		case GMAC_PHY_MII:
		default:
		mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement 100M full duplex, pause capable on */
		mii_write(tp->phy_addr,0x09,0x0000); /* advertise no 1000M */
		break;
	}
	
#if defined(ICPLUS_PHY) || defined(REALTEK_PHY) || defined(CONFIG_RTL8366SR_PHY)
	printk(" IC PLUS or RealTek Phy select .\n");
#else
#ifndef STORLINK_PHY
	mii_write(tp->phy_addr,0x18,0x0041);	// Phy active led
#endif	
#endif	

#ifndef STORLINK_PHY		//Marvell phy do reset phy
	if (tp->auto_nego_cfg)
	{
		reg_val = 0x1200 | (1 << 15);
		mii_write(tp->phy_addr,0x00,reg_val); /* Enable and Restart Auto-Negotiation */
		mdelay(500);
		reg_val &= ~(1 << 15);
		mii_write(tp->phy_addr, 0x00, reg_val);
	}
	else
	{
		reg_val = 0;
		reg_val |= (tp->full_duplex_cfg) ? (1 << 8) : 0;
		reg_val |= (tp->speed_cfg == GMAC_SPEED_1000) ? (1 << 6) : 0;
		reg_val |= (tp->speed_cfg == GMAC_SPEED_100) ? (1 << 13) : 0;
		mii_write(tp->phy_addr, 0x00, reg_val);
		mdelay(100);

		reg_val |= (1 << 15);	// Reset PHY;
		mii_write(tp->phy_addr, 0x00, reg_val);
	}
#endif

	status.bits32 = 0;
	/* set PHY operation mode */
	status.bits.mii_rmii = tp->phy_mode;
	status.bits.reserved = 1;
	mdelay(100);
	while (((reg_val=mii_read(tp->phy_addr,0x01)) & 0x00000004)!=0x04)
	{
		msleep(100);
		i++;
		if (i > 30)
		break;
	}
	if (i>30)
	{
		tp->pre_phy_status = LINK_DOWN;
		status.bits.link = LINK_DOWN;
		clear_bit(__LINK_STATE_START, &dev->state);
		printk("Link Down (0x%04x) ", reg_val);
		netif_carrier_off(dev);
		netif_stop_queue(dev);
		if(Giga_switch == 1)
		{
				wan_port_id = 1;
				storlink_ctl.link[ wan_port_id] = 0;
		}
		else
		{
				storlink_ctl.link[ tp->port_id] = 0;
		}
	}
	else
	{
		tp->pre_phy_status = LINK_UP;
		status.bits.link = LINK_UP;
		set_bit(__LINK_STATE_START, &dev->state);
		printk("Link Up (0x%04x) ",reg_val);
		netif_carrier_on(dev);
		if(Giga_switch == 1)
		{
				wan_port_id = 1;
				storlink_ctl.link[ wan_port_id] = 1;
		}
		else
		{
				storlink_ctl.link[ tp->port_id] = 1;
		}	
	}
	//    value = mii_read(PHY_ADDR,0x05);

	ability = (mii_read(tp->phy_addr,0x05) & 0x05E0) >> 5;

	//#ifdef CONFIG_SL3516_ASIC
	reg_val = mii_read(tp->phy_addr,10);
	printk("MII REG 10 = 0x%x\n",reg_val);

#ifdef STORLINK_PHY		
	if ((reg_val & 0x3800) == 0x3800)
#else
	if ((reg_val & 0x0800) == 0x0800)
#endif		
	{
		status.bits.duplex = 1;
		status.bits.speed = 2;
		if (status.bits.mii_rmii == GMAC_PHY_RGMII_100)
		status.bits.mii_rmii = GMAC_PHY_RGMII_1000;

		printk(" 1000M/Full \n");
	}
#ifdef STORLINK_PHY		
	else if ((reg_val & 0x3400) == 0x3400)
#else
	else if ((reg_val & 0x0400) == 0x0400)
#endif		
	{
		status.bits.duplex = 0;
		status.bits.speed = 2;
		if (status.bits.mii_rmii == GMAC_PHY_RGMII_100)
		status.bits.mii_rmii = GMAC_PHY_RGMII_1000;

		printk(" 1000M/Half \n");
	}
	//#endif
	else
	{
		#ifdef CONFIG_SL3516_ASIC
		if (status.bits.mii_rmii == GMAC_PHY_RGMII_1000)
		status.bits.mii_rmii = GMAC_PHY_RGMII_100;
		#endif
		printk("MII REG 5 (bit 5:15) = 0x%x\n", ability);
		if ((ability & 0x08)==0x08) /* 100M full duplex */
		{
			status.bits.duplex = 1;
			status.bits.speed = 1;
			printk(" 100M/Full\n");

		}
		else if ((ability & 0x04)==0x04) /* 100M half duplex */
		{
			status.bits.duplex = 0;
			status.bits.speed = 1;
			printk(" 100M/Half\n");

		}
		else if ((ability & 0x02)==0x02) /* 10M full duplex */
		{
			status.bits.duplex = 1;
			status.bits.speed = 0;
			printk(" 10M/Full\n");

		}
		else if ((ability & 0x01)==0x01) /* 10M half duplex */
		{
			status.bits.duplex = 0;
			status.bits.speed = 0;
			printk(" 10M/Half\n");

		}
	}
	if ((ability & 0x20)==0x20)
	{
		tp->flow_control_enable = 1;
		printk("Flow Control Enable.\n");
	}
	else
	{
		tp->flow_control_enable = 0;
		printk("Flow Control Disable.\n");
	}
	tp->full_duplex_status = status.bits.duplex;
	tp->speed_status = status.bits.speed;
	if (!tp->auto_nego_cfg)
	{
		status.bits.duplex = tp->full_duplex_cfg;
		status.bits.speed = tp->speed_cfg;
	}
	toe_gmac_disable_tx_rx(dev);
	mdelay(10);
	gmac_write_reg(tp->base_addr, GMAC_STATUS, status.bits32, 0x0000007f);
	toe_gmac_enable_tx_rx(dev);
}

/*----------------------------------------------------------------------
* gmac_phy_thread
*----------------------------------------------------------------------*/
static int gmac_phy_thread (void *data)
{
	struct net_device   *dev = data;
	GMAC_INFO_T *tp = dev->priv;
	unsigned long       timeout;

    daemonize("%s", dev->name); 
	allow_signal(SIGTERM);

	while (1)
	{
	    timeout = next_tick;
		do
		{
			timeout = interruptible_sleep_on_timeout (&tp->thr_wait, timeout);
		} while (!signal_pending (current) && (timeout > 0));

		if (signal_pending (current))
		{
//			spin_lock_irq(&current->sigmask_lock);
			flush_signals(current);
//			spin_unlock_irq(&current->sigmask_lock);
		}

		if (tp->time_to_die)
			break;

		// printk("%s : Polling MAC %d PHY Status...\n",__func__, tp->port_id);
		rtnl_lock ();
		if (tp->auto_nego_cfg){
			
//joel
#ifdef ALPHA_RTL8366SR_MODULE
		//gmac_get_rtl8366sr_status_callback(dev);
#elif defined(RTL8366SR)
				gmac_get_rtl8366sr_status(dev);
#else
#ifdef VITESSE_G5SWITCH
        		if((tp->port_id == GMAC_PORT1)&&(Giga_switch==1))
	        		gmac_get_switch_status(dev);
        		else
#endif
        			gmac_get_phy_status(dev); 
#endif        			
        	}
		rtnl_unlock ();
	}
	complete_and_exit (&tp->thr_exited, 0);
}
/*
This is for Auto-Negotiation
*/

#ifdef RTL8366SR
void gmac_get_rtl8366sr_status(struct net_device *dev)
{
	GMAC_INFO_T *tp = dev->priv;
	if(tp->port_id == GMAC_PORT1)/*WAN*/
	{
	GMAC_CONFIG0_T	config0,config0_mask;
		rtl8366rb_phyAbility_t    phyA, linkPartnerAb;
        uint32          nwayStatus, linkStatus; 
		GMAC_STATUS_T   status, old_status;

		status.bits32 = old_status.bits32 = gmac_read_reg(tp->base_addr, GMAC_STATUS);
    	
    	/* read PHY status register */
        rtl8366rb_getPHYLinkStatus(4, &linkStatus);
        //rtl8366s_getPHYNWayStatus(4, &nwayStatus);
        //printk("WAN link %d\n",linkStatus);
        if( (linkStatus == 1))
    	{
#if 0    	   
    	    int speed,duplex;
    	    uint32 link,txPause,rxPause,nWay;
    	    if(rtl8366s_getAsicPortLinkState(4,&speed,&duplex,&link,&txPause,&rxPause,&nWay)!=0)
	{
    	    	printk("error get link status fail\n");
    	    	return;
    	    }
    	    
            if(speed == SPD_1000M)
    	    {
    	    	//rtl8366sr_printk("Full_1000 \n");
    	    	if(status.bits.mii_rmii == 2)
    	    		status.bits.mii_rmii = 3;
    	        status.bits.duplex = 1;
    	        status.bits.speed = 2;
    	    }
    	    else
    	    {  
    	    	if(status.bits.mii_rmii == 3)
    	    		status.bits.mii_rmii = 2;
                if(speed == SPD_100M && duplex == FULL_DUPLEX)
    	    	{
    	    	        //rtl8366sr_printk("Full_100 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 1;
    	    	}
                else if(speed == SPD_100M && duplex == HALF_DUPLEX)
    	    	{
    	    	        //rtl8366sr_printk("Half_100 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 1;
    	    	}
                else if(speed == SPD_10M && duplex == FULL_DUPLEX)
    	    	{
    	    	        //rtl8366sr_printk("Full_10 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 0;
    	    	}
                else if(speed == SPD_10M && duplex == HALF_DUPLEX)
    	    	{
    	    	        //rtl8366sr_printk("Half_10 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 0;
    	    	}
	}
#else		
            /* read PHY Auto-Negotiation Link Partner Ability Register */
            rtl8366rb_getPartnerAbility(4, &linkPartnerAb);

            if(linkPartnerAb.Full_1000 == 1)
    	    {
    	    	//rtl8366sr_printk("Full_1000 \n");
    	    	if(status.bits.mii_rmii == 2)
    	    		status.bits.mii_rmii = 3;
    	        status.bits.duplex = 1;
    	        status.bits.speed = 2;
    	    }
    	    else
    	    {  
    	    	if(status.bits.mii_rmii == 3)
    	    		status.bits.mii_rmii = 2;
                if(linkPartnerAb.Full_100 == 1)
    	    	{
    	    	        //rtl8366sr_printk("Full_100 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 1;
    	    	}
                else if(linkPartnerAb.Half_100 == 1)
    	    	{
    	    	        //rtl8366sr_printk("Half_100 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 1;
    	    	}
                else if(linkPartnerAb.Full_10 == 1)
    	    	{
    	    	        //rtl8366sr_printk("Full_10 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 0;
    	    	}
                else if(linkPartnerAb.Half_10 == 1)
    	    	{
    	    	        //rtl8366sr_printk("Half_10 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 0;
    	    	}
    	    }
#endif
    	    status.bits.link = LINK_UP; /* link up */
            if(linkPartnerAb.FC == 1)
    	    {
    	        if (tp->flow_control_enable == 0)
				{
    	            config0.bits32 = 0;
    	            config0_mask.bits32 = 0;
    	            config0.bits.tx_fc_en = 1; /* enable tx flow control */
    	            config0.bits.rx_fc_en = 1; /* enable rx flow control */
    	            config0_mask.bits.tx_fc_en = 1;
    	            config0_mask.bits.rx_fc_en = 1;
    	            gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
    	           //rtl8366sr_printk("Flow Control Enable. \n");
    	        }
    	        tp->flow_control_enable = 1;
    	    }
    	    else
    	    {
    	        if (tp->flow_control_enable == 1)
    	        {
    	            config0.bits32 = 0;
    	            config0_mask.bits32 = 0;
    	            config0.bits.tx_fc_en = 0; /* disable tx flow control */
    	            config0.bits.rx_fc_en = 0; /* disable rx flow control */
    	            config0_mask.bits.tx_fc_en = 1;
    	            config0_mask.bits.rx_fc_en = 1;
    	            gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
    	            //rtl8366sr_printk("Flow Control Disable. \n");
    	        }
    	        tp->flow_control_enable = 0;
    	    }
    	
    	    if (tp->pre_phy_status == LINK_DOWN)
    	    {
    	        toe_gmac_enable_tx_rx(dev);
    	        tp->pre_phy_status = LINK_UP;
    	        netif_wake_queue(dev);
				set_bit(__LINK_STATE_START, &dev->state);
				storlink_ctl.link[tp->port_id] = 1;
				//send event to LCD tell ether up.
				key_handler(0x30); 
				printk("GMAC-%d link up\n",tp->port_id);
    	    }
    	}
    	else
    	{
    	    status.bits.link = LINK_DOWN; /* link down */
    	    if (tp->pre_phy_status == LINK_UP)
    	    {
    	        toe_gmac_disable_tx_rx(dev);
    	        tp->pre_phy_status = LINK_DOWN;
    	        netif_stop_queue(dev);
				clear_bit(__LINK_STATE_START, &dev->state);
				storlink_ctl.link[tp->port_id] = 0;
				printk("GMAC-%d link down\n",tp->port_id);
    	    }
    	}
    	tp->full_duplex_status = status.bits.duplex;
		tp->speed_status = status.bits.speed;
		if (!tp->auto_nego_cfg)
		{
			status.bits.duplex = tp->full_duplex_cfg;
			status.bits.speed = tp->speed_cfg;
		}
    	
		if (status.bits32 != old_status.bits32) 
    		gmac_write_reg(tp->base_addr, GMAC_STATUS, status.bits32, 0x0000007f);
    }
}
#endif		
/*----------------------------------------------------------------------
* gmac_get_switch_status
*----------------------------------------------------------------------*/
#ifdef VITESSE_G5SWITCH
void gmac_get_switch_status(struct net_device *dev)
{
	GMAC_INFO_T *tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;
	unsigned int	switch_port_id;
	int get_link=0;
 	
	get_link = Get_Set_port_status();
	if(get_link){				// link
		if(ever_dwon){
			ever_dwon = 0;
			toe_gmac_enable_tx_rx(dev);
			netif_wake_queue(dev);
			set_bit(__LINK_STATE_START, &dev->state);
		}
	}
	else{					// all down
		//printk("All link down\n");
		ever_dwon=1;
		netif_stop_queue(dev);
		toe_gmac_disable_tx_rx(dev);
		clear_bit(__LINK_STATE_START, &dev->state);
	}
	
	if ( tp->port_id == 1 )
		switch_port_id = 0;
	if (get_link)
	{
		storlink_ctl.link[switch_port_id] = 1;
	}
	else
	{
		storlink_ctl.link[switch_port_id] = 0;
	}
	if (storlink_ctl.napi == 2)	//For IXIA 2544 throughput test 
		{
			if (tp->flow_control_enable == 1)
			{
				config0.bits32 = 0;
				config0_mask.bits32 = 0;
				config0.bits.tx_fc_en = 0; /* disable tx flow control */
				config0.bits.rx_fc_en = 0; /* disable rx flow control */
				config0_mask.bits.tx_fc_en = 1;
				config0_mask.bits.rx_fc_en = 1;
				gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
				printk("Disable SWITCH Flow Control...\n");
			}
				tp->flow_control_enable = 0;
		}
		else
		{
			if (tp->flow_control_enable == 0)
			{
				config0.bits32 = 0;
				config0_mask.bits32 = 0;
				config0.bits.tx_fc_en = 1; /* enable tx flow control */
				config0.bits.rx_fc_en = 1; /* enable rx flow control */
				config0_mask.bits.tx_fc_en = 1;
				config0_mask.bits.rx_fc_en = 1;
				gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
				printk("Enable SWITCH Flow Control...\n");
			}
			tp->flow_control_enable = 1;
		} 
		
	return ;
	
}
#endif

/*----------------------------------------------------------------------
* gmac_get_phy_status
*----------------------------------------------------------------------*/
void gmac_get_phy_status(struct net_device *dev)
{
	GMAC_INFO_T *tp = dev->priv;
	GMAC_CONFIG0_T	config0,config0_mask;
	GMAC_STATUS_T   status, old_status;
	unsigned int    reg_val,ability,wan_port_id;
#ifdef PHY_WORKAROUND
#ifdef CONFIG_SL351X_IPSEC
	unsigned int	re_poll_counter = 5;	// updated by Wen
#else
	unsigned int	re_poll_counter = 3;
#endif
#endif	

	old_status.bits32 = status.bits32 = gmac_read_reg(tp->base_addr, GMAC_STATUS);

#ifdef PHY_WORKAROUND
phy_status:
#endif
	/* read PHY status register */
	reg_val = mii_read(tp->phy_addr,0x01);
	if ((reg_val & 0x0024) == 0x0024) /* link is established and auto_negotiate process completed */
	{
		ability = (mii_read(tp->phy_addr,0x05) & 0x05E0) >> 5;
		/* read PHY Auto-Negotiation Link Partner Ability Register */
		#ifdef CONFIG_SL3516_ASIC
		reg_val = mii_read(tp->phy_addr,10);
#ifdef STORLINK_PHY			
		link_status = 1;
		mdelay(10);
		if ((reg_val & 0x3800) == 0x3800)	//local phy 1G master
#else
		if ((reg_val & 0x0800) == 0x0800)
#endif
		{
			status.bits.duplex = 1;
			status.bits.speed = 2;
			if (status.bits.mii_rmii == GMAC_PHY_RGMII_100)
			status.bits.mii_rmii = GMAC_PHY_RGMII_1000;
		}
#ifdef STORLINK_PHY			
		else if ((reg_val & 0x3400) == 0x3400)	//local phy 1G slave
#else		
		else if ((reg_val & 0x0400) == 0x0400)
#endif		
		{
			status.bits.duplex = 0;
			status.bits.speed = 2;
			if (status.bits.mii_rmii == GMAC_PHY_RGMII_100)
			status.bits.mii_rmii = GMAC_PHY_RGMII_1000;
		}
		else
		#endif
		{
			#ifdef CONFIG_SL3516_ASIC
			if (status.bits.mii_rmii == GMAC_PHY_RGMII_1000)
			status.bits.mii_rmii = GMAC_PHY_RGMII_100;
			#endif
			if ((ability & 0x08)==0x08) /* 100M full duplex */
			{
				status.bits.duplex = 1;
				status.bits.speed = 1;
			}
			else if ((ability & 0x04)==0x04) /* 100M half duplex */
			{
				status.bits.duplex = 0;
				status.bits.speed = 1;
			}
			else if ((ability & 0x02)==0x02) /* 10M full duplex */
			{
				status.bits.duplex = 1;
				status.bits.speed = 0;
			}
			else if ((ability & 0x01)==0x01) /* 10M half duplex */
			{
				status.bits.duplex = 0;
				status.bits.speed = 0;
			}
		}
		status.bits.link = LINK_UP; /* link up */
		if(Giga_switch==1)
		{
				wan_port_id = 1;
				storlink_ctl.link[ wan_port_id] = 1;
		}
		else
		{
				storlink_ctl.link[ tp->port_id] = 1;
		}	
		if ((ability & 0x20)==0x20)
		{
			if (tp->flow_control_enable == 0)
			{
				config0.bits32 = 0;
				config0_mask.bits32 = 0;
				config0.bits.tx_fc_en = 1; /* enable tx flow control */
				config0.bits.rx_fc_en = 1; /* enable rx flow control */
				config0_mask.bits.tx_fc_en = 1;
				config0_mask.bits.rx_fc_en = 1;
				gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
				printk("GMAC-%d Flow Control Enable.\n", tp->port_id);
			}
			tp->flow_control_enable = 1;
		}
		else
		{
			if (tp->flow_control_enable == 1)
			{
				config0.bits32 = 0;
				config0_mask.bits32 = 0;
				config0.bits.tx_fc_en = 0; /* disable tx flow control */
				config0.bits.rx_fc_en = 0; /* disable rx flow control */
				config0_mask.bits.tx_fc_en = 1;
				config0_mask.bits.rx_fc_en = 1;
				gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
				printk("GMAC-%d Flow Control Disable.\n", tp->port_id);
			}
			tp->flow_control_enable = 0;
		}

		if (tp->pre_phy_status == LINK_DOWN)
		{
			printk("GMAC-%d LINK_UP......\n",tp->port_id);
			tp->pre_phy_status = LINK_UP;
			netif_carrier_on(dev);
		}
	}
	else	//link down or auto negotiate not complete yet.
	{
#ifdef STORLINK_PHY
		if ((link_status == 1) && (tp->port_id == 0))
		{
			mii_write(tp->phy_addr,0x0,0x8000);
			mii_write(tp->phy_addr,16,0x8000);
			mii_write(tp->phy_addr,17,0x4040);
			mii_write(tp->phy_addr,18,0x1a38);
			mii_write(tp->phy_addr,16,0x4000);
			mii_write(tp->phy_addr,22,0x02d4);
			mii_write(tp->phy_addr,23,0x02d4);
			mii_write(tp->phy_addr,16,0x0000);
			mii_write(tp->phy_addr,0x04,0x05e1); /* advertisement STORM phy to 100M full duplex, pause capable on */
			mii_write(tp->phy_addr,0x09,0x0f00); /* advertise no 1000M full/half duplex */
			printk("\nPHY reset !!\n");
			link_status = 0;
			mdelay(100);
		}
#else
#ifdef PHY_WORKAROUND		
		re_poll_counter--;
		if (re_poll_counter == 0)
			goto phy_status_down;
		else
		{
			goto phy_status;
		}
phy_status_down:	
#endif
#endif
		
		status.bits.link = LINK_DOWN; /* link down */
		if(Giga_switch == 1)
		{
				wan_port_id = 1;
				storlink_ctl.link[ wan_port_id] = 0;
		}
		else
		{
				storlink_ctl.link[ tp->port_id] = 0;
		}
		if (tp->pre_phy_status == LINK_UP)
		{
			printk("GMAC-%d LINK_Down......\n",tp->port_id);
			tp->pre_phy_status = LINK_DOWN;
			netif_carrier_off(dev);
		}
	}

	tp->full_duplex_status = status.bits.duplex;
	tp->speed_status = status.bits.speed;
	if (!tp->auto_nego_cfg)
	{
		status.bits.duplex = tp->full_duplex_cfg;
		status.bits.speed = tp->speed_cfg;
	}

	if (old_status.bits32 != status.bits32)
	{
		netif_stop_queue(dev);
		toe_gmac_disable_tx_rx(dev);
		clear_bit(__LINK_STATE_START, &dev->state);
		printk("GMAC-%d Change Status Bits 0x%x-->0x%x\n",tp->port_id, old_status.bits32, status.bits32);
		mdelay(10); // let GMAC consume packet
		gmac_write_reg(tp->base_addr, GMAC_STATUS, status.bits32, 0x0000007f);
		if (status.bits.link == LINK_UP)
		{
			toe_gmac_enable_tx_rx(dev);
			netif_wake_queue(dev);
			set_bit(__LINK_STATE_START, &dev->state);
		}
	}
}

/***************************************/
/* define GPIO module base address     */
/***************************************/
#define GPIO_BASE_ADDR  (IO_ADDRESS(SL2312_GPIO_BASE))
#define GPIO_BASE_ADDR1  (IO_ADDRESS(SL2312_GPIO_BASE1))

/* define GPIO pin for MDC/MDIO */
#ifdef CONFIG_SL3516_ASIC
#ifdef CONFIG_RTL8366SR_PHY
#define H_MDC_PIN           22	//depend on hardware layout
#define H_MDIO_PIN          21
//#define G_MDC_PIN           0	//for new Cameo board 22, old is 0
#define G_MDC_PIN           22	//for new Cameo board 22, old is 0
#define G_MDIO_PIN          21
#else	//CONFIG_RTL8366SR_PHY
#define H_MDC_PIN           22
#define H_MDIO_PIN          21
#define G_MDC_PIN           22
#define G_MDIO_PIN          21
#endif	//CONFIG_RTL8366SR_PHY
#define H_MDC_PIN_c         21	//3516A3 Vitess 
#define H_MDIO_PIN_c        22	//3516A3 Vitess
#define G_MDC_PIN_c         21	//3516A3 Vitess
#define G_MDIO_PIN_c        22	//3516A3 Vitess
#else	//CONFIG_SL3516_ASIC
#define H_MDC_PIN           3
#define H_MDIO_PIN          2
#define G_MDC_PIN           0
#define G_MDIO_PIN          1
#endif	//CONFIG_SL3516_ASIC

//#define GPIO_MDC             0x80000000
//#define GPIO_MDIO            0x00400000

static unsigned int GPIO_MDC = 0;
static unsigned int GPIO_MDIO = 0;
static unsigned int GPIO_MDC_PIN = 0;
static unsigned int GPIO_MDIO_PIN = 0;

// For PHY test definition!!
#define LPC_EECK		0x02
#define LPC_EDIO		0x04
#define LPC_GPIO_SET		3
#define LPC_BASE_ADDR		IO_ADDRESS(IT8712_IO_BASE)
#define inb_gpio(x)		inb(LPC_BASE_ADDR + IT8712_GPIO_BASE + x)
#define outb_gpio(x, y)		outb(y, LPC_BASE_ADDR + IT8712_GPIO_BASE + x)

enum GPIO_REG
{
    GPIO_DATA_OUT   = 0x00,
    GPIO_DATA_IN    = 0x04,
    GPIO_PIN_DIR    = 0x08,
    GPIO_BY_PASS    = 0x0c,
    GPIO_DATA_SET   = 0x10,
    GPIO_DATA_CLEAR = 0x14,
};
/***********************/
/*    MDC : GPIO[31]   */
/*    MDIO: GPIO[22]   */
/***********************/

/***************************************************
* All the commands should have the frame structure:
*<PRE><ST><OP><PHYAD><REGAD><TA><DATA><IDLE>
****************************************************/

/*****************************************************************
* Inject a bit to NWay register through CSR9_MDC,MDIO
*******************************************************************/
void mii_serial_write(char bit_MDO) // write data into mii PHY
{
#ifdef CONFIG_SL2312_LPC_IT8712
	unsigned char iomode,status;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode |= (LPC_EECK|LPC_EDIO) ;				// Set EECK,EDIO,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	if(bit_MDO)
	{
		status = inb_gpio( LPC_GPIO_SET);
		status |= LPC_EDIO ;		//EDIO high
		outb_gpio(LPC_GPIO_SET, status);
	}
	else
	{
		status = inb_gpio( LPC_GPIO_SET);
		status &= ~(LPC_EDIO) ;		//EDIO low
		outb_gpio(LPC_GPIO_SET, status);
	}

	status |= LPC_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(LPC_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

#else
    unsigned int addr;
    unsigned int value;

    addr = GPIO_BASE_ADDR + GPIO_PIN_DIR;
    value = readl(addr) | GPIO_MDC | GPIO_MDIO; /* set MDC/MDIO Pin to output */
    writel(value,addr);
    if(bit_MDO)
    {
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDIO,addr); /* set MDIO to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDC,addr); /* set MDC to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDC,addr); /* set MDC to 0 */
    }
    else
    {
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDIO,addr); /* set MDIO to 0 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_SET);
        writel(GPIO_MDC,addr); /* set MDC to 1 */
        addr = (GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
        writel(GPIO_MDC,addr); /* set MDC to 0 */
    }

#endif
}

/**********************************************************************
* read a bit from NWay register through CSR9_MDC,MDIO
***********************************************************************/
unsigned int mii_serial_read(void) // read data from mii PHY
{
#ifdef CONFIG_SL2312_LPC_IT8712
  	unsigned char iomode,status;
	unsigned int value ;

	iomode = LPCGetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET);
	iomode &= ~(LPC_EDIO) ;		// Set EDIO input
	iomode |= (LPC_EECK) ;		// Set EECK,EECS output
	LPCSetConfig(LDN_GPIO, 0xc8 + LPC_GPIO_SET, iomode);

	status = inb_gpio( LPC_GPIO_SET);
	status |= LPC_EECK ;		//EECK high
	outb_gpio(LPC_GPIO_SET, status);

	status &= ~(LPC_EECK) ;		//EECK low
	outb_gpio(LPC_GPIO_SET, status);

	value = inb_gpio( LPC_GPIO_SET);

	value = value>>2 ;
	value &= 0x01;

	return value ;

#else
    unsigned int *addr;
    unsigned int value;

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_PIN_DIR);
    value = readl(addr) & ~GPIO_MDIO; //0xffbfffff;   /* set MDC to output and MDIO to input */
    writel(value,addr);

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_SET);
    writel(GPIO_MDC,addr); /* set MDC to 1 */
    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_CLEAR);
    writel(GPIO_MDC,addr); /* set MDC to 0 */

    addr = (unsigned int *)(GPIO_BASE_ADDR + GPIO_DATA_IN);
    value = readl(addr);
    value = (value & (1<<GPIO_MDIO_PIN)) >> GPIO_MDIO_PIN;
    return(value);

#endif
}

/***************************************
* preamble + ST
***************************************/
void mii_pre_st(void)
{
    unsigned char i;

    for(i=0;i<32;i++) // PREAMBLE
        mii_serial_write(1);
    mii_serial_write(0); // ST
    mii_serial_write(1);
}
 
/******************************************
* Read MII register
* phyad -> physical address
* regad -> register address
***************************************** */
unsigned int mii_read(unsigned char phyad,unsigned char regad)
{
    unsigned int i,value;
    unsigned int bit;
    
     local_irq_disable();
#ifdef CONFIG_RTL8366SR_PHY
	GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
    GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
#else	
//	if (c_GPIO)
//	{
//    	if (phyad == GPHY_ADDR)
//    	{
///    	    GPIO_MDC_PIN = G_MDC_PIN_c;   /* assigned MDC pin for giga PHY */
//    	    GPIO_MDIO_PIN = G_MDIO_PIN_c; /* assigned MDIO pin for giga PHY */
//    	}
//    	else
//    	{    
//    	    GPIO_MDC_PIN = H_MDC_PIN_c;   /* assigned MDC pin for 10/100 PHY */
//    	    GPIO_MDIO_PIN = H_MDIO_PIN_c; /* assigned MDIO pin for 10/100 PHY */
//    	}
//	}
//	else
	{
    	if (phyad == GPHY_ADDR)
    	{
    	    GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
    	    GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
    	}
    	else
    	{    
    	    GPIO_MDC_PIN = H_MDC_PIN;   /* assigned MDC pin for 10/100 PHY */
    	    GPIO_MDIO_PIN = H_MDIO_PIN; /* assigned MDIO pin for 10/100 PHY */
    	}
	}
#endif  //CONFIG_RTL8366SR_PHY  	
    GPIO_MDC = (1<<GPIO_MDC_PIN);   
    GPIO_MDIO = (1<<GPIO_MDIO_PIN); 

    mii_pre_st(); // PRE+ST
    mii_serial_write(1); // OP
    mii_serial_write(0);

    for (i=0;i<5;i++) { // PHYAD
        bit= ((phyad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    for (i=0;i<5;i++) { // REGAD
        bit= ((regad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    mii_serial_read(); // TA_Z
//    mii_serial_read(); // TA_0 turn around
//    if((bit=mii_serial_read()) !=0 ) // TA_0
//    {
//        return(0);
//    }
    value=0;
    for (i=0;i<16;i++) { // READ DATA
        bit=mii_serial_read();
        value += (bit<<(15-i)) ;
    }

    mii_serial_write(0); // dumy clock
    mii_serial_write(0); // dumy clock
	
	//printk("%s: phy_addr=0x%x reg_addr=0x%x value=0x%x \n",__func__,phyad,regad,value);
	local_irq_enable();
    return(value);
}

/******************************************
* Write MII register
* phyad -> physical address
* regad -> register address
* value -> value to be write
***************************************** */
void mii_write(unsigned char phyad,unsigned char regad,unsigned int value)
{
    unsigned int i;
    char bit;
#ifndef STORLINK_PHY
	printk("%s: phy_addr=0x%x reg_addr=0x%x value=0x%x \n",__func__,phyad,regad,value);
#endif	
#ifdef CONFIG_RTL8366SR_PHY
	GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
    GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
#else
//	if (c_GPIO)
//	{
//    	if (phyad == GPHY_ADDR)
//    	{
///    	    GPIO_MDC_PIN = G_MDC_PIN_c;   /* assigned MDC pin for giga PHY */
//    	    GPIO_MDIO_PIN = G_MDIO_PIN_c; /* assigned MDIO pin for giga PHY */
//    	}
//    	else
//    	{    
//    	    GPIO_MDC_PIN = H_MDC_PIN_c;   /* assigned MDC pin for 10/100 PHY */
//    	    GPIO_MDIO_PIN = H_MDIO_PIN_c; /* assigned MDIO pin for 10/100 PHY */
//    	}
//	}
//	else
	{
    	if (phyad == GPHY_ADDR)
    	{
    	    GPIO_MDC_PIN = G_MDC_PIN;   /* assigned MDC pin for giga PHY */
    	    GPIO_MDIO_PIN = G_MDIO_PIN; /* assigned MDIO pin for giga PHY */
    	}
    	else
    	{    
    	    GPIO_MDC_PIN = H_MDC_PIN;   /* assigned MDC pin for 10/100 PHY */
    	    GPIO_MDIO_PIN = H_MDIO_PIN; /* assigned MDIO pin for 10/100 PHY */
    	}
	}
#endif   //CONFIG_RTL8366SR_PHY	
    GPIO_MDC = (1<<GPIO_MDC_PIN);   
    GPIO_MDIO = (1<<GPIO_MDIO_PIN); 

    mii_pre_st(); // PRE+ST
    mii_serial_write(0); // OP
    mii_serial_write(1);
    for (i=0;i<5;i++) { // PHYAD
        bit= ((phyad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }

    for (i=0;i<5;i++) { // REGAD
        bit= ((regad>>(4-i)) & 0x01) ? 1 :0 ;
        mii_serial_write(bit);
    }
    mii_serial_write(1); // TA_1
    mii_serial_write(0); // TA_0

    for (i=0;i<16;i++) { // OUT DATA
        bit= ((value>>(15-i)) & 0x01) ? 1 : 0 ;
        mii_serial_write(bit);
    }
    mii_serial_write(0); // dumy clock
    mii_serial_write(0); // dumy clock
}

/*----------------------------------------------------------------------
* gmac_set_rx_mode
*----------------------------------------------------------------------*/
static void gmac_set_rx_mode(struct net_device *dev)
{
    GMAC_RX_FLTR_T      filter;
	unsigned int        mc_filter[2];	/* Multicast hash filter */
    int                 bit_nr;
	unsigned int        i;
	GMAC_INFO_T 		*tp = dev->priv;

//    printk("%s : dev->flags = %x \n",__func__,dev->flags);
//    dev->flags |= IFF_ALLMULTI;  /* temp */
    filter.bits32 = 0;
    filter.bits.error = 0;
	if (dev->flags & IFF_PROMISC)
	{
	    filter.bits.error = 1;
        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	}
	else if (dev->flags & IFF_ALLMULTI)
	{
//        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0xffffffff;
	}
	else
	{
		struct dev_mc_list *mclist;

//        filter.bits.promiscuous = 1;
        filter.bits.broadcast = 1;
        filter.bits.multicast = 1;
        filter.bits.unicast = 1;
		mc_filter[1] = mc_filter[0] = 0;
		for (i = 0, mclist = dev->mc_list; mclist && i < dev->mc_count;i++, mclist = mclist->next)
		{
            bit_nr = ether_crc(ETH_ALEN,mclist->dmi_addr) & 0x0000003f;
            if (bit_nr < 32)
            {
                mc_filter[0] = mc_filter[0] | (1<<bit_nr);
            }
            else
            {
                mc_filter[1] = mc_filter[1] | (1<<(bit_nr-32));
            }
		}
	}
    gmac_write_reg(tp->base_addr,GMAC_RX_FLTR,filter.bits32,0xffffffff);  //chech base address!!!
    gmac_write_reg(tp->base_addr,GMAC_MCAST_FIL0,mc_filter[0],0xffffffff);
    gmac_write_reg(tp->base_addr,GMAC_MCAST_FIL1,mc_filter[1],0xffffffff);
    return;
}

#ifdef CONFIG_SL_NAPI
/*----------------------------------------------------------------------
* gmac_rx_poll
*----------------------------------------------------------------------*/
static inline int gmac_rx_poll(struct net_device *dev, int *budget) 
{ 
	TOE_INFO_T			*toe;
    GMAC_RXDESC_T   	*curr_desc;
	struct sk_buff 		*skb;
    DMA_RWPTR_T			rwptr;
	unsigned int 		pkt_size;
	unsigned int        desc_count;
	unsigned int        good_frame, chksum_status, rx_status;
	int                 rx_pkts_num = 0,max_pkts_num,classq_pkts_num;
	int                 quota = min(dev->quota, *budget);
	GMAC_INFO_T			*tp = (GMAC_INFO_T *)dev->priv;
	unsigned long		flags;
#ifdef CONFIG_SL351x_RTLDMZ	 	
	int                 si;
    unsigned short      vid_skb = 0;
	struct vlan_hdr *vhdr;
#endif					
 
	if (((tp->port_id == 0) && (tp->poll_bits & DEFAULT_Q0_INT_BIT)) ||
		((tp->port_id == 1) && (tp->poll_bits & DEFAULT_Q1_INT_BIT)))
	{
		struct net_device_stats *isPtr = (struct net_device_stats *)&tp->ifStatics;
	
		rwptr.bits32 = readl(&tp->default_qhdr->word1);
#if 0
		if (rwptr.bits.rptr != tp->rx_rwptr.bits.rptr)
		{ 
			mac_stop_txdma((struct net_device *)tp->dev);
			printk("Default Queue HW RD ptr (0x%x) != SW RD Ptr (0x%x)\n",
					rwptr.bits32, tp->rx_rwptr.bits.rptr);
			while(1);
		}
#endif
		toe = (TOE_INFO_T *)&toe_private_data;
		while ((rwptr.bits.rptr != rwptr.bits.wptr) && (rx_pkts_num < quota))
		{	       
    		curr_desc = (GMAC_RXDESC_T *)tp->default_desc_base + rwptr.bits.rptr;
#ifdef GMAC_DEBUG
			tp->default_q_cnt++;
#endif
			tp->rx_curr_desc = (unsigned int)curr_desc;
    		rx_status = curr_desc->word0.bits.status;
	    	chksum_status = curr_desc->word0.bits.chksum_status;
#ifdef GMAC_DEBUG
	    	tp->rx_status_cnt[rx_status]++;
			tp->rx_chksum_cnt[chksum_status]++;
#endif
			pkt_size = curr_desc->word1.bits.byte_count;  /*total byte count in a frame*/
			desc_count = curr_desc->word0.bits.desc_count; /* get descriptor count per frame */
			good_frame=1;
			if ((curr_desc->word0.bits32 & (GMAC_RXDESC_0_T_derr | GMAC_RXDESC_0_T_perr))
				|| (pkt_size < 60)
				|| (chksum_status & 0x4)
				|| rx_status )
			{
				good_frame = 0;
#ifdef GMAC_DEBUG
				if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_derr)
					printk("%s::derr (GMAC-%d)!!!\n", __func__, tp->port_id);
				if (curr_desc->word0.bits32 & GMAC_RXDESC_0_T_perr)
					printk("%s::perr (GMAC-%d)!!!\n", __func__, tp->port_id);
#endif
				if (rx_status)
				{
					if (rx_status == 4 || rx_status == 7)
						isPtr->rx_crc_errors++;
#ifdef GMAC_DEBUG
					printk("%s::Status=%d (GMAC-%d)!!!\n", __func__, rx_status, tp->port_id);
#endif
				}
#ifdef SL351x_GMAC_WORKAROUND
				else if (pkt_size < 60)
				{
					if (tp->short_frames_cnt < GMAC_SHORT_FRAME_THRESHOLD)
						tp->short_frames_cnt++;
					if (tp->short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
					{
						GMAC_CONFIG0_T config0;
						config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
						config0.bits.dis_rx = 1;
						writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
						config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
						config0.bits.dis_rx = 1;
						writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
					}
				}
#endif
#ifdef GMAC_DEBUG
				if (chksum_status)
					printk("%s::Checksum Status=%d (GMAC-%d)!!!\n", __func__, chksum_status, tp->port_id);
#endif
				skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
#ifdef GMAC_DEBUG_U
				if (gmac_dump_rxpkt)
				{
					printk("******** GMAC %d Rx %d Bytes: BAD Packet ********\n",tp->port_id, pkt_size);
					printk("Checksum Status=%d, L3:0x%x, L4:0x%x, L7:0x%x skb->data = 0x%p\n", 
							chksum_status, curr_desc->word3.bits.l3_offset, curr_desc->word3.bits.l4_offset, curr_desc->word3.bits.l7_offset
							,skb->data);
					dm_byte((u32)skb->head, (pkt_size > 256) ? 256 : pkt_size);
					printk("\n");
				}
#endif
				dev_kfree_skb_any(skb);
			}
			if (good_frame)
			{
#ifdef GMAC_DEBUG
				if (curr_desc->word0.bits.drop)
					printk("%s::Drop (GMAC-%d)!!!\n", __func__, tp->port_id);
				if (chksum_status)
					printk("%s::Checksum Status=%d (GMAC-%d)!!!\n", __func__, chksum_status, tp->port_id);
#endif
#ifdef SL351x_GMAC_WORKAROUND
				if (tp->short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
				{
					GMAC_CONFIG0_T config0;
					config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
					config0.bits.dis_rx = 0;
					writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
					config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
					config0.bits.dis_rx = 0;
					writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
				}
				tp->short_frames_cnt = 0;
#endif
				/* get frame information from the first descriptor of the frame */
				isPtr->rx_packets++;
				//consistent_sync((void *)__va(curr_desc->word2.buf_adr), pkt_size, PCI_DMA_FROMDEVICE);					
				skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
#ifdef CONFIG_SL351x_RTLDMZ	 	
				vhdr = (struct vlan_hdr *)(skb->data);
#endif	
				tp->curr_rx_skb = skb;
		//		curr_desc->word2.buf_adr = 0;
				skb_reserve (skb, RX_INSERT_BYTES);	/* 2 byte align the IP fields. */
#ifdef GMAC_DEBUG
			if ((skb->tail+pkt_size) > skb->end )
				printk("%s::-->Fail skb->len=%d,pkt_size= %d,skb->head=0x%x,skb->tail= 0x%x, skb->end= 0x%x\n", __func__, skb->len, pkt_size,skb->head,skb->tail,skb->end);
#endif
				skb_put(skb, pkt_size);
				skb->dev = dev;	
				
				
#ifdef ALPHA_RTL8366SR_MODULE				
				if(igmp_snoop_input 
					   && tp->port_id == GMAC_PORT0 /*LAN */
					   && (skb->data[0]&0x01) == 0x01 
					   && ((struct ethhdr*)skb->data)->h_proto == htons(0x0800))
				{
					igmp_snoop_input(skb,-1);
				}
#endif
#ifdef ALPHA_IGMP_SNOOP
#if 1//hendry for enable/disable snoop option
				if(g_igmp_snoop_enable)
#endif
				{
					if(tp->port_id == GMAC_PORT0 /*LAN */
					   && skb->data[0]&0x01== 0x01 
					   && ((struct ethhdr*)skb->data)->h_proto == htons(0x0800))
					{
						igmp_snoop_input(skb);
					}
				}
#endif
	
#ifdef CONFIG_SL351x_RTLDMZ	 
			if (tp->port_id == GMAC_PORT0)	
			{
   				if((*(skb->data + 12)) == 0x81)
   		 		{
   		     		vid_skb = (*(skb->data + 15))|(((*(skb->data + 14)) & 0xFF )<<8);
#ifdef DUAL_BAND_VIRTUAL_INTERFACE				
					if(vid_skb == 0x2002)		//if incoming packet from LAN
					{
						dev=GMACVIF0;
                      	skb->dev = GMACVIF0;	
                    }
					if(vid_skb == 0x2003)		//if incoming packet from DMZ 
					{
						dev=GMACVIF1;
				      	skb->dev = GMACVIF1;    
				    }
#else
					skb->dev = dev;	
#endif		   		     
   		    		for(si = 0; si < 3; si++)
   		         	memcpy((skb->data+12-4*si), (skb->data +12-4*(si + 1)), VLAN_HLEN);
   		     		skb_pull(skb,VLAN_HLEN);
					skb_postpull_rcsum(skb, vhdr, VLAN_HLEN);
   		 		}
   		 	}
   		 	else	
			    skb->dev = dev;	
#ifdef CONFIG_SL351x_NAT
#ifdef SKIP_NAT_CFG
			nat_cfg.enabled = 1; 
#endif	//SKIP_NAT_CFG		
#endif	//CONFIG_SL351x_NAT				    	
#endif	//CONFIG_SL351x_RTLDMZ	
				if (chksum_status == RX_CHKSUM_IP_UDP_TCP_OK)
				{
					skb->ip_summed = CHECKSUM_UNNECESSARY;
#ifdef CONFIG_SL351x_NAT
					if (nat_cfg.enabled && curr_desc->word3.bits.l3_offset && curr_desc->word3.bits.l4_offset)
					{
//						struct iphdr	*ip_hdr;
//						ip_hdr = (struct iphdr *)&(skb->data[curr_desc->word3.bits.l3_offset]);
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
					}
#endif
#ifdef CONFIG_SL351X_BR
					sl351x_br_input(skb,tp->port_id);
#endif
//					skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
#if 0
#ifdef CONFIG_SL351x_RXTOE
					if (storlink_ctl.rx_max_pktsize) {
						struct iphdr	*ip_hdr;
						struct tcphdr	*tcp_hdr;
						int ip_hdrlen;
 					
 						ip_hdr = (struct iphdr*)&(skb->data[0]);
						if ((skb->protocol == __constant_htons(ETH_P_IP)) && 
						   ((ip_hdr->protocol & 0x00ff) == IPPROTO_TCP)) {
							ip_hdrlen = ip_hdr->ihl << 2;
							tcp_hdr = (struct tcphdr*)&(skb->data[ip_hdrlen]);
							if (tcp_hdr->syn) {
								struct toe_conn* connection = init_toeq(ip_hdr->version,
									ip_hdr, tcp_hdr, toe, &(skb->data[0]) - 14);
								TCP_SKB_CB(skb)->connection = connection;
								//	hash_dump_entry(TCP_SKB_CB(skb)->connection->hash_entry_index);
								//		printk("%s::skb data %x, conn %x, mode %x\n",
								//			__func__, skb->data, connection, connection->mode);
							}
						}
					}
#endif
#endif
				}
				else if (chksum_status == RX_CHKSUM_IP_OK_ONLY)
				{
					skb->ip_summed = CHECKSUM_UNNECESSARY;
#ifdef CONFIG_SL351x_NAT
					if (nat_cfg.enabled && curr_desc->word3.bits.l3_offset && curr_desc->word3.bits.l4_offset)
					{
						struct iphdr	*ip_hdr;
						ip_hdr = (struct iphdr *)&(skb->data[curr_desc->word3.bits.l3_offset]);
						if (ip_hdr->protocol == IPPROTO_UDP)
						{
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
						}
						else if (ip_hdr->protocol == IPPROTO_GRE)
						{
#ifdef CONFIG_SL351x_RTLDMZ	
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset,vid_skb);
#else					
					sl351x_nat_input(skb, tp->port_id, (void *)curr_desc->word3.bits.l3_offset,
					  	(void *)curr_desc->word3.bits.l4_offset);
#endif	
						}
					}
#endif
#ifdef CONFIG_SL351X_BR
					sl351x_br_input(skb,tp->port_id);
#endif	
//					skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
				}
				else
				{
#ifdef CONFIG_SL351X_BR
					sl351x_br_input(skb,tp->port_id);
#endif
//					skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
				}
#ifdef GMAC_DEBUG_U
				if (gmac_dump_rxpkt)
				{
					printk("******** GMAC %d Rx %d Bytes: ********\n",tp->port_id, pkt_size);
					printk("Checksum Status=%d, L3:0x%x, L4:0x%x, L7:0x%x, skb->data =0x%p\n", 
							chksum_status, curr_desc->word3.bits.l3_offset, curr_desc->word3.bits.l4_offset, curr_desc->word3.bits.l7_offset,
							skb->data);
					dm_byte((u32)skb->head, (pkt_size > 256) ? 512 : pkt_size);
			 		printk("\n");
				}
#endif			
				skb->protocol = eth_type_trans(skb,dev); /* set skb protocol */
				netif_receive_skb(skb); // socket rx for NAPI

				dev->last_rx = jiffies;
				isPtr->rx_bytes += pkt_size;
			}
			// advance one for Rx default Q 0/1
			rwptr.bits.rptr = RWPTR_ADVANCE_ONE(rwptr.bits.rptr, tp->default_desc_num);
			SET_RPTR(&tp->default_qhdr->word1, rwptr.bits.rptr);
			tp->rx_rwptr.bits32 = rwptr.bits32;
			rx_pkts_num++;
	//		toe_gmac_fill_free_q();
		}
		toe_gmac_fill_free_q();
		quota -= rx_pkts_num;
	} 
	max_pkts_num = rx_pkts_num;

#ifdef CONFIG_SL351X_IPSEC
	// Handle classfication queue for ipsec VPN
	classq_pkts_num=0;

	if (Giga_switch)
	{
		if (((tp->port_id == 0) && (tp->poll_bits & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID))) ||
				((tp->port_id == 1) && (tp->poll_bits & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID)))) {
			classq_pkts_num = ipsec_handle_class_queue(dev,tp, 10<quota?10:quota);
		}
	}
	else 
	{
		if (((tp->port_id == 0) && (tp->poll_bits & CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID))) ||
				((tp->port_id == 1) && (tp->poll_bits & CLASS_RX_INT_BIT(IPSEC_INBOUND_QID)))) {
			classq_pkts_num = ipsec_handle_class_queue(dev,tp, 10<quota?10:quota);
		}
	}
//		classq_pkts_num = ipsec_handle_class_queue(dev,tp, 16<quota?16:quota);

	max_pkts_num = rx_pkts_num > classq_pkts_num ? rx_pkts_num : classq_pkts_num;
//	max_pkts_num = rx_pkts_num + classq_pkts_num;
#endif

	dev->quota -= max_pkts_num;
	*budget -= max_pkts_num;

	//if (rwptr.bits.rptr == rwptr.bits.wptr)
	if(max_pkts_num == 0)
	{
		unsigned int data32;

		/* Receive descriptor is empty now */
		netif_rx_complete(dev);
		local_irq_save(flags);

		// enable GMAC-0 rx interrupt
		// class-Q & TOE-Q are implemented in future
		data32 = readl(TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);
		if (tp->port_id == 0) {
			if (Giga_switch) {
#ifdef CONFIG_SL351X_IPSEC
				data32 |= (DEFAULT_Q0_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID));

				tp->poll_bits &= ~(DEFAULT_Q0_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID));
#else
				data32 |= DEFAULT_Q0_INT_BIT;
				tp->poll_bits &= ~DEFAULT_Q0_INT_BIT;
#endif
			}
			else {
#ifdef CONFIG_SL351X_IPSEC
				data32 |= (DEFAULT_Q0_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID));
				tp->poll_bits &= ~(DEFAULT_Q0_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID));
#else
				data32 |= DEFAULT_Q0_INT_BIT;
				tp->poll_bits &= ~DEFAULT_Q0_INT_BIT;
#endif
			}
		}
		else {
			if (Giga_switch) {
#ifdef CONFIG_SL351X_IPSEC
				data32 |= (DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID));
				tp->poll_bits &= ~(DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_OUTBOUND_QID));
#else
				data32 |= DEFAULT_Q1_INT_BIT;
				tp->poll_bits &= ~DEFAULT_Q1_INT_BIT;
#endif
			}
			else {
#ifdef CONFIG_SL351X_IPSEC
				data32 |= (DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID));
				tp->poll_bits &= ~(DEFAULT_Q1_INT_BIT | CLASS_RX_INT_BIT(IPSEC_INBOUND_QID));
#else
				data32 |= DEFAULT_Q1_INT_BIT;
				tp->poll_bits &= ~DEFAULT_Q1_INT_BIT;
#endif
			}
		}
		writel(data32, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_ENABLE_1_REG);

		local_irq_restore(flags);
		return 0; 
    }
    else
    {
       	return 1;
    }   
}
#endif

/*----------------------------------------------------------------------
* gmac_tx_timeout
*----------------------------------------------------------------------*/
void gmac_tx_timeout(struct net_device *dev)
{
	GMAC_INFO_T				*tp = (GMAC_INFO_T *)dev->priv;
	
	printk(KERN_INFO "%s: TX timeout  ", dev->name);
	schedule_work(&tp->tx_timeout_task);
	
#if 0
	if (tp->operation && storlink_ctl.link[tp->port_id])
	{
		netif_wake_queue(dev);
	}
#endif	
}

/*----------------------------------------------------------------------
* gmac_change_mtu -- Change the Maximum Transfer Unit
* @netdev: network interface device structure
* @new_mtu: new value for maximum frame size
*
* Returns 0 on success, negative on failure
*----------------------------------------------------------------------*/
static int gmac_change_mtu(struct net_device *dev, int new_mtu)
{
	GMAC_INFO_T				*tp = (GMAC_INFO_T *)dev->priv;
	int max_frame = new_mtu + ENET_HEADER_SIZE + ETHERNET_FCS_SIZE;
	GMAC_STATUS_T   status, old_status;
	
	old_status.bits32 = status.bits32 = gmac_read_reg(tp->base_addr, GMAC_STATUS);
	
	if((max_frame < MINIMUM_ETHERNET_FRAME_SIZE) ||	(max_frame > MAX_JUMBO_FRAME_SIZE)) 
	{
		printk("Invalid MTU setting\n");
		return -EINVAL;
	}

	dev->mtu = new_mtu;
	if (new_mtu > 1500)
	{
		printk("GMAC-%d MTU must <= 1500 \n",tp->port_id);
		return -EINVAL;
	}
	else 
	{
		printk("GMAC-%d Change MTU = %d\n",tp->port_id,new_mtu);
	}
	if (!netif_running(dev))
		goto out;
	
	if (gmac_close(dev))

                printk(KERN_ERR "%s: Fatal error on stopping device\n", dev->name);

        if (gmac_open(dev))

                printk(KERN_ERR "%s: Fatal error on opening device\n", dev->name);		
out:	
	return 0;


}
/*----------------------------------------------------------------------
* mac_set_rule_reg
*----------------------------------------------------------------------*/
int mac_set_rule_reg(int mac, int rule, int enabled, u32 reg0, u32 reg1, u32 reg2)
{
	int		total_key_dwords;
	
	total_key_dwords = 1;
	
	if (reg0 & MR_L2_BIT)
	{
		if (reg0 & MR_DA_BIT) total_key_dwords += 2;
		if (reg0 & MR_SA_BIT) total_key_dwords += 2;
		if ((reg0 & MR_DA_BIT) && ( reg0 & MR_SA_BIT)) total_key_dwords--;
		if (reg0 & (MR_PPPOE_BIT | MR_VLAN_BIT)) total_key_dwords++;
	}
	if (reg0 & MR_L3_BIT)
	{
		if (reg0 & (MR_IP_HDR_LEN_BIT | MR_TOS_TRAFFIC_BIT | MR_SPR_BITS))
			total_key_dwords++;
		if (reg0 & MR_FLOW_LABLE_BIT) total_key_dwords++;
		if ((reg0 & MR_IP_VER_BIT) == 0) // IPv4
		{
			if (reg1 & 0xff000000) total_key_dwords += 1;
			if (reg1 & 0x00ff0000) total_key_dwords += 1;
		}
		else
		{
			if (reg1 & 0xff000000) total_key_dwords += 4;
			if (reg1 & 0x00ff0000) total_key_dwords += 4;
		}				
	}
	if (reg0 & MR_L4_BIT)
	{
		if (reg1 & 0x0000f000) total_key_dwords += 1;
		if (reg1 & 0x00000f00) total_key_dwords += 1;
		if (reg1 & 0x000000f0) total_key_dwords += 1;
		if (reg1 & 0x0000000f) total_key_dwords += 1;
		if (reg2 & 0xf0000000) total_key_dwords += 1;
		if (reg2 & 0x0f000000) total_key_dwords += 1;
	}			
	if (reg0 & MR_L7_BIT)
	{
		if (reg2 & 0x00f00000) total_key_dwords += 1;
		if (reg2 & 0x000f0000) total_key_dwords += 1;
		if (reg2 & 0x0000f000) total_key_dwords += 1;
		if (reg2 & 0x00000f00) total_key_dwords += 1;
		if (reg2 & 0x000000f0) total_key_dwords += 1;
		if (reg2 & 0x0000000f) total_key_dwords += 1;
	}
	
	if (total_key_dwords > HASH_MAX_KEY_DWORD)
		return -1;
		
	if (total_key_dwords == 0 && enabled)
		return -2;
	
	mac_set_rule_enable_bit(mac, rule, 0);
	if (enabled)
	{
		mac_set_MRxCRx(mac, rule, 0, reg0);
		mac_set_MRxCRx(mac, rule, 1, reg1);
		mac_set_MRxCRx(mac, rule, 2, reg2);
		mac_set_rule_action(mac, rule, total_key_dwords);
		mac_set_rule_enable_bit(mac, rule, enabled);
	}
	else
	{
		mac_set_rule_action(mac, rule, 0);
	}
	return total_key_dwords;	
}

/*----------------------------------------------------------------------
* mac_get_rule_enable_bit
*----------------------------------------------------------------------*/
int mac_get_rule_enable_bit(int mac, int rule)
{
	switch (rule)
	{
		case 0: return ((mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) >> 15) & 1);
		case 1: return ((mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) >> 31) & 1);
		case 2: return ((mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) >> 15) & 1);
		case 3: return ((mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) >> 31) & 1);
		default: return 0;
	}
}

/*----------------------------------------------------------------------
* mac_set_rule_enable_bit
*----------------------------------------------------------------------*/
void mac_set_rule_enable_bit(int mac, int rule, int data)
{
	u32 reg;
	
	if (data & ~1)
		return;
		
	switch (rule)
	{
		case 0: 
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) & ~(1<<15)) | (data << 15);
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG0, reg);
			break;
		case 1:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) & ~(1<<31)) | (data << 31);
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG0, reg);
			break;
		case 2:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) & ~(1<<15)) | (data << 15);
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG1, reg);
			break;
		case 3:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) & ~(1<<31)) | (data << 31);
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG1, reg);
	}
}

/*----------------------------------------------------------------------
* mac_set_rule_action
*----------------------------------------------------------------------*/
int mac_set_rule_action(int mac, int rule, int data)
{
	u32 reg;
	
	if (data > 32)
		return -1;
	
	if (data)
		data = (data << 6) | (data + HASH_ACTION_DWORDS); 
	switch (rule)
	{
		case 0: 
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) & ~(0x7ff));
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG0, reg | data);
			break;
		case 1:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG0) & ~(0x7ff<<16));
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG0, reg | (data << 16));
			break;
		case 2:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) & ~(0x7ff));
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG1,  reg | data);
			break;
		case 3:
			reg = (mac_read_dma_reg(mac, GMAC_HASH_ENGINE_REG1) & ~(0x7ff<<16));
			mac_write_dma_reg(mac, GMAC_HASH_ENGINE_REG1, reg | (data << 16));
			break;
		default:
			return -1;
	}
	
	return 0;
}
/*----------------------------------------------------------------------
* mac_get_MRxCRx
*----------------------------------------------------------------------*/
int mac_get_MRxCRx(int mac, int rule, int ctrlreg)
{
	int reg;
	
	switch (rule)
	{
		case 0: reg = GMAC_MR0CR0 + ctrlreg * 4; break;
		case 1: reg = GMAC_MR1CR0 + ctrlreg * 4; break;
		case 2: reg = GMAC_MR2CR0 + ctrlreg * 4; break;
		case 3: reg = GMAC_MR3CR0 + ctrlreg * 4; break;
		default: return 0;
	}
	return mac_read_dma_reg(mac, reg);
}

/*----------------------------------------------------------------------
* mac_set_MRxCRx
*----------------------------------------------------------------------*/
void mac_set_MRxCRx(int mac, int rule, int ctrlreg, u32 data)
{
	int reg;
	
	switch (rule)
	{
		case 0: reg = GMAC_MR0CR0 + ctrlreg * 4; break;
		case 1: reg = GMAC_MR1CR0 + ctrlreg * 4; break;
		case 2: reg = GMAC_MR2CR0 + ctrlreg * 4; break;
		case 3: reg = GMAC_MR3CR0 + ctrlreg * 4; break;
		default: return;
	}
	mac_write_dma_reg(mac, reg, data);
}

/*----------------------------------------------------------------------
* mac_set_rule_priority
*----------------------------------------------------------------------*/
void mac_set_rule_priority(int mac, int p0, int p1, int p2, int p3)
{
	int 			i;
	GMAC_MRxCR0_T	reg[4];
	
	for (i=0; i<4; i++)
		reg[i].bits32 = mac_get_MRxCRx(mac, i, 0);
			
	reg[0].bits.priority = p0;
	reg[1].bits.priority = p1;
	reg[2].bits.priority = p2;
	reg[3].bits.priority = p3;
	
	for (i=0; i<4; i++)
		mac_set_MRxCRx(mac, i, 0, reg[i].bits32);
}

/*----------------------------------------------------------------------
* gmac_netdev_ioctl
*----------------------------------------------------------------------*/
static int gmac_netdev_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	int 				rc = 0;
    unsigned char		*hwa = rq->ifr_ifru.ifru_hwaddr.sa_data;

#ifdef br_if_ioctl    
    struct 				ethtool_cmd ecmd; 	//br_if.c will call this ioctl
	GMAC_INFO_T 		*tp = dev->priv;
#endif

#ifdef 	CONFIG_SL351x_NAT
	if (cmd == SIOCDEVPRIVATE)
		return sl351x_nat_ioctl(dev, rq, cmd);
#else
	if (cmd == SIOCDEVPRIVATE)
	{
		return sl351x_gmac_ioctl(dev, rq, cmd);
	}
#endif

	if (!netif_running(dev))
	{
	    printk("Before changing the H/W address,please down the device.\n");
		return -EINVAL;
    }

	switch (cmd) {
	case SIOCETHTOOL:
	
#ifdef br_if_ioctl  	//br_if.c will call this ioctl
		memset((void *) &ecmd, 0, sizeof (ecmd));
           	    ecmd.supported =
                	SUPPORTED_Autoneg | SUPPORTED_TP | SUPPORTED_MII |
                    SUPPORTED_10baseT_Half | SUPPORTED_10baseT_Full |
                    SUPPORTED_100baseT_Half | SUPPORTED_100baseT_Full;
         		    ecmd.port = PORT_TP;
            	    ecmd.transceiver = XCVR_EXTERNAL;
            	    ecmd.phy_address = tp->phy_addr;
            	    switch (tp->speed_status)
            	    {
            	    case GMAC_SPEED_10: ecmd.speed = SPEED_10; break;
             	    case GMAC_SPEED_100: ecmd.speed = SPEED_100; break;
            	    case GMAC_SPEED_1000: ecmd.speed = SPEED_1000; break;
            	    default: ecmd.speed = SPEED_10; break;
            	   }
            	    ecmd.duplex = tp->full_duplex_status ? DUPLEX_FULL : DUPLEX_HALF;
            	    ecmd.advertising = ADVERTISED_TP;
            	    ecmd.advertising |= ADVERTISED_Autoneg;
           	    ecmd.autoneg = AUTONEG_ENABLE;
                    if (copy_to_user(rq->ifr_data, &ecmd, sizeof (ecmd)))
                  	return -EFAULT;
#endif               
  
        break;

    case SIOCSIFHWADDR:
        gmac_set_mac_address(dev,hwa);
        break;

	case SIOCGMIIPHY:	/* Get the address of the PHY in use. */
        break;

	case SIOCGMIIREG:	/* Read the specified MII register. */
		break;

	case SIOCSMIIREG:	/* Write the specified MII register */
		break;

	default:
		rc = -EOPNOTSUPP;
		break;
	}

	return rc;
}

/*----------------------------------------------------------------------
* dm_long_1
*	gmac read mem -b 0xc1ff4740 -l 8 -4
*	
*	0xc1ff4740: E5D24001 E5D21004 - E59D302C E1A05000 
*	0xc1ff4750: E5D20005 E0643003 - E1A05435 E1814400 
*----------------------------------------------------------------------*/
void dm_long_1(u32 location, int length)
{
	u32		*start_p, *curr_p, *end_p;
	u32		*datap, data;
	int		i;
		
	start_p = (u32 *)location;
	end_p = (u32 *)location + length;
	curr_p = (u32 *)((u32)location & 0xfffffff0);
	datap = (u32 *)location;
	while (curr_p < end_p)
	{
		printk("0x%08x: ",(u32)curr_p & 0xfffffff0);
		for (i=0; i<4; i++)
		{
			if (curr_p < start_p || curr_p >= end_p)
               printk("         ");
			else
			{
				data = *datap;
				printk("%08X ", data);
			}
			if (i==1)
              printk("- ");
			
			curr_p++;
			datap++;
		}
        printk("\n");
	} 
}
/*----------------------------------------------------------------------
* dm_byte
*	gmac read mem -b 0xc1ff4740 -l 64 -1
*	
*	0xc1ff4740: 01 40 D2 E5 04 10 D2 E5 - 2C 30 9D E5 00 50 A0 E1 .@......,0...P..
*	0xc1ff4750: 05 00 D2 E5 03 30 64 E0 - 35 54 A0 E1 00 44 81 E1 .....0d.5T...D..
*	0xc1ff4760: 0F 00 54 E3 04 B0 A0 91 - 2C 30 8D E5 01 60 86 92 ..T.....,0...`..
*	0xc1ff4770: 70 B9 08 95 04 80 88 92 - 20 20 8D E5 56 00 00 9A p.......  ..V...
*
*----------------------------------------------------------------------*/
void dm_byte(u32 location, int length)
{
	u8		*start_p, *end_p, *curr_p;
	u8		*datap, data;
	int		i;

	start_p = (u8 *)location;
	end_p = (u8 *)start_p + length;
	curr_p=(u8 *)((u32)location & 0xfffffff0);
	datap = (u8 *)location;

	while (curr_p < end_p)
	{
		u8 *p1, *p2;
        printk("0x%08x: ",(u32)curr_p & 0xfffffff0);
        p1 = curr_p;
        p2 = datap;
		// dump data		        
		for (i=0; i<16; i++)
        {
			if (curr_p < start_p || curr_p >= end_p)
				 printk("         ");
			else
			{
				data = *datap;
				printk("%02X ", data);
			}
			if (i==7)
				printk("- ");
			curr_p++;
			datap++;
        }
		// dump ascii	        
		curr_p = p1;
		datap = p2;
		for (i=0; i<16; i++)
		{
			if (curr_p < start_p || curr_p >= end_p)
				printk(".");
			else
			{
				data = *datap ;
				if (data<0x20 || data>0x7f || data==0x25) 
					printk(".");
				else
					printk("%c", data);;
			}
			curr_p++;
			datap++;
		}
		printk("\n");
	} 
}

/*----------------------------------------------------------------------
* dm_short
*	 gmac read mem -b 0xc1ff4740 -l 24 -2
*
*	0xc1ff4740: 4001 E5D2 1004 E5D2 - 302C E59D 5000 E1A0 
*	0xc1ff4750: 0005 E5D2 3003 E064 - 5435 E1A0 4400 E181 
*	0xc1ff4760: 000F E354 B004 91A0 - 302C E58D 6001 9286 
*
*----------------------------------------------------------------------*/
void dm_short(u32 location, int length)
{
	u16		*start_p, *curr_p, *end_p;
	u16		*datap, data;
	int		i;

	start_p = (u16 *)location;
	end_p =  (u16 *)location + length;
	curr_p = (u16 *)((u32)location & 0xfffffff0);
	datap = (u16 *)location;
	
	while (curr_p < end_p)
	{
		printk("0x%08x: ",(u32)curr_p & 0xfffffff0);
		for (i=0; i<8; i++)
		{
			if (curr_p < start_p || curr_p >= end_p)
				printk("         ");
			else
			{
				data = *datap;
				printk("%04X ", data);
			}
			if (i==3)
              printk("- ");
			curr_p++;
			datap++;
		}
		printk("\n");
	} 
}

//#ifdef SL351x_GMAC_WORKAROUND

#define GMAC_TX_STATE_OFFSET	0x60
#define GMAC_RX_STATE_OFFSET	0x64
#define GMAC_POLL_HANGED_NUM	200
#define GMAC_RX_HANGED_STATE	0x4b2000
#define GMAC_RX_HANGED_MASK		0xdff000
#define GMAC_TX_HANGED_STATE	0x34012
#define GMAC_TX_HANGED_MASK		0xfffff
#define TOE_GLOBAL_REG_SIZE		(0x78/sizeof(u32))
#define TOE_DMA_REG_SIZE		(0xd0/sizeof(u32))
#define TOE_GMAC_REG_SIZE		(0x30/sizeof(u32))
#define GMAC0_RX_HANG_BIT		(1 << 0)
#define GMAC0_TX_HANG_BIT		(1 << 1)
#define GMAC1_RX_HANG_BIT		(1 << 2)
#define GMAC1_TX_HANG_BIT		(1 << 3)

int		gmac_in_do_workaround;
#if 0
int		debug_cnt, poll_max_cnt;
#endif
u32		gmac_workaround_cnt[4];
u32		toe_global_reg[TOE_GLOBAL_REG_SIZE];
u32		toe_dma_reg[GMAC_NUM][TOE_DMA_REG_SIZE];
u32		toe_gmac_reg[GMAC_NUM][TOE_GMAC_REG_SIZE];
u32		gmac_short_frame_workaround_cnt[2];

static void sl351x_gmac_release_buffers(void);
static void sl351x_gmac_release_swtx_q(void);
static void sl351x_gmac_release_rx_q(void);
#ifdef _TOEQ_CLASSQ_READY_
static void sl351x_gmac_release_class_q(void);
static void sl351x_gmac_release_toe_q(void);
static void sl351x_gmac_release_intr_q(void);
#endif
static void sl351x_gmac_release_sw_free_q(void);
static void sl351x_gmac_release_hw_free_q(void);
#ifdef CONFIG_SL351x_NAT
#ifdef SL351x_GMAC_WORKAROUND
static int get_free_desc_cnt(unsigned long rwptr, int total);
#endif
static void sl351x_gmac_release_hwtx_q(void);
u32     sl351x_nat_workaround_cnt;
#endif
void sl351x_gmac_save_reg(void);
void sl351x_gmac_restore_reg(void);

#ifdef SL351x_GMAC_WORKAROUND
/*----------------------------------------------------------------------
* 	sl351x_poll_gmac_hanged_status
* 	- Called by timer routine, period 10ms
*	- If (state != 0 && state == prev state && )
*----------------------------------------------------------------------*/
void sl351x_poll_gmac_hanged_status(u32 data)
{
	int 			i;
	u32 			state;
	TOE_INFO_T		*toe;
	GMAC_INFO_T		*tp;
	u32				hanged_state;
	// int				old_operation[GMAC_NUM];
#ifdef CONFIG_SL351x_NAT
	u32				hw_free_cnt;
#endif
	
	if (gmac_in_do_workaround)
		return;
		
	gmac_in_do_workaround = 1;
	
	toe = (TOE_INFO_T *)&toe_private_data;
	hanged_state = 0;
	
#ifdef SL351x_TEST_WORKAROUND
	if (toe->gmac[0].operation || toe->gmac[1].operation)
	{
		debug_cnt++;
		if (debug_cnt == (30 * HZ))
		{
			debug_cnt = 0;
			hanged_state = GMAC0_RX_HANG_BIT;
			goto do_workaround;
		}
	}
#endif
	if (toe->gmac[0].operation)
		hanged_state |= GMAC0_RX_HANG_BIT | GMAC0_TX_HANG_BIT;
	
#if (GMAC_NUM > 1)
	if (toe->gmac[1].operation)
		hanged_state |= GMAC1_RX_HANG_BIT | GMAC1_TX_HANG_BIT;
#endif
	
	for (i=0; i<GMAC_POLL_HANGED_NUM; i++)
	{
		if (hanged_state & GMAC0_RX_HANG_BIT)
		{
			state = readl(TOE_GMAC0_BASE + GMAC_RX_STATE_OFFSET) & GMAC_RX_HANGED_MASK;
			if (state != GMAC_RX_HANGED_STATE)
				hanged_state &= ~GMAC0_RX_HANG_BIT;
		}
		if (hanged_state & GMAC0_TX_HANG_BIT)
		{
			state = readl(TOE_GMAC0_BASE + GMAC_TX_STATE_OFFSET) & GMAC_TX_HANGED_MASK;
			if (state != GMAC_TX_HANGED_STATE)
				hanged_state &= ~GMAC0_TX_HANG_BIT;
		}
#if (GMAC_NUM > 1)
		if (hanged_state & GMAC1_RX_HANG_BIT)
		{
			state = readl(TOE_GMAC1_BASE + GMAC_RX_STATE_OFFSET) & GMAC_RX_HANGED_MASK;
			if (state != GMAC_RX_HANGED_STATE)
				hanged_state &= ~GMAC1_RX_HANG_BIT;
		}
		if (hanged_state & GMAC1_TX_HANG_BIT)
		{
			state = readl(TOE_GMAC1_BASE + GMAC_TX_STATE_OFFSET) & GMAC_TX_HANGED_MASK;
			if (state != GMAC_TX_HANGED_STATE)
				hanged_state &= ~GMAC1_TX_HANG_BIT;
		}
#endif
		if (!hanged_state)
		{
#if 0			
			if (i < poll_max_cnt)
				poll_max_cnt = i;
#endif
			if (toe->gmac[0].short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
			{
				gmac_short_frame_workaround_cnt[0]++;
				toe->gmac[0].short_frames_cnt = 0;
				goto do_workaround;
			}
#if (GMAC_NUM > 1)
			if (toe->gmac[1].short_frames_cnt >= GMAC_SHORT_FRAME_THRESHOLD)
			{
				gmac_short_frame_workaround_cnt[1]++;
				toe->gmac[1].short_frames_cnt = 0;
				goto do_workaround;
			}
#endif

#ifdef CONFIG_SL351x_NAT
			hw_free_cnt = readl(TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);
			hw_free_cnt = get_free_desc_cnt(hw_free_cnt, TOE_HW_FREEQ_DESC_NUM);
#ifdef NAT_WORKAROUND_BY_RESET_GMAC				
			if (readl(TOE_GLOBAL_BASE + 0x4084) && (hw_free_cnt <= PAUSE_SET_HW_FREEQ))
			{
				sl351x_nat_workaround_cnt++;
				goto do_workaround;
			}
#else
			if (readl(TOE_GLOBAL_BASE + 0x4084) && (hw_free_cnt <= (PAUSE_SET_HW_FREEQ*2)))
			{
				sl351x_nat_workaround_cnt++;
				sl351x_nat_workaround_handler();
			}
#endif
#endif				
			gmac_in_do_workaround = 0;
			add_timer(&gmac_workround_timer_obj);
			return;
		}
	}
	
do_workaround:

	gmac_initialized = 0;
	if (hanged_state)
	{
		if (hanged_state & GMAC0_RX_HANG_BIT) gmac_workaround_cnt[0]++;
		if (hanged_state & GMAC0_TX_HANG_BIT) gmac_workaround_cnt[1]++;
		if (hanged_state & GMAC1_RX_HANG_BIT) gmac_workaround_cnt[2]++;
		if (hanged_state & GMAC1_TX_HANG_BIT) gmac_workaround_cnt[3]++;
	}
	
	for (i=0; i<GMAC_NUM; i++)
	{
		tp=(GMAC_INFO_T *)&toe->gmac[i];
		// old_operation[i] = tp->operation;
		if (tp->operation)
		{
			netif_stop_queue(tp->dev);
			clear_bit(__LINK_STATE_START, &tp->dev->state);
			toe_gmac_disable_interrupt(tp->irq);
			toe_gmac_disable_tx_rx(tp->dev);
			toe_gmac_hw_stop(tp->dev);
		}
	}

	// clear all status bits
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_0_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_1_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_2_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_3_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_4_REG);

#if 0
	if ((hanged_state & GMAC0_RX_HANG_BIT) && 
		(readl(TOE_GMAC0_DMA_BASE + 0xdc) & 0xf0))
	{
		struct sk_buff *skb;
		unsigned int buf;
		buf = readl(TOE_GMAC0_DMA_BASE + 0x68) & ~3;
#ifdef CONFIG_SL351x_NAT
		if (buf < toe->hwfq_buf_base_dma || buf > toe->hwfq_buf_end_dma)
#endif
		{
			skb = (struct sk_buff *)(REG32(buf - SKB_RESERVE_BYTES));
			printk("GMAC-0 free a loss SKB 0x%x\n", (u32)skb);
			dev_kfree_skb(skb);
		}
	}
	if ((hanged_state & GMAC1_RX_HANG_BIT)  && 
		(readl(TOE_GMAC1_DMA_BASE + 0xdc) & 0xf0))
	{
		struct sk_buff *skb;
		unsigned int buf;
		buf = readl(TOE_GMAC1_DMA_BASE + 0x68) & ~3;
#ifdef CONFIG_SL351x_NAT
		if (buf < toe->hwfq_buf_base_dma || buf > toe->hwfq_buf_end_dma)
#endif		
		{
			skb = (struct sk_buff *)(REG32(buf - SKB_RESERVE_BYTES));
			printk("GMAC-1 free a loss SKB 0x%x\n", (u32)skb);
			dev_kfree_skb(skb);
		}
	}
#endif
	
	sl351x_gmac_release_buffers();
	sl351x_gmac_save_reg();
	toe_gmac_sw_reset();
	sl351x_gmac_restore_reg();

	if (toe->gmac[0].default_qhdr->word1.bits32)
	{
		// printk("===> toe->gmac[0].default_qhdr->word1 = 0x%x\n", toe->gmac[0].default_qhdr->word1);
		sl351x_gmac_release_rx_q();
		writel(0, &toe->gmac[0].default_qhdr->word1);
	}
	if (toe->gmac[1].default_qhdr->word1.bits32)
	{
		// printk("===> toe->gmac[1].default_qhdr->word1 = 0x%x\n", toe->gmac[1].default_qhdr->word1);
		sl351x_gmac_release_rx_q();
		writel(0, &toe->gmac[1].default_qhdr->word1);
	}
	
	gmac_initialized = 1;
	
#ifdef 	CONFIG_SL351x_NAT
	writel(0, TOE_GLOBAL_BASE + 0x4084);
#endif
	
	for (i=0; i<GMAC_NUM; i++)
	{
		tp=(GMAC_INFO_T *)&toe->gmac[i];
 		if (tp->operation)
 		{
			toe_gmac_enable_interrupt(tp->irq);
			toe_gmac_hw_start(tp->dev);
			toe_gmac_enable_tx_rx(tp->dev);
			netif_wake_queue(tp->dev);
			set_bit(__LINK_STATE_START, &tp->dev->state);
		}
	}

	gmac_in_do_workaround = 0;
	add_timer(&gmac_workround_timer_obj);
}
#endif	//end of SL351x_GMAC_WORKAROUND

/*----------------------------------------------------------------------
*	gmac_reset_task()
* 	Actual routine to reset the adapter when a timeout on Tx has occurred
*----------------------------------------------------------------------*/
static void gmac_reset_task(struct net_device *dev)	
{
	int 			i;
	TOE_INFO_T		*toe;
	GMAC_INFO_T		*tp;
	
	if (gmac_in_do_workaround)
		return;
		
	gmac_in_do_workaround = 1;
//	GMAC_INFO_T				*tp = (GMAC_INFO_T *)dev->priv;
	toe = (TOE_INFO_T *)&toe_private_data;
	
	for (i=0; i<GMAC_NUM; i++)
	{
		tp=(GMAC_INFO_T *)&toe->gmac[i];
		// old_operation[i] = tp->operation;
		if (tp->operation)
		{
			netif_stop_queue(tp->dev);
			clear_bit(__LINK_STATE_START, &tp->dev->state);
			toe_gmac_disable_interrupt(tp->irq);
			toe_gmac_disable_tx_rx(tp->dev);
			toe_gmac_hw_stop(tp->dev);
		}
	}

	// clear all status bits
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_0_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_1_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_2_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_3_REG);
	writel(0xffffffff, TOE_GLOBAL_BASE + GLOBAL_INTERRUPT_STATUS_4_REG);

	sl351x_gmac_release_buffers();
	sl351x_gmac_save_reg();
	toe_gmac_sw_reset();
	sl351x_gmac_restore_reg();

	if (toe->gmac[0].default_qhdr->word1.bits32)
	{
		sl351x_gmac_release_rx_q();
		writel(0, &toe->gmac[0].default_qhdr->word1);
	}
	if (toe->gmac[1].default_qhdr->word1.bits32)
	{
		sl351x_gmac_release_rx_q();
		writel(0, &toe->gmac[1].default_qhdr->word1);
	}
	
	gmac_initialized = 1;
	
	
	for (i=0; i<GMAC_NUM; i++)
	{
		tp=(GMAC_INFO_T *)&toe->gmac[i];
 		if (tp->operation)
 		{
			toe_gmac_enable_interrupt(tp->irq);
			toe_gmac_hw_start(tp->dev);
			toe_gmac_enable_tx_rx(tp->dev);
			netif_wake_queue(tp->dev);
			set_bit(__LINK_STATE_START, &tp->dev->state);
		}
	}

	gmac_in_do_workaround = 0;
}


/*----------------------------------------------------------------------
*	get_free_desc_cnt
*----------------------------------------------------------------------*/
#ifdef SL351x_GMAC_WORKAROUND
#ifdef CONFIG_SL351x_NAT
static int get_free_desc_cnt(unsigned long rwptr, int total)
{
	unsigned short wptr = rwptr & 0xffff;
	unsigned short rptr = rwptr >> 16;
	
	if (wptr >= rptr)
		return (total - wptr + rptr);
	else 
		return (rptr - wptr);
}
#endif
#endif
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_buffers
*----------------------------------------------------------------------*/
static void sl351x_gmac_release_buffers(void)
{
	// Free buffers & Descriptors in all SW Tx Queues
	sl351x_gmac_release_swtx_q();
	
	// Free buffers in Default Rx Queues
	sl351x_gmac_release_rx_q();
	
#ifdef _TOEQ_CLASSQ_READY_
	// Free buffers in Classification Queues
	sl351x_gmac_release_class_q();
	
	// Free buffers in TOE Queues
	sl351x_gmac_release_toe_q();

	// Free buffers in Interrupt Queues
	sl351x_gmac_release_intr_q();
#endif
	
	// Free buffers & descriptors in SW free queue
	sl351x_gmac_release_sw_free_q();
	
	// Free buffers & descriptors in HW free queue
	sl351x_gmac_release_hw_free_q();
	
#ifdef CONFIG_SL351x_NAT
	// Free buffers & descriptors in HW free queue
	sl351x_gmac_release_hwtx_q();
#endif	
}
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_swtx_q
*----------------------------------------------------------------------*/
static void sl351x_gmac_release_swtx_q(void)
{
	int				i, j;
	GMAC_TXDESC_T	*curr_desc;
	unsigned int	desc_count;
	TOE_INFO_T		*toe;
	GMAC_INFO_T		*tp;
	GMAC_SWTXQ_T	*swtxq;
	DMA_RWPTR_T		rwptr;

	toe = (TOE_INFO_T *)&toe_private_data;
	tp = (GMAC_INFO_T *)&toe->gmac[0];
	for (i=0; i<GMAC_NUM; i++, tp++)
	{
		if (!tp->existed) continue;
		swtxq = (GMAC_SWTXQ_T *)&tp->swtxq[0];
		for (j=0; j<TOE_SW_TXQ_NUM; j++, swtxq++)
		{
			for (;;)
			{
				rwptr.bits32 = readl(swtxq->rwptr_reg);
				if (rwptr.bits.rptr == swtxq->finished_idx)
				break;
				curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
				// if (curr_desc->word0.bits.status_tx_ok)
				{
					desc_count = curr_desc->word0.bits.desc_count;
					while (--desc_count)
					{
						curr_desc->word0.bits.status_tx_ok = 0;
						swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, swtxq->total_desc_num);
						curr_desc = (GMAC_TXDESC_T *)swtxq->desc_base + swtxq->finished_idx;
					}

					curr_desc->word0.bits.status_tx_ok = 0;
					if (swtxq->tx_skb[swtxq->finished_idx])
					{
						dev_kfree_skb_irq(swtxq->tx_skb[swtxq->finished_idx]);
						swtxq->tx_skb[swtxq->finished_idx] = NULL;
					}
				}
				swtxq->finished_idx = RWPTR_ADVANCE_ONE(swtxq->finished_idx, swtxq->total_desc_num);
			}
			writel(0, swtxq->rwptr_reg);
			swtxq->finished_idx = 0;
		}
	}

}
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_rx_q
*----------------------------------------------------------------------*/
static void sl351x_gmac_release_rx_q(void)
{
	int				i;
	TOE_INFO_T		*toe;
	GMAC_INFO_T		*tp;
	DMA_RWPTR_T		rwptr;
	volatile GMAC_RXDESC_T	*curr_desc;
	struct sk_buff			*skb;

	toe = (TOE_INFO_T *)&toe_private_data;
	tp = (GMAC_INFO_T *)&toe->gmac[0];
	for (i=0; i<GMAC_NUM; i++, tp++)
	{
		if (!tp->existed) continue;
		rwptr.bits32 = readl(&tp->default_qhdr->word1);
		while (rwptr.bits.rptr != rwptr.bits.wptr)
		{
			curr_desc = (GMAC_RXDESC_T *)tp->default_desc_base + rwptr.bits.rptr;
			skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
			dev_kfree_skb_irq(skb);
			rwptr.bits.rptr = RWPTR_ADVANCE_ONE(rwptr.bits.rptr, tp->default_desc_num);
			SET_RPTR(&tp->default_qhdr->word1, rwptr.bits.rptr);
			rwptr.bits32 = readl(&tp->default_qhdr->word1);
		}  // while
		writel(0, &tp->default_qhdr->word1);
		tp->rx_rwptr.bits32 = 0;
	} // for

}
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_class_q
*----------------------------------------------------------------------*/
#ifdef _TOEQ_CLASSQ_READY_
static void sl351x_gmac_release_class_q(void)
{
	int				i;
	TOE_INFO_T		*toe;
	CLASSQ_INFO_T	*classq;
	DMA_RWPTR_T		rwptr;
	volatile GMAC_RXDESC_T	*curr_desc;
	struct sk_buff			*skb;

	toe = (TOE_INFO_T *)&toe_private_data;
	classq = (CLASSQ_INFO_T *)&toe->classq[0];
	for (i=0; i<TOE_CLASS_QUEUE_NUM; i++, classq++)
	{
		rwptr.bits32 = readl(&classq->qhdr->word1);
		while (rwptr.bits.rptr != rwptr.bits.wptr)
		{
			curr_desc = (GMAC_RXDESC_T *)classq->desc_base + rwptr.bits.rptr;
			skb = (struct sk_buff *)(REG32(__va(curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
			dev_kfree_skb_irq(skb);
			rwptr.bits.rptr = RWPTR_ADVANCE_ONE(rwptr.bits.rptr, classq->desc_num);
			SET_RPTR(&classq->qhdr->word1, rwptr.bits.rptr);
			rwptr.bits32 = readl(&classq->qhdr->word1);
		}  // while
		writel(0, &classq->qhdr->word1);
		classq->rwptr.bits32 = 0;
	} // for

}
#endif
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_toe_q
*----------------------------------------------------------------------*/
#ifdef _TOEQ_CLASSQ_READY_
static void sl351x_gmac_release_toe_q(void)
{
	int				i;
	TOE_INFO_T		*toe;
	TOEQ_INFO_T		*toeq_info;
	TOE_QHDR_T		*toe_qhdr;
	DMA_RWPTR_T		rwptr;
	volatile GMAC_RXDESC_T	*curr_desc;
	unsigned int	rptr, wptr;
	GMAC_RXDESC_T	*toe_curr_desc;
	struct sk_buff			*skb;

	toe = (TOE_INFO_T *)&toe_private_data;
	toe_qhdr = (TOE_QHDR_T *)TOE_TOE_QUE_HDR_BASE;
	for (i=0; i<TOE_TOE_QUEUE_NUM; i++, toe_qhdr++)
	{
		toeq_info = (TOEQ_INFO_T *)&toe->toeq[i];
		wptr = toe_qhdr->word1.bits.wptr;
		rptr = toe_qhdr->word1.bits.rptr;
		while (rptr != wptr)
		{
			toe_curr_desc = (GMAC_RXDESC_T *)toeq_info->desc_base + rptr;
			skb = (struct sk_buff *)(REG32(__va(toe_curr_desc->word2.buf_adr) - SKB_RESERVE_BYTES));
			dev_kfree_skb_irq(skb);
			rptr = RWPTR_ADVANCE_ONE(rptr, toeq_info->desc_num);
			SET_RPTR(&toe_qhdr->word1.bits32, rptr);
			wptr = toe_qhdr->word1.bits.wptr;
			rptr = toe_qhdr->word1.bits.rptr;
		}
		toe_qhdr->word1.bits32 = 0;
		toeq_info->rwptr.bits32 = 0;
	}
}
#endif
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_intr_q
*----------------------------------------------------------------------*/
#ifdef _TOEQ_CLASSQ_READY_
static void sl351x_gmac_release_intr_q(void)
{
}
#endif
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_sw_free_q
*----------------------------------------------------------------------*/
static void sl351x_gmac_release_sw_free_q(void)
{
	TOE_INFO_T				*toe;
	volatile DMA_RWPTR_T	fq_rwptr;
	volatile GMAC_RXDESC_T	*fq_desc;

	toe = (TOE_INFO_T *)&toe_private_data;
	fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);

	while ((unsigned short)RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr, TOE_SW_FREEQ_DESC_NUM) != fq_rwptr.bits.rptr)
	{
		struct sk_buff *skb;
		if ((skb = dev_alloc_skb(SW_RX_BUF_SIZE))==NULL)  /* allocate socket buffer */
		{
			printk("%s::skb buffer allocation fail !\n",__func__); while(1);
		}
		// *(unsigned int *)(skb->data) = (unsigned int)skb;
		REG32(skb->data) = (unsigned long)skb;
		skb_reserve(skb, SKB_RESERVE_BYTES);

		fq_rwptr.bits.wptr = RWPTR_ADVANCE_ONE(fq_rwptr.bits.wptr, TOE_SW_FREEQ_DESC_NUM);
		fq_desc = (volatile GMAC_RXDESC_T *)toe->swfq_desc_base + fq_rwptr.bits.wptr;
		fq_desc->word2.buf_adr = (unsigned int)__pa(skb->data);
		SET_WPTR(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG, fq_rwptr.bits.wptr);
		fq_rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);
	}

	toe->fq_rx_rwptr.bits.wptr = TOE_SW_FREEQ_DESC_NUM - 1;
	toe->fq_rx_rwptr.bits.rptr = 0;
	writel(toe->fq_rx_rwptr.bits32, TOE_GLOBAL_BASE + GLOBAL_SWFQ_RWPTR_REG);

}
/*----------------------------------------------------------------------
* 	sl351x_gmac_release_hw_free_q
*----------------------------------------------------------------------*/
static void sl351x_gmac_release_hw_free_q(void)
{
	DMA_RWPTR_T			rwptr_reg;

#ifdef CONFIG_SL351x_NAT
	int					i;
	TOE_INFO_T			*toe;
	GMAC_RXDESC_T		*desc_ptr;
	unsigned int		buf_ptr;
	
	toe = (TOE_INFO_T *)&toe_private_data;
	desc_ptr = (GMAC_RXDESC_T *)toe->hwfq_desc_base;
	buf_ptr = (unsigned int)toe->hwfq_buf_base_dma;
	for (i=0; i<TOE_HW_FREEQ_DESC_NUM; i++)
	{
		desc_ptr->word0.bits.buffer_size = HW_RX_BUF_SIZE;
		desc_ptr->word1.bits.sw_id = i;
		desc_ptr->word2.buf_adr = (unsigned int)buf_ptr;
   		desc_ptr++;
   		buf_ptr += HW_RX_BUF_SIZE;
	}
#endif	
	rwptr_reg.bits.wptr = TOE_HW_FREEQ_DESC_NUM - 1;
	rwptr_reg.bits.rptr = 0;
	writel(rwptr_reg.bits32, TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);
}

/*----------------------------------------------------------------------
* 	sl351x_gmac_release_hw_free_q
*----------------------------------------------------------------------*/
#ifdef CONFIG_SL351x_NAT
static void sl351x_gmac_release_hwtx_q(void)
{
	int				i;
	unsigned int	rwptr_addr;
	
	rwptr_addr = TOE_GMAC0_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		writel(0, rwptr_addr);
		rwptr_addr+=4;
	}
	rwptr_addr = TOE_GMAC1_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		writel(0, rwptr_addr);
		rwptr_addr+=4;
	}
}
#endif

/*----------------------------------------------------------------------
* 	sl351x_gmac_save_reg
*----------------------------------------------------------------------*/
void sl351x_gmac_save_reg(void)
{
	int	i;
	volatile u32	*destp;
	unsigned int	srce_addr;
	
	srce_addr = TOE_GLOBAL_BASE;
	destp = (volatile u32 *)toe_global_reg;
	for (i=0; i<TOE_GLOBAL_REG_SIZE; i++, destp++, srce_addr+=4)
		*destp = readl(srce_addr);
		
	srce_addr = TOE_GMAC0_DMA_BASE;
	destp = (volatile u32 *)&toe_dma_reg[0][0];
	for (i=0; i<TOE_DMA_REG_SIZE; i++, destp++, srce_addr+=4)
	{
		if (srce_addr ==  (TOE_GMAC0_DMA_BASE+0x38))
			srce_addr = (TOE_GMAC0_DMA_BASE+0x50);
		if (srce_addr ==  (TOE_GMAC0_DMA_BASE+0x58))
			srce_addr = (TOE_GMAC0_DMA_BASE+0x70);
			
		*destp = readl(srce_addr);
	}
	srce_addr = TOE_GMAC1_DMA_BASE;
	destp = (volatile u32 *)&toe_dma_reg[1][0];
	for (i=0; i<TOE_DMA_REG_SIZE; i++, destp++, srce_addr+=4)
	{
		if (srce_addr ==  (TOE_GMAC0_DMA_BASE+0x38))
			srce_addr = (TOE_GMAC0_DMA_BASE+0x50);
		if (srce_addr ==  (TOE_GMAC0_DMA_BASE+0x58))
			srce_addr = (TOE_GMAC0_DMA_BASE+0x70);
			
		*destp = readl(srce_addr);
	}
		
	srce_addr = TOE_GMAC0_BASE;
	destp = (volatile u32 *)&toe_gmac_reg[0][0];
	for (i=0; i<TOE_GMAC_REG_SIZE; i++, destp++, srce_addr+=4)
		*destp = readl(srce_addr);
		
	srce_addr = TOE_GMAC1_BASE;
	destp = (volatile u32 *)&toe_gmac_reg[1][0];
	for (i=0; i<TOE_GMAC_REG_SIZE; i++, destp++, srce_addr+=4)
		*destp = readl(srce_addr);
}

/*----------------------------------------------------------------------
* 	sl351x_gmac_restore_reg
*----------------------------------------------------------------------*/
void sl351x_gmac_restore_reg(void)
{
	int	i;
	volatile u32	*srcep;
	unsigned int	dest_addr;

	srcep = (volatile u32 *)&toe_dma_reg[0][0];
	dest_addr = TOE_GMAC0_DMA_BASE;
	for (i=0; i<TOE_DMA_REG_SIZE; i++, dest_addr+=4, srcep++)
	{
		if (dest_addr == (TOE_GMAC0_DMA_BASE+0x38))
			dest_addr = (TOE_GMAC0_DMA_BASE+0x50);
		if (dest_addr == (TOE_GMAC0_DMA_BASE+0x58))
			dest_addr = (TOE_GMAC0_DMA_BASE+0x70);
			
		writel(*srcep, dest_addr);
		// gmac_write_reg(dest_addr, 0, *srcep, 0xffffffff);
	}
	srcep = (volatile u32 *)&toe_dma_reg[1][0];
	dest_addr = TOE_GMAC1_DMA_BASE;
	for (i=0; i<TOE_DMA_REG_SIZE; i++, dest_addr+=4, srcep++)
	{
		if (dest_addr == (TOE_GMAC0_DMA_BASE+0x38))
			dest_addr = (TOE_GMAC0_DMA_BASE+0x50);
		if (dest_addr == (TOE_GMAC0_DMA_BASE+0x58))
			dest_addr = (TOE_GMAC0_DMA_BASE+0x70);
			
		writel(*srcep, dest_addr);
		// gmac_write_reg(dest_addr, 0, *srcep, 0xffffffff);
	}
		
	srcep = (volatile u32 *)&toe_gmac_reg[0][0];
	dest_addr = TOE_GMAC0_BASE;
	for (i=0; i<TOE_GMAC_REG_SIZE; i++, dest_addr+=4, srcep++)
		writel(*srcep, dest_addr);
	
	srcep = (volatile u32 *)&toe_gmac_reg[1][0];
	dest_addr = TOE_GMAC1_BASE;
	for (i=0; i<TOE_GMAC_REG_SIZE; i++, dest_addr+=4, srcep++)
		writel(*srcep, dest_addr);
	
	srcep = (volatile u32 *)toe_global_reg;
	dest_addr = TOE_GLOBAL_BASE;
	for (i=0; i<TOE_GLOBAL_REG_SIZE; i++, dest_addr+=4, srcep++)
		writel(*srcep, dest_addr);
	
}

#ifdef CONFIG_SL351x_NAT
/*----------------------------------------------------------------------
* 	sl351x_nat_workaround_init
*----------------------------------------------------------------------*/
#define NAT_WORAROUND_DESC_POWER	(6)
#define NAT_WORAROUND_DESC_NUM		(2 << NAT_WORAROUND_DESC_POWER)
dma_addr_t sl351x_nat_workaround_desc_dma;
void sl351x_nat_workaround_init(void)
{
	unsigned int	desc_buf;
	
	desc_buf = (unsigned int)DMA_MALLOC((NAT_WORAROUND_DESC_NUM * sizeof(GMAC_RXDESC_T)),
						(dma_addr_t *)&sl351x_nat_workaround_desc_dma) ;
	memset((void *)desc_buf, 0, NAT_WORAROUND_DESC_NUM * sizeof(GMAC_RXDESC_T));
	
	// DMA Queue Base & Size
	writel((sl351x_nat_workaround_desc_dma & DMA_Q_BASE_MASK) | NAT_WORAROUND_DESC_POWER,
			TOE_GLOBAL_BASE + 0x4080);
	writel(0, TOE_GLOBAL_BASE + 0x4084);
}

/*----------------------------------------------------------------------
* 	sl351x_nat_workaround_handler
*----------------------------------------------------------------------*/
#ifndef NAT_WORKAROUND_BY_RESET_GMAC
static void sl351x_nat_workaround_handler(void)
{
	int					i;
	DMA_RWPTR_T			rwptr;
	GMAC_RXDESC_T		*desc_ptr;
	unsigned int		buf_ptr;
	TOE_INFO_T			*toe;
	GMAC_CONFIG0_T		config0;
	unsigned int		rwptr_addr;
	
	toe = (TOE_INFO_T *)&toe_private_data;

	// disable Rx of GMAC-0 & 1 
	config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
	config0.bits.dis_rx = 1;
	writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
	config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
	config0.bits.dis_rx = 1;
	writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
	
	// wait GMAC-0 HW Tx finished
	rwptr_addr = TOE_GMAC0_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		rwptr.bits32 = readl(rwptr_addr);
		if (rwptr.bits.rptr != rwptr.bits.wptr)
			return;	// wait the HW to send packets and release buffers
		rwptr_addr+=4;
	}
	rwptr_addr = TOE_GMAC1_DMA_BASE + GMAC_HW_TX_QUEUE0_PTR_REG;
	for (i=0; i<TOE_HW_TXQ_NUM; i++)
	{
		rwptr.bits32 = readl(rwptr_addr);
		if (rwptr.bits.rptr != rwptr.bits.wptr)
			return;	// wait the HW to send packets and release buffers
		rwptr_addr+=4;
	}
	
	// printk("sl351x_nat_workaround_handler %d\n", sl351x_nat_workaround_cnt);
	desc_ptr = (GMAC_RXDESC_T *)toe->hwfq_desc_base;
	buf_ptr = (unsigned int)toe->hwfq_buf_base_dma;
	for (i=0; i<TOE_HW_FREEQ_DESC_NUM; i++)
	{
		desc_ptr->word0.bits.buffer_size = HW_RX_BUF_SIZE;
		desc_ptr->word1.bits.sw_id = i;
		desc_ptr->word2.buf_adr = (unsigned int)buf_ptr;
		desc_ptr++;
		buf_ptr += HW_RX_BUF_SIZE;
	}
	rwptr.bits32 = readl(TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);
	rwptr.bits.wptr = RWPTR_RECEDE_ONE(rwptr.bits.rptr, TOE_HW_FREEQ_DESC_NUM);
	writel(rwptr.bits32, TOE_GLOBAL_BASE + GLOBAL_HWFQ_RWPTR_REG);
	writel(0, TOE_GLOBAL_BASE + 0x4084);
	
	// Enable Rx of GMAC-0 & 1 
	config0.bits32 = readl(TOE_GMAC0_BASE+GMAC_CONFIG0);
	config0.bits.dis_rx = 0;
	writel(config0.bits32, TOE_GMAC0_BASE+GMAC_CONFIG0);
	config0.bits32 = readl(TOE_GMAC1_BASE+GMAC_CONFIG0);
	config0.bits.dis_rx = 0;
	writel(config0.bits32, TOE_GMAC1_BASE+GMAC_CONFIG0);
}
#endif
#endif // CONFIG_SL351x_NAT
#ifdef ALPHA_RTL8366SR_MODULE
void register_igmp_callback(igmp_funp fun)
{
	igmp_snoop_input = fun;
}
void unregister_igmp_callback(void)
{
	igmp_snoop_input = NULL;
}
void gmac_set_wan_rgmii(int linkup,int speed,int duplex,int fc)
{
	GMAC_INFO_T  *tp;
	struct net_device *dev;
	tp = (GMAC_INFO_T *)&toe_private_data.gmac[GMAC_PORT1];/*wan is gmac 1*/
	
	if(!tp->dev) 
	{
		printk("error device\n");	
		return;
	}
	dev = tp->dev;
	//tp = dev->priv;
	
	if(tp->port_id != GMAC_PORT1)
	{
		printk("port error\n");	
		return ;	
	}
	
	//if(tp->port_id == GMAC_PORT1)/*WAN*/
	{
		GMAC_CONFIG0_T	config0,config0_mask;
		GMAC_STATUS_T   status, old_status;

		status.bits32 = old_status.bits32 = gmac_read_reg(tp->base_addr, GMAC_STATUS);
    	
        if( (linkup == 1))
    	{
            if(speed == 1000)
    	    {
    	    	//rtl8366sr_printk("Full_1000 \n");
    	    	if(status.bits.mii_rmii == 2)
    	    		status.bits.mii_rmii = 3;
    	        status.bits.duplex = 1;
    	        status.bits.speed = 2;
    	    }
    	    else
    	    {  
    	    	if(status.bits.mii_rmii == 3)
    	    		status.bits.mii_rmii = 2;
                if(speed == 100 && duplex == 1)
    	    	{
    	    	        //rtl8366sr_printk("Full_100 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 1;
    	    	}
                else if(speed == 100 && duplex == 0)
    	    	{
    	    	        //rtl8366sr_printk("Half_100 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 1;
    	    	}
                else if(speed == 10 && duplex == 1)
    	    	{
    	    	        //rtl8366sr_printk("Full_10 \n");
    	    	        status.bits.duplex = 1;
    	    	        status.bits.speed = 0;
    	    	}
                else if(speed == 10 && duplex == 0)
    	    	{
    	    	        //rtl8366sr_printk("Half_10 \n");
    	    	        status.bits.duplex = 0;
    	    	        status.bits.speed = 0;
    	    	}
    	    	else
    	    		printk("unknpw speed\n");
    	    }
    	    
    	    status.bits.link = LINK_UP; /* link up */
            if(fc == 1)
    	    {
    	        if (tp->flow_control_enable == 0)
				{
    	            config0.bits32 = 0;
    	            config0_mask.bits32 = 0;
    	            config0.bits.tx_fc_en = 1; /* enable tx flow control */
    	            config0.bits.rx_fc_en = 1; /* enable rx flow control */
    	            config0_mask.bits.tx_fc_en = 1;
    	            config0_mask.bits.rx_fc_en = 1;
    	            gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
    	           //rtl8366sr_printk("Flow Control Enable. \n");
    	        }
    	        tp->flow_control_enable = 1;
    	    }
    	    else
    	    {
    	        if (tp->flow_control_enable == 1)
    	        {
    	            config0.bits32 = 0;
    	            config0_mask.bits32 = 0;
    	            config0.bits.tx_fc_en = 0; /* disable tx flow control */
    	            config0.bits.rx_fc_en = 0; /* disable rx flow control */
    	            config0_mask.bits.tx_fc_en = 1;
    	            config0_mask.bits.rx_fc_en = 1;
    	            gmac_write_reg(tp->base_addr, GMAC_CONFIG0,config0.bits32,config0_mask.bits32);
    	            //rtl8366sr_printk("Flow Control Disable. \n");
    	        }
    	        tp->flow_control_enable = 0;
    	    }
    	
    	    if (tp->pre_phy_status == LINK_DOWN)
    	    {
    	        toe_gmac_enable_tx_rx(dev);
    	        tp->pre_phy_status = LINK_UP;
    	        netif_wake_queue(dev);
				set_bit(__LINK_STATE_START, &dev->state);
				storlink_ctl.link[tp->port_id] = 1;
				printk("GMAC-%d link up\n",tp->port_id);
    	    }
    	}
    	else
    	{
    	    status.bits.link = LINK_DOWN; /* link down */
    	    if (tp->pre_phy_status == LINK_UP)
    	    {
    	        toe_gmac_disable_tx_rx(dev);
    	        tp->pre_phy_status = LINK_DOWN;
    	        netif_stop_queue(dev);
				clear_bit(__LINK_STATE_START, &dev->state);
				storlink_ctl.link[tp->port_id] = 0;
				printk("GMAC-%d link down\n",tp->port_id);
    	    }
    	}
    	tp->full_duplex_status = status.bits.duplex;
		tp->speed_status = status.bits.speed;

		if (status.bits32 != old_status.bits32) 
    		gmac_write_reg(tp->base_addr, GMAC_STATUS, status.bits32, 0x0000007f);
    }
}
EXPORT_SYMBOL(gmac_set_wan_rgmii);
EXPORT_SYMBOL(register_igmp_callback);
EXPORT_SYMBOL(unregister_igmp_callback);
#endif
//#endif // SL351x_GMAC_WORKAROUND


