/****************************************************************************
* Copyright 2006 StorLink Semiconductors, Inc.  All rights reserved.
*--------------------------------------------------------------------------
* Name			: sl351x_gmac.h
* Description	: 
*		Define for device driver of Storlink SL351x network Engine
*
* Historych
*
*	Date		Writer		Description
*	-----------	-----------	-------------------------------------------------
*	08/22/2005	Gary Chen	Create and implement
*
****************************************************************************/
#ifndef _GMAC_SL351x_H
#define _GMAC_SL351x_H
#include <linux/skbuff.h>

//#define SL351x_GMAC_WORKAROUND		1
#undef BIG_ENDIAN
#define BIG_ENDIAN  				0
#define GMAC_DEBUG      			1
#define GMAC_NUM					2
//#define	L2_jumbo_frame				1
//#define GMAC_DEBUG_U					1

#define _PACKED_					__attribute__ ((aligned(1), packed))
 
#ifndef BIT
#define BIT(x)						(1 << (x))
#endif

#define REG32(addr)     			(*(volatile unsigned long  * const)(addr))

#define DMA_MALLOC(size,handle)		pci_alloc_consistent(NULL,size,handle) 
#define DMA_MFREE(mem,size,handle)	pci_free_consistent(NULL,size,mem,handle)			

// Define frame size
#define ETHER_ADDR_LEN				6
#define GMAC_MAX_ETH_FRAME_SIZE		1514
#define GMAC_TX_BUF_SIZE			((GMAC_MAX_ETH_FRAME_SIZE + 31) & (~31))
#define MAX_ISR_WORK        		20
#define ENET_HEADER_SIZE             14
#define MAXIMUM_ETHERNET_FRAME_SIZE  1518 /* With FCS */
#define MINIMUM_ETHERNET_FRAME_SIZE  64   /* With FCS */
#define ETHERNET_FCS_SIZE            4
#define MAX_JUMBO_FRAME_SIZE         2048  //

#ifdef	L2_jumbo_frame
#define SW_RX_BUF_SIZE				9234	// 2048 ,9234
#elif	defined(CONFIG_SL351X_IPSEC)
#define	SW_RX_BUF_SIZE				1664
#else
#define SW_RX_BUF_SIZE				1536	// 2048
#endif

#ifdef	CONFIG_SL351X_IPSEC
#define	HW_RX_BUF_SIZE				1664	// require slightly larger buffer size
#else
#define HW_RX_BUF_SIZE				1536	// 2048
#endif

#define GMAC_DEV_TX_TIMEOUT  		(10*HZ)			//add by CH
#ifdef	CONFIG_SL351X_IPSEC
#define	SKB_RESERVE_BYTES			64
#else
#define	SKB_RESERVE_BYTES			16
#endif

/**********************************************************************
 * Base Register
 **********************************************************************/
#define TOE_BASE					(IO_ADDRESS(SL2312_TOE_BASE))
#define GMAC_GLOBAL_BASE_ADDR       (IO_ADDRESS(SL2312_GLOBAL_BASE))

#define TOE_GLOBAL_BASE				(TOE_BASE + 0x0000)
#define TOE_NONTOE_QUE_HDR_BASE		(TOE_BASE + 0x2000)
#define TOE_TOE_QUE_HDR_BASE		(TOE_BASE + 0x3000)
#define TOE_V_BIT_BASE				(TOE_BASE + 0x4000)
#define TOE_A_BIT_BASE				(TOE_BASE + 0x6000)
#define TOE_GMAC0_DMA_BASE			(TOE_BASE + 0x8000)
#define TOE_GMAC0_BASE				(TOE_BASE + 0xA000)
#define TOE_GMAC1_DMA_BASE			(TOE_BASE + 0xC000)
#define TOE_GMAC1_BASE				(TOE_BASE + 0xE000)

/**********************************************************************
 * Queue ID
 **********************************************************************/
#define TOE_SW_FREE_QID				0x00
#define TOE_HW_FREE_QID				0x01
#define TOE_GMAC0_SW_TXQ0_QID		0x02
#define TOE_GMAC0_SW_TXQ1_QID		0x03
#define TOE_GMAC0_SW_TXQ2_QID		0x04
#define TOE_GMAC0_SW_TXQ3_QID		0x05
#define TOE_GMAC0_SW_TXQ4_QID		0x06
#define TOE_GMAC0_SW_TXQ5_QID		0x07
#define TOE_GMAC0_HW_TXQ0_QID		0x08
#define TOE_GMAC0_HW_TXQ1_QID		0x09
#define TOE_GMAC0_HW_TXQ2_QID		0x0A
#define TOE_GMAC0_HW_TXQ3_QID		0x0B
#define TOE_GMAC1_SW_TXQ0_QID		0x12
#define TOE_GMAC1_SW_TXQ1_QID		0x13
#define TOE_GMAC1_SW_TXQ2_QID		0x14
#define TOE_GMAC1_SW_TXQ3_QID		0x15
#define TOE_GMAC1_SW_TXQ4_QID		0x16
#define TOE_GMAC1_SW_TXQ5_QID		0x17
#define TOE_GMAC1_HW_TXQ0_QID		0x18
#define TOE_GMAC1_HW_TXQ1_QID		0x19
#define TOE_GMAC1_HW_TXQ2_QID		0x1A
#define TOE_GMAC1_HW_TXQ3_QID		0x1B
#define TOE_GMAC0_DEFAULT_QID		0x20
#define TOE_GMAC1_DEFAULT_QID		0x21
#define TOE_CLASSIFICATION_QID(x)	(0x22 + x)	// 0x22 ~ 0x2F
#define TOE_TOE_QID(x)				(0x40 + x)	// 0x40 ~ 0x7F

/**********************************************************************
 * TOE DMA Queue Number should be 2^n, n = 6...12
 * TOE DMA Queues are the following queue types:
 *		SW Free Queue, HW Free Queue, 
 *		GMAC 0/1 SW TX Q0-5, and GMAC 0/1 HW TX Q0-5
 * They have same descriptor numbers.
 * The base address and descriptor number are configured at
 * DMA Queues Descriptor Ring Base Address/Size Register (offset 0x0004)
 **********************************************************************/
#define TOE_SW_FREEQ_DESC_POWER		8
#define TOE_SW_FREEQ_DESC_NUM		(1<<TOE_SW_FREEQ_DESC_POWER)
#define TOE_HW_FREEQ_DESC_POWER		8
#define TOE_HW_FREEQ_DESC_NUM		(1<<TOE_HW_FREEQ_DESC_POWER)
#define TOE_GMAC0_SWTXQ_DESC_POWER	8
#define TOE_GMAC0_SWTXQ_DESC_NUM	(1<<TOE_GMAC0_SWTXQ_DESC_POWER)
#define TOE_GMAC0_HWTXQ_DESC_POWER	8
#define TOE_GMAC0_HWTXQ_DESC_NUM	(1<<TOE_GMAC0_HWTXQ_DESC_POWER)
#define TOE_GMAC1_SWTXQ_DESC_POWER	8
#define TOE_GMAC1_SWTXQ_DESC_NUM	(1<<TOE_GMAC1_SWTXQ_DESC_POWER)
#define TOE_GMAC1_HWTXQ_DESC_POWER	8
#define TOE_GMAC1_HWTXQ_DESC_NUM	(1<<TOE_GMAC1_HWTXQ_DESC_POWER)
#define TOE_DEFAULT_Q0_DESC_POWER	10
#define TOE_DEFAULT_Q0_DESC_NUM		(1<<TOE_DEFAULT_Q0_DESC_POWER)
#define TOE_DEFAULT_Q1_DESC_POWER	10
#define TOE_DEFAULT_Q1_DESC_NUM		(1<<TOE_DEFAULT_Q1_DESC_POWER)
#define TOE_TOE_DESC_POWER			8
#define TOE_TOE_DESC_NUM			(1<<TOE_TOE_DESC_POWER)
#define TOE_CLASS_DESC_POWER		10
#define TOE_CLASS_DESC_NUM			(1<<TOE_CLASS_DESC_POWER)
#define TOE_INTR_DESC_POWER			8
#define TOE_INTR_DESC_NUM			(1<<TOE_INTR_DESC_POWER)

#define TOE_TOE_QUEUE_MAX			64
#define TOE_TOE_QUEUE_NUM			64
#define TOE_CLASS_QUEUE_MAX			14
#define TOE_CLASS_QUEUE_NUM			14
#define TOE_INTR_QUEUE_MAX			4
#define TOE_INTR_QUEUE_NUM			4
#define TOE_SW_TXQ_MAX				6
#define TOE_SW_TXQ_NUM				1
#define TOE_HW_TXQ_MAX				4
#define TOE_HW_TXQ_NUM				4
#define _max(x,y)					((x>y) ? x :y)
#define TX_DESC_NUM					_max(TOE_GMAC0_SWTXQ_DESC_NUM, TOE_GMAC1_SWTXQ_DESC_NUM)

#define RWPTR_ADVANCE_ONE(x, max)	((x == (max -1)) ? 0 : x+1)
#define RWPTR_RECEDE_ONE(x, max)	((x == 0) ? (max -1) : x-1)
#define SET_WPTR(addr, data)		(*(volatile u16 * const)((u32)(addr)+2) = (u16)data)
#define SET_RPTR(addr, data)		(*(volatile u16 * const)((u32)(addr)) = (u16)data)

/**********************************************************************
 * Global registers
 * #define TOE_GLOBAL_BASE			(TOE_BASE + 0x0000)
 * Base 0x60000000
 **********************************************************************/
#define GLOBAL_TOE_VERSION_REG			0x0000
#define GLOBAL_SW_FREEQ_BASE_SIZE_REG	0x0004
#define GLOBAL_HW_FREEQ_BASE_SIZE_REG	0x0008
#define GLOBAL_DMA_SKB_SIZE_REG			0x0010
#define GLOBAL_SWFQ_RWPTR_REG			0x0014
#define GLOBAL_HWFQ_RWPTR_REG			0x0018
#define GLOBAL_INTERRUPT_STATUS_0_REG	0x0020
#define GLOBAL_INTERRUPT_ENABLE_0_REG	0x0024
#define GLOBAL_INTERRUPT_SELECT_0_REG	0x0028
#define GLOBAL_INTERRUPT_STATUS_1_REG	0x0030
#define GLOBAL_INTERRUPT_ENABLE_1_REG	0x0034
#define GLOBAL_INTERRUPT_SELECT_1_REG	0x0038
#define GLOBAL_INTERRUPT_STATUS_2_REG	0x0040
#define GLOBAL_INTERRUPT_ENABLE_2_REG	0x0044
#define GLOBAL_INTERRUPT_SELECT_2_REG	0x0048
#define GLOBAL_INTERRUPT_STATUS_3_REG	0x0050
#define GLOBAL_INTERRUPT_ENABLE_3_REG	0x0054
#define GLOBAL_INTERRUPT_SELECT_3_REG	0x0058
#define GLOBAL_INTERRUPT_STATUS_4_REG	0x0060
#define GLOBAL_INTERRUPT_ENABLE_4_REG	0x0064
#define GLOBAL_INTERRUPT_SELECT_4_REG	0x0068
#define GLOBAL_HASH_TABLE_BASE_REG		0x006C
#define GLOBAL_QUEUE_THRESHOLD_REG		0x0070

/**********************************************************************
 * GMAC 0/1 DMA/TOE register
 * #define TOE_GMAC0_DMA_BASE		(TOE_BASE + 0x8000)
 * #define TOE_GMAC1_DMA_BASE		(TOE_BASE + 0xC000)
 * Base 0x60008000 or 0x6000C000
 **********************************************************************/
#define GMAC_DMA_CTRL_REG				0x0000
#define GMAC_TX_WEIGHTING_CTRL_0_REG	0x0004
#define GMAC_TX_WEIGHTING_CTRL_1_REG	0x0008
#define GMAC_SW_TX_QUEUE0_PTR_REG		0x000C
#define GMAC_SW_TX_QUEUE1_PTR_REG		0x0010
#define GMAC_SW_TX_QUEUE2_PTR_REG		0x0014
#define GMAC_SW_TX_QUEUE3_PTR_REG		0x0018
#define GMAC_SW_TX_QUEUE4_PTR_REG		0x001C
#define GMAC_SW_TX_QUEUE5_PTR_REG		0x0020
#define GMAC_HW_TX_QUEUE0_PTR_REG		0x0024
#define GMAC_HW_TX_QUEUE1_PTR_REG		0x0028
#define GMAC_HW_TX_QUEUE2_PTR_REG		0x002C
#define GMAC_HW_TX_QUEUE3_PTR_REG		0x0030
#define GMAC_DMA_TX_FIRST_DESC_REG		0x0038
#define GMAC_DMA_TX_CURR_DESC_REG		0x003C
#define GMAC_DMA_TX_DESC_WORD0_REG		0x0040
#define GMAC_DMA_TX_DESC_WORD1_REG		0x0044
#define GMAC_DMA_TX_DESC_WORD2_REG		0x0048
#define GMAC_DMA_TX_DESC_WORD3_REG		0x004C
#define GMAC_SW_TX_QUEUE_BASE_REG		0x0050
#define GMAC_HW_TX_QUEUE_BASE_REG		0x0054
#define GMAC_DMA_RX_FIRST_DESC_REG		0x0058
#define GMAC_DMA_RX_CURR_DESC_REG		0x005C
#define GMAC_DMA_RX_DESC_WORD0_REG		0x0060
#define GMAC_DMA_RX_DESC_WORD1_REG		0x0064
#define GMAC_DMA_RX_DESC_WORD2_REG		0x0068
#define GMAC_DMA_RX_DESC_WORD3_REG		0x006C
#define GMAC_HASH_ENGINE_REG0			0x0070
#define GMAC_HASH_ENGINE_REG1			0x0074
#define GMAC_MR0CR0						0x0078 	// matching rule 0 Control register 0
#define GMAC_MR0CR1						0x007C	// matching rule 0 Control register 1
#define GMAC_MR0CR2						0x0080	// matching rule 0 Control register 2
#define GMAC_MR1CR0						0x0084	// matching rule 1 Control register 0
#define GMAC_MR1CR1						0x0088	// matching rule 1 Control register 1
#define GMAC_MR1CR2						0x008C	// matching rule 1 Control register 2
#define GMAC_MR2CR0						0x0090	// matching rule 2 Control register 0
#define GMAC_MR2CR1						0x0094	// matching rule 2 Control register 1
#define GMAC_MR2CR2						0x0098	// matching rule 2 Control register 2
#define GMAC_MR3CR0						0x009C	// matching rule 3 Control register 0
#define GMAC_MR3CR1						0x00A0	// matching rule 3 Control register 1
#define GMAC_MR3CR2						0x00A4	// matching rule 3 Control register 2
#define GMAC_SPR0						0x00A8	// Support Protocol Regsister 0
#define GMAC_SPR1						0x00AC	// Support Protocol Regsister 1
#define GMAC_SPR2						0x00B0	// Support Protocol Regsister 2
#define GMAC_SPR3						0x00B4	// Support Protocol Regsister 3
#define GMAC_SPR4						0x00B8	// Support Protocol Regsister 4
#define GMAC_SPR5						0x00BC	// Support Protocol Regsister 5
#define GMAC_SPR6						0x00C0	// Support Protocol Regsister 6
#define GMAC_SPR7						0x00C4	// Support Protocol Regsister 7
#define GMAC_AHB_WEIGHT_REG				0x00C8	// GMAC Hash/Rx/Tx AHB Weighting register

/**********************************************************************
 * TOE GMAC 0/1 register
 * #define TOE_GMAC0_BASE				(TOE_BASE + 0xA000)
 * #define TOE_GMAC1_BASE				(TOE_BASE + 0xE000)
 * Base 0x6000A000 or 0x6000E000
 **********************************************************************/
enum GMAC_REGISTER {
	GMAC_STA_ADD0 	= 0x0000,
	GMAC_STA_ADD1	= 0x0004,
	GMAC_STA_ADD2	= 0x0008,
	GMAC_RX_FLTR	= 0x000c,
	GMAC_MCAST_FIL0 = 0x0010,
	GMAC_MCAST_FIL1 = 0x0014,
	GMAC_CONFIG0	= 0x0018,
	GMAC_CONFIG1	= 0x001c,
	GMAC_CONFIG2	= 0x0020,
	GMAC_CONFIG3	= 0x0024,
	GMAC_RESERVED	= 0x0028,
	GMAC_STATUS		= 0x002c,
	GMAC_IN_DISCARDS= 0x0030,
	GMAC_IN_ERRORS  = 0x0034,
	GMAC_IN_MCAST   = 0x0038,
	GMAC_IN_BCAST   = 0x003c,
	GMAC_IN_MAC1    = 0x0040,	// for STA 1 MAC Address
	GMAC_IN_MAC2    = 0x0044	// for STA 2 MAC Address
};
/**********************************************************************
 * TOE version Register (offset 0x0000)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit
	{
#if (BIG_ENDIAN==1)
		
		unsigned int reserved		: 15;	// bit 31:17
		unsigned int v_bit_mode		: 1;	// bit 16		1: 128-entry
		unsigned int device_id		: 12;	// bit 15:4 	Device ID
		unsigned int revision_id	: 4;	// bit  3:0 	Revision ID
#else
		unsigned int revision_id	: 4;	// bit  3:0 	Revision ID
		unsigned int device_id		: 12;	// bit 15:4 	Device ID
		unsigned int v_bit_mode		: 1;	// bit 16		1: 128-entry
		unsigned int reserved		: 15;	// bit 31:17
#endif
	} bits;
} TOE_VERSION_T;


/**********************************************************************
 * DMA Queues description Ring Base Address/Size Register (offset 0x0004)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int base_size;
} DMA_Q_BASE_SIZE_T;
#define DMA_Q_BASE_MASK 	(~0x0f)

/**********************************************************************
 * DMA SKB Buffer register (offset 0x0008)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0008
	{
#if (BIG_ENDIAN==1)
		
		unsigned int hw_skb_size	: 16;	// bit 31:16	HW Free poll SKB Size
		unsigned int sw_skb_size	: 16;	// bit 15:0 	SW Free poll SKB Size
#else
		unsigned int sw_skb_size	: 16;	// bit 15:0 	SW Free poll SKB Size
		unsigned int hw_skb_size	: 16;	// bit 31:16	HW Free poll SKB Size
#endif
	} bits;
} DMA_SKB_SIZE_T;

/**********************************************************************
 * DMA SW Free Queue Read/Write Pointer Register (offset 0x000C)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_000c
	{
#if (BIG_ENDIAN==1)
		
		unsigned int wptr			: 16;	// bit 31:16 	Write Ptr, RW
		unsigned int rptr			: 16;	// bit 15:0		Read Ptr, RO
#else
		unsigned int rptr			: 16;	// bit 15:0		Read Ptr, RO
		unsigned int wptr			: 16;	// bit 31:16 	Write Ptr, RW
#endif
	} bits;
} DMA_RWPTR_T;

/**********************************************************************
 * DMA HW Free Queue Read/Write Pointer Register (offset 0x0010)
 **********************************************************************/
// see DMA_RWPTR_T structure

/**********************************************************************
 * Interrupt Status Register 0 	(offset 0x0020)
 * Interrupt Mask Register 0 	(offset 0x0024)
 * Interrupt Select Register 0 	(offset 0x0028)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0020
	{
#if (BIG_ENDIAN==1)
		unsigned int txDerr1		: 1;	// bit 31	GMAC1 AHB Bus Error while Tx
		unsigned int txPerr1		: 1;	// bit 30	GMAC1 Tx Descriptor Protocol Error
		unsigned int txDerr0		: 1;	// bit 29	GMAC0 AHB Bus Error while Tx
		unsigned int txPerr0		: 1;	// bit 28	GMAC0 Tx Descriptor Protocol Error
		unsigned int rxDerr1		: 1;	// bit 27	GMAC1 AHB Bus Error while Rx
		unsigned int rxPerr1		: 1;	// bit 26	GMAC1 Rx Descriptor Protocol Error
		unsigned int rxDerr0		: 1;	// bit 25	GMAC0 AHB Bus Error while Rx
		unsigned int rxPerr0		: 1;	// bit 24	GMAC0 Rx Descriptor Protocol Error
		unsigned int swtq15_fin		: 1;	// bit 23	GMAC1 SW Tx Queue 5 Finish Interrupt
		unsigned int swtq14_fin		: 1;	// bit 22	GMAC1 SW Tx Queue 4 Finish Interrupt
		unsigned int swtq13_fin		: 1;	// bit 21	GMAC1 SW Tx Queue 3 Finish Interrupt
		unsigned int swtq12_fin		: 1;	// bit 20	GMAC1 SW Tx Queue 2 Finish Interrupt
		unsigned int swtq11_fin		: 1;	// bit 19	GMAC1 SW Tx Queue 1 Finish Interrupt
		unsigned int swtq10_fin		: 1;	// bit 18	GMAC1 SW Tx Queue 0 Finish Interrupt
		unsigned int swtq05_fin		: 1;	// bit 17	GMAC0 SW Tx Queue 5 Finish Interrupt
		unsigned int swtq04_fin		: 1;	// bit 16	GMAC0 SW Tx Queue 4 Finish Interrupt
		unsigned int swtq03_fin		: 1;	// bit 15	GMAC0 SW Tx Queue 3 Finish Interrupt
		unsigned int swtq02_fin		: 1;	// bit 14	GMAC0 SW Tx Queue 2 Finish Interrupt
		unsigned int swtq01_fin		: 1;	// bit 13	GMAC0 SW Tx Queue 1 Finish Interrupt
		unsigned int swtq00_fin		: 1;	// bit 12	GMAC0 SW Tx Queue 0 Finish Interrupt
		unsigned int swtq15_eof		: 1;	// bit 11	GMAC1 SW Tx Queue 5 EOF Interrupt
		unsigned int swtq14_eof		: 1;	// bit 10	GMAC1 SW Tx Queue 4 EOF Interrupt
		unsigned int swtq13_eof		: 1;	// bit 9	GMAC1 SW Tx Queue 3 EOF Interrupt
		unsigned int swtq12_eof		: 1;	// bit 8	GMAC1 SW Tx Queue 2 EOF Interrupt
		unsigned int swtq11_eof		: 1;	// bit 7	GMAC1 SW Tx Queue 1 EOF Interrupt
		unsigned int swtq10_eof		: 1;	// bit 6	GMAC1 SW Tx Queue 0 EOF Interrupt
		unsigned int swtq05_eof		: 1;	// bit 5	GMAC0 SW Tx Queue 5 EOF Interrupt
		unsigned int swtq04_eof		: 1;	// bit 4	GMAC0 SW Tx Queue 4 EOF Interrupt
		unsigned int swtq03_eof		: 1;	// bit 3	GMAC0 SW Tx Queue 3 EOF Interrupt
		unsigned int swtq02_eof		: 1;	// bit 2	GMAC0 SW Tx Queue 2 EOF Interrupt
		unsigned int swtq01_eof		: 1;	// bit 1	GMAC0 SW Tx Queue 1 EOF Interrupt
		unsigned int swtq00_eof		: 1;	// bit 0	GMAC0 SW Tx Queue 0 EOF Interrupt
#else
		unsigned int swtq00_eof		: 1;	// bit 0	GMAC0 SW Tx Queue 0 EOF Interrupt
		unsigned int swtq01_eof		: 1;	// bit 1	GMAC0 SW Tx Queue 1 EOF Interrupt
		unsigned int swtq02_eof		: 1;	// bit 2	GMAC0 SW Tx Queue 2 EOF Interrupt
		unsigned int swtq03_eof		: 1;	// bit 3	GMAC0 SW Tx Queue 3 EOF Interrupt
		unsigned int swtq04_eof		: 1;	// bit 4	GMAC0 SW Tx Queue 4 EOF Interrupt
		unsigned int swtq05_eof		: 1;	// bit 5	GMAC0 SW Tx Queue 5 EOF Interrupt
		unsigned int swtq10_eof		: 1;	// bit 6	GMAC1 SW Tx Queue 0 EOF Interrupt
		unsigned int swtq11_eof		: 1;	// bit 7	GMAC1 SW Tx Queue 1 EOF Interrupt
		unsigned int swtq12_eof		: 1;	// bit 8	GMAC1 SW Tx Queue 2 EOF Interrupt
		unsigned int swtq13_eof		: 1;	// bit 9	GMAC1 SW Tx Queue 3 EOF Interrupt
		unsigned int swtq14_eof		: 1;	// bit 10	GMAC1 SW Tx Queue 4 EOF Interrupt
		unsigned int swtq15_eof		: 1;	// bit 11	GMAC1 SW Tx Queue 5 EOF Interrupt
		unsigned int swtq00_fin		: 1;	// bit 12	GMAC0 SW Tx Queue 0 Finish Interrupt
		unsigned int swtq01_fin		: 1;	// bit 13	GMAC0 SW Tx Queue 1 Finish Interrupt
		unsigned int swtq02_fin		: 1;	// bit 14	GMAC0 SW Tx Queue 2 Finish Interrupt
		unsigned int swtq03_fin		: 1;	// bit 15	GMAC0 SW Tx Queue 3 Finish Interrupt
		unsigned int swtq04_fin		: 1;	// bit 16	GMAC0 SW Tx Queue 4 Finish Interrupt
		unsigned int swtq05_fin		: 1;	// bit 17	GMAC0 SW Tx Queue 5 Finish Interrupt
		unsigned int swtq10_fin		: 1;	// bit 18	GMAC1 SW Tx Queue 0 Finish Interrupt
		unsigned int swtq11_fin		: 1;	// bit 19	GMAC1 SW Tx Queue 1 Finish Interrupt
		unsigned int swtq12_fin		: 1;	// bit 20	GMAC1 SW Tx Queue 2 Finish Interrupt
		unsigned int swtq13_fin		: 1;	// bit 21	GMAC1 SW Tx Queue 3 Finish Interrupt
		unsigned int swtq14_fin		: 1;	// bit 22	GMAC1 SW Tx Queue 4 Finish Interrupt
		unsigned int swtq15_fin		: 1;	// bit 23	GMAC1 SW Tx Queue 5 Finish Interrupt
		unsigned int rxPerr0		: 1;	// bit 24	GMAC0 Rx Descriptor Protocol Error
		unsigned int rxDerr0		: 1;	// bit 25	GMAC0 AHB Bus Error while Rx
		unsigned int rxPerr1		: 1;	// bit 26	GMAC1 Rx Descriptor Protocol Error
		unsigned int rxDerr1		: 1;	// bit 27	GMAC1 AHB Bus Error while Rx
		unsigned int txPerr0		: 1;	// bit 28	GMAC0 Tx Descriptor Protocol Error
		unsigned int txDerr0		: 1;	// bit 29	GMAC0 AHB Bus Error while Tx
		unsigned int txPerr1		: 1;	// bit 30	GMAC1 Tx Descriptor Protocol Error
		unsigned int txDerr1		: 1;	// bit 31	GMAC1 AHB Bus Error while Tx
#endif
	} bits;
} INTR_REG0_T;

#define GMAC1_TXDERR_INT_BIT		BIT(31)
#define GMAC1_TXPERR_INT_BIT		BIT(30)
#define GMAC0_TXDERR_INT_BIT		BIT(29)
#define GMAC0_TXPERR_INT_BIT		BIT(28)
#define GMAC1_RXDERR_INT_BIT		BIT(27)
#define GMAC1_RXPERR_INT_BIT		BIT(26)
#define GMAC0_RXDERR_INT_BIT		BIT(25)
#define GMAC0_RXPERR_INT_BIT		BIT(24)
#define GMAC1_SWTQ15_FIN_INT_BIT	BIT(23)
#define GMAC1_SWTQ14_FIN_INT_BIT	BIT(22)
#define GMAC1_SWTQ13_FIN_INT_BIT	BIT(21)
#define GMAC1_SWTQ12_FIN_INT_BIT	BIT(20)
#define GMAC1_SWTQ11_FIN_INT_BIT	BIT(19)
#define GMAC1_SWTQ10_FIN_INT_BIT	BIT(18)
#define GMAC0_SWTQ05_FIN_INT_BIT	BIT(17)
#define GMAC0_SWTQ04_FIN_INT_BIT	BIT(16)
#define GMAC0_SWTQ03_FIN_INT_BIT	BIT(15)
#define GMAC0_SWTQ02_FIN_INT_BIT	BIT(14)
#define GMAC0_SWTQ01_FIN_INT_BIT	BIT(13)
#define GMAC0_SWTQ00_FIN_INT_BIT	BIT(12)
#define GMAC1_SWTQ15_EOF_INT_BIT	BIT(11)
#define GMAC1_SWTQ14_EOF_INT_BIT	BIT(10)
#define GMAC1_SWTQ13_EOF_INT_BIT	BIT(9)
#define GMAC1_SWTQ12_EOF_INT_BIT	BIT(8)
#define GMAC1_SWTQ11_EOF_INT_BIT	BIT(7)
#define GMAC1_SWTQ10_EOF_INT_BIT	BIT(6)
#define GMAC0_SWTQ05_EOF_INT_BIT	BIT(5)
#define GMAC0_SWTQ04_EOF_INT_BIT	BIT(4)
#define GMAC0_SWTQ03_EOF_INT_BIT	BIT(3)
#define GMAC0_SWTQ02_EOF_INT_BIT	BIT(2)
#define GMAC0_SWTQ01_EOF_INT_BIT	BIT(1)
#define GMAC0_SWTQ00_EOF_INT_BIT	BIT(0)


/**********************************************************************
 * Interrupt Status Register 1 	(offset 0x0030)
 * Interrupt Mask Register 1 	(offset 0x0034)
 * Interrupt Select Register 1 	(offset 0x0038)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0030
	{
#if (BIG_ENDIAN==1)
		unsigned int toe_iq3_full	: 1;	// bit 31	TOE Interrupt Queue 3 Full Interrupt
		unsigned int toe_iq2_full	: 1;	// bit 30	TOE Interrupt Queue 2 Full Interrupt
		unsigned int toe_iq1_full	: 1;	// bit 29	TOE Interrupt Queue 1 Full Interrupt
		unsigned int toe_iq0_full	: 1;	// bit 28	TOE Interrupt Queue 0 Full Interrupt
		unsigned int toe_iq3_intr	: 1;	// bit 27	TOE Interrupt Queue 3 with Interrupts
		unsigned int toe_iq2_intr	: 1;	// bit 26	TOE Interrupt Queue 2 with Interrupts
		unsigned int toe_iq1_intr	: 1;	// bit 25	TOE Interrupt Queue 1 with Interrupts
		unsigned int toe_iq0_intr	: 1;	// bit 24	TOE Interrupt Queue 0 with Interrupts
		unsigned int hwtq13_eof		: 1;	// bit 23	GMAC1 HW Tx Queue3 EOF Interrupt
		unsigned int hwtq12_eof		: 1;	// bit 22	GMAC1 HW Tx Queue2 EOF Interrupt
		unsigned int hwtq11_eof		: 1;	// bit 21	GMAC1 HW Tx Queue1 EOF Interrupt
		unsigned int hwtq10_eof		: 1;	// bit 20	GMAC1 HW Tx Queue0 EOF Interrupt
		unsigned int hwtq03_eof		: 1;	// bit 19	GMAC0 HW Tx Queue3 EOF Interrupt
		unsigned int hwtq02_eof		: 1;	// bit 18	GMAC0 HW Tx Queue2 EOF Interrupt
		unsigned int hwtq01_eof		: 1;	// bit 17	GMAC0 HW Tx Queue1 EOF Interrupt
		unsigned int hwtq00_eof		: 1;	// bit 16	GMAC0 HW Tx Queue0 EOF Interrupt
		unsigned int class_rx		: 14;	// bit 15:2	Classification Queue Rx Interrupt
		unsigned int default_q1_eof	: 1;	// bit 1	Default Queue 1 EOF Interrupt
		unsigned int default_q0_eof	: 1;	// bit 0	Default Queue 0 EOF Interrupt
#else
		unsigned int default_q0_eof	: 1;	// bit 0	Default Queue 0 EOF Interrupt
		unsigned int default_q1_eof	: 1;	// bit 1	Default Queue 1 EOF Interrupt
		unsigned int class_rx		: 14;	// bit 15:2	Classification Queue Rx Interrupt
		unsigned int hwtq00_eof		: 1;	// bit 16	GMAC0 HW Tx Queue0 EOF Interrupt
		unsigned int hwtq01_eof		: 1;	// bit 17	GMAC0 HW Tx Queue1 EOF Interrupt
		unsigned int hwtq02_eof		: 1;	// bit 18	GMAC0 HW Tx Queue2 EOF Interrupt
		unsigned int hwtq03_eof		: 1;	// bit 19	GMAC0 HW Tx Queue3 EOF Interrupt
		unsigned int hwtq10_eof		: 1;	// bit 20	GMAC1 HW Tx Queue0 EOF Interrupt
		unsigned int hwtq11_eof		: 1;	// bit 21	GMAC1 HW Tx Queue1 EOF Interrupt
		unsigned int hwtq12_eof		: 1;	// bit 22	GMAC1 HW Tx Queue2 EOF Interrupt
		unsigned int hwtq13_eof		: 1;	// bit 23	GMAC1 HW Tx Queue3 EOF Interrupt
		unsigned int toe_iq0_intr	: 1;	// bit 24	TOE Interrupt Queue 0 with Interrupts
		unsigned int toe_iq1_intr	: 1;	// bit 25	TOE Interrupt Queue 1 with Interrupts
		unsigned int toe_iq2_intr	: 1;	// bit 26	TOE Interrupt Queue 2 with Interrupts
		unsigned int toe_iq3_intr	: 1;	// bit 27	TOE Interrupt Queue 3 with Interrupts
		unsigned int toe_iq0_full	: 1;	// bit 28	TOE Interrupt Queue 0 Full Interrupt
		unsigned int toe_iq1_full	: 1;	// bit 29	TOE Interrupt Queue 1 Full Interrupt
		unsigned int toe_iq2_full	: 1;	// bit 30	TOE Interrupt Queue 2 Full Interrupt
		unsigned int toe_iq3_full	: 1;	// bit 31	TOE Interrupt Queue 3 Full Interrupt
#endif
	} bits;
} INTR_REG1_T;

#define TOE_IQ3_FULL_INT_BIT		BIT(31)
#define TOE_IQ2_FULL_INT_BIT		BIT(30)
#define TOE_IQ1_FULL_INT_BIT		BIT(29)
#define TOE_IQ0_FULL_INT_BIT		BIT(28)
#define TOE_IQ3_INT_BIT				BIT(27)
#define TOE_IQ2_INT_BIT				BIT(26)
#define TOE_IQ1_INT_BIT				BIT(25)
#define TOE_IQ0_INT_BIT				BIT(24)
#define GMAC1_HWTQ13_EOF_INT_BIT	BIT(23)
#define GMAC1_HWTQ12_EOF_INT_BIT	BIT(22)
#define GMAC1_HWTQ11_EOF_INT_BIT	BIT(21)
#define GMAC1_HWTQ10_EOF_INT_BIT	BIT(20)
#define GMAC0_HWTQ03_EOF_INT_BIT	BIT(19)
#define GMAC0_HWTQ02_EOF_INT_BIT	BIT(18)
#define GMAC0_HWTQ01_EOF_INT_BIT	BIT(17)
#define GMAC0_HWTQ00_EOF_INT_BIT	BIT(16)
#define CLASS_RX_INT_BIT(x)			BIT((x+2))
#define DEFAULT_Q1_INT_BIT			BIT(1)
#define DEFAULT_Q0_INT_BIT			BIT(0)

#define TOE_IQ_INT_BITS				(TOE_IQ0_INT_BIT | TOE_IQ1_INT_BIT | \
		               	 			TOE_IQ2_INT_BIT | TOE_IQ3_INT_BIT)
#define	TOE_IQ_FULL_BITS			(TOE_IQ0_FULL_INT_BIT | TOE_IQ1_FULL_INT_BIT | \
		                	 		TOE_IQ2_FULL_INT_BIT | TOE_IQ3_FULL_INT_BIT)
#define	TOE_IQ_ALL_BITS				(TOE_IQ_INT_BITS | TOE_IQ_FULL_BITS)
#define TOE_CLASS_RX_INT_BITS		0xfffc

/**********************************************************************
 * Interrupt Status Register 2 	(offset 0x0040)
 * Interrupt Mask Register 2 	(offset 0x0044)
 * Interrupt Select Register 2 	(offset 0x0048)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0040
	{
#if (BIG_ENDIAN==1)
		unsigned int toe_q31_full	: 1;	// bit 31	TOE Queue 31 Full Interrupt
		unsigned int toe_q30_full	: 1;	// bit 30	TOE Queue 30 Full Interrupt
		unsigned int toe_q29_full	: 1;	// bit 29	TOE Queue 29 Full Interrupt
		unsigned int toe_q28_full	: 1;	// bit 28	TOE Queue 28 Full Interrupt
		unsigned int toe_q27_full	: 1;	// bit 27	TOE Queue 27 Full Interrupt
		unsigned int toe_q26_full	: 1;	// bit 26	TOE Queue 26 Full Interrupt
		unsigned int toe_q25_full	: 1;	// bit 25	TOE Queue 25 Full Interrupt
		unsigned int toe_q24_full	: 1;	// bit 24	TOE Queue 24 Full Interrupt
		unsigned int toe_q23_full	: 1;	// bit 23	TOE Queue 23 Full Interrupt
		unsigned int toe_q22_full	: 1;	// bit 22	TOE Queue 22 Full Interrupt
		unsigned int toe_q21_full	: 1;	// bit 21	TOE Queue 21 Full Interrupt
		unsigned int toe_q20_full	: 1;	// bit 20	TOE Queue 20 Full Interrupt
		unsigned int toe_q19_full	: 1;	// bit 19	TOE Queue 19 Full Interrupt
		unsigned int toe_q18_full	: 1;	// bit 18	TOE Queue 18 Full Interrupt
		unsigned int toe_q17_full	: 1;	// bit 17	TOE Queue 17 Full Interrupt
		unsigned int toe_q16_full	: 1;	// bit 16	TOE Queue 16 Full Interrupt
		unsigned int toe_q15_full	: 1;	// bit 15	TOE Queue 15 Full Interrupt
		unsigned int toe_q14_full	: 1;	// bit 14	TOE Queue 14 Full Interrupt
		unsigned int toe_q13_full	: 1;	// bit 13	TOE Queue 13 Full Interrupt
		unsigned int toe_q12_full	: 1;	// bit 12	TOE Queue 12 Full Interrupt
		unsigned int toe_q11_full	: 1;	// bit 11	TOE Queue 11 Full Interrupt
		unsigned int toe_q10_full	: 1;	// bit 10	TOE Queue 10 Full Interrupt
		unsigned int toe_q9_full	: 1;	// bit 9	TOE Queue 9 Full Interrupt
		unsigned int toe_q8_full	: 1;	// bit 8	TOE Queue 8 Full Interrupt
		unsigned int toe_q7_full	: 1;	// bit 7	TOE Queue 7 Full Interrupt
		unsigned int toe_q6_full	: 1;	// bit 6	TOE Queue 6 Full Interrupt
		unsigned int toe_q5_full	: 1;	// bit 5	TOE Queue 5 Full Interrupt
		unsigned int toe_q4_full	: 1;	// bit 4	TOE Queue 4 Full Interrupt
		unsigned int toe_q3_full	: 1;	// bit 3	TOE Queue 3 Full Interrupt
		unsigned int toe_q2_full	: 1;	// bit 2	TOE Queue 2 Full Interrupt
		unsigned int toe_q1_full	: 1;	// bit 1	TOE Queue 1 Full Interrupt
		unsigned int toe_q0_full	: 1;	// bit 0	TOE Queue 0 Full Interrupt
#else
		unsigned int toe_q0_full	: 1;	// bit 0	TOE Queue 0 Full Interrupt
		unsigned int toe_q1_full	: 1;	// bit 1	TOE Queue 1 Full Interrupt
		unsigned int toe_q2_full	: 1;	// bit 2	TOE Queue 2 Full Interrupt
		unsigned int toe_q3_full	: 1;	// bit 3	TOE Queue 3 Full Interrupt
		unsigned int toe_q4_full	: 1;	// bit 4	TOE Queue 4 Full Interrupt
		unsigned int toe_q5_full	: 1;	// bit 5	TOE Queue 5 Full Interrupt
		unsigned int toe_q6_full	: 1;	// bit 6	TOE Queue 6 Full Interrupt
		unsigned int toe_q7_full	: 1;	// bit 7	TOE Queue 7 Full Interrupt
		unsigned int toe_q8_full	: 1;	// bit 8	TOE Queue 8 Full Interrupt
		unsigned int toe_q9_full	: 1;	// bit 9	TOE Queue 9 Full Interrupt
		unsigned int toe_q10_full	: 1;	// bit 10	TOE Queue 10 Full Interrupt
		unsigned int toe_q11_full	: 1;	// bit 11	TOE Queue 11 Full Interrupt
		unsigned int toe_q12_full	: 1;	// bit 12	TOE Queue 12 Full Interrupt
		unsigned int toe_q13_full	: 1;	// bit 13	TOE Queue 13 Full Interrupt
		unsigned int toe_q14_full	: 1;	// bit 14	TOE Queue 14 Full Interrupt
		unsigned int toe_q15_full	: 1;	// bit 15	TOE Queue 15 Full Interrupt
		unsigned int toe_q16_full	: 1;	// bit 16	TOE Queue 16 Full Interrupt
		unsigned int toe_q17_full	: 1;	// bit 17	TOE Queue 17 Full Interrupt
		unsigned int toe_q18_full	: 1;	// bit 18	TOE Queue 18 Full Interrupt
		unsigned int toe_q19_full	: 1;	// bit 19	TOE Queue 19 Full Interrupt
		unsigned int toe_q20_full	: 1;	// bit 20	TOE Queue 20 Full Interrupt
		unsigned int toe_q21_full	: 1;	// bit 21	TOE Queue 21 Full Interrupt
		unsigned int toe_q22_full	: 1;	// bit 22	TOE Queue 22 Full Interrupt
		unsigned int toe_q23_full	: 1;	// bit 23	TOE Queue 23 Full Interrupt
		unsigned int toe_q24_full	: 1;	// bit 24	TOE Queue 24 Full Interrupt
		unsigned int toe_q25_full	: 1;	// bit 25	TOE Queue 25 Full Interrupt
		unsigned int toe_q26_full	: 1;	// bit 26	TOE Queue 26 Full Interrupt
		unsigned int toe_q27_full	: 1;	// bit 27	TOE Queue 27 Full Interrupt
		unsigned int toe_q28_full	: 1;	// bit 28	TOE Queue 28 Full Interrupt
		unsigned int toe_q29_full	: 1;	// bit 29	TOE Queue 29 Full Interrupt
		unsigned int toe_q30_full	: 1;	// bit 30	TOE Queue 30 Full Interrupt
		unsigned int toe_q31_full	: 1;	// bit 31	TOE Queue 31 Full Interrupt
#endif
	} bits;
} INTR_REG2_T;

#define TOE_QL_FULL_INT_BIT(x)		BIT(x)

/**********************************************************************
 * Interrupt Status Register 3 	(offset 0x0050)
 * Interrupt Mask Register 3 	(offset 0x0054)
 * Interrupt Select Register 3 	(offset 0x0058)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0050
	{
#if (BIG_ENDIAN==1)
		unsigned int toe_q63_full	: 1;	// bit 63	TOE Queue 63 Full Interrupt
		unsigned int toe_q62_full	: 1;	// bit 62	TOE Queue 62 Full Interrupt
		unsigned int toe_q61_full	: 1;	// bit 61	TOE Queue 61 Full Interrupt
		unsigned int toe_q60_full	: 1;	// bit 60	TOE Queue 60 Full Interrupt
		unsigned int toe_q59_full	: 1;	// bit 59	TOE Queue 59 Full Interrupt
		unsigned int toe_q58_full	: 1;	// bit 58	TOE Queue 58 Full Interrupt
		unsigned int toe_q57_full	: 1;	// bit 57	TOE Queue 57 Full Interrupt
		unsigned int toe_q56_full	: 1;	// bit 56	TOE Queue 56 Full Interrupt
		unsigned int toe_q55_full	: 1;	// bit 55	TOE Queue 55 Full Interrupt
		unsigned int toe_q54_full	: 1;	// bit 54	TOE Queue 54 Full Interrupt
		unsigned int toe_q53_full	: 1;	// bit 53	TOE Queue 53 Full Interrupt
		unsigned int toe_q52_full	: 1;	// bit 52	TOE Queue 52 Full Interrupt
		unsigned int toe_q51_full	: 1;	// bit 51	TOE Queue 51 Full Interrupt
		unsigned int toe_q50_full	: 1;	// bit 50	TOE Queue 50 Full Interrupt
		unsigned int toe_q49_full	: 1;	// bit 49	TOE Queue 49 Full Interrupt
		unsigned int toe_q48_full	: 1;	// bit 48	TOE Queue 48 Full Interrupt
		unsigned int toe_q47_full	: 1;	// bit 47	TOE Queue 47 Full Interrupt
		unsigned int toe_q46_full	: 1;	// bit 46	TOE Queue 46 Full Interrupt
		unsigned int toe_q45_full	: 1;	// bit 45	TOE Queue 45 Full Interrupt
		unsigned int toe_q44_full	: 1;	// bit 44	TOE Queue 44 Full Interrupt
		unsigned int toe_q43_full	: 1;	// bit 43	TOE Queue 43 Full Interrupt
		unsigned int toe_q42_full	: 1;	// bit 42	TOE Queue 42 Full Interrupt
		unsigned int toe_q41_full	: 1;	// bit 41	TOE Queue 41 Full Interrupt
		unsigned int toe_q40_full	: 1;	// bit 40	TOE Queue 40 Full Interrupt
		unsigned int toe_q39_full	: 1;	// bit 39 	TOE Queue 39 Full Interrupt
		unsigned int toe_q38_full	: 1;	// bit 38	TOE Queue 38 Full Interrupt
		unsigned int toe_q37_full	: 1;	// bit 37	TOE Queue 37 Full Interrupt
		unsigned int toe_q36_full	: 1;	// bit 36	TOE Queue 36 Full Interrupt
		unsigned int toe_q35_full	: 1;	// bit 35	TOE Queue 35 Full Interrupt
		unsigned int toe_q34_full	: 1;	// bit 34	TOE Queue 34 Full Interrupt
		unsigned int toe_q33_full	: 1;	// bit 33	TOE Queue 33 Full Interrupt
		unsigned int toe_q32_full	: 1;	// bit 32	TOE Queue 32 Full Interrupt
#else
		unsigned int toe_q32_full	: 1;	// bit 32	TOE Queue 32 Full Interrupt
		unsigned int toe_q33_full	: 1;	// bit 33	TOE Queue 33 Full Interrupt
		unsigned int toe_q34_full	: 1;	// bit 34	TOE Queue 34 Full Interrupt
		unsigned int toe_q35_full	: 1;	// bit 35	TOE Queue 35 Full Interrupt
		unsigned int toe_q36_full	: 1;	// bit 36	TOE Queue 36 Full Interrupt
		unsigned int toe_q37_full	: 1;	// bit 37	TOE Queue 37 Full Interrupt
		unsigned int toe_q38_full	: 1;	// bit 38	TOE Queue 38 Full Interrupt
		unsigned int toe_q39_full	: 1;	// bit 39	TOE Queue 39 Full Interrupt
		unsigned int toe_q40_full	: 1;	// bit 40	TOE Queue 40 Full Interrupt
		unsigned int toe_q41_full	: 1;	// bit 41	TOE Queue 41 Full Interrupt
		unsigned int toe_q42_full	: 1;	// bit 42	TOE Queue 42 Full Interrupt
		unsigned int toe_q43_full	: 1;	// bit 43	TOE Queue 43 Full Interrupt
		unsigned int toe_q44_full	: 1;	// bit 44	TOE Queue 44 Full Interrupt
		unsigned int toe_q45_full	: 1;	// bit 45	TOE Queue 45 Full Interrupt
		unsigned int toe_q46_full	: 1;	// bit 46	TOE Queue 46 Full Interrupt
		unsigned int toe_q47_full	: 1;	// bit 47	TOE Queue 47 Full Interrupt
		unsigned int toe_q48_full	: 1;	// bit 48	TOE Queue 48 Full Interrupt
		unsigned int toe_q49_full	: 1;	// bit 49	TOE Queue 49 Full Interrupt
		unsigned int toe_q50_full	: 1;	// bit 50	TOE Queue 50 Full Interrupt
		unsigned int toe_q51_full	: 1;	// bit 51	TOE Queue 51 Full Interrupt
		unsigned int toe_q52_full	: 1;	// bit 52	TOE Queue 52 Full Interrupt
		unsigned int toe_q53_full	: 1;	// bit 53	TOE Queue 53 Full Interrupt
		unsigned int toe_q54_full	: 1;	// bit 54	TOE Queue 54 Full Interrupt
		unsigned int toe_q55_full	: 1;	// bit 55	TOE Queue 55 Full Interrupt
		unsigned int toe_q56_full	: 1;	// bit 56	TOE Queue 56 Full Interrupt
		unsigned int toe_q57_full	: 1;	// bit 57	TOE Queue 57 Full Interrupt
		unsigned int toe_q58_full	: 1;	// bit 58	TOE Queue 58 Full Interrupt
		unsigned int toe_q59_full	: 1;	// bit 59	TOE Queue 59 Full Interrupt
		unsigned int toe_q60_full	: 1;	// bit 60	TOE Queue 60 Full Interrupt
		unsigned int toe_q61_full	: 1;	// bit 61	TOE Queue 61 Full Interrupt
		unsigned int toe_q62_full	: 1;	// bit 62	TOE Queue 62 Full Interrupt
		unsigned int toe_q63_full	: 1;	// bit 63	TOE Queue 63 Full Interrupt
#endif
	} bits;
} INTR_REG3_T;

#define TOE_QH_FULL_INT_BIT(x)		BIT(x-32)

/**********************************************************************
 * Interrupt Status Register 4 	(offset 0x0060)
 * Interrupt Mask Register 4 	(offset 0x0064)
 * Interrupt Select Register 4 	(offset 0x0068)
 **********************************************************************/
typedef union
{
	unsigned char byte;
	struct bit_0060
	{
#if (BIG_ENDIAN==1)
		unsigned char reserved		: 1;	// 
		unsigned char cnt_full 		: 1;	// MIB counters half full interrupt
		unsigned char rx_pause_on	: 1;	// received pause on frame interrupt  
		unsigned char tx_pause_on	: 1;	// transmit pause on frame interrupt 
		unsigned char rx_pause_off  : 1;	// received pause off frame interrupt  
		unsigned char tx_pause_off	: 1;	// received pause off frame interrupt 
		unsigned char rx_overrun	: 1;    // GMAC Rx FIFO overrun interrupt 
		unsigned char status_changed: 1;	// Status Changed Intr for RGMII Mode
#else
		unsigned char status_changed: 1;	// Status Changed Intr for RGMII Mode
		unsigned char rx_overrun	: 1;   // GMAC Rx FIFO overrun interrupt 
		unsigned char tx_pause_off	: 1;	// received pause off frame interrupt 
		unsigned char rx_pause_off  : 1;	// received pause off frame interrupt  
		unsigned char tx_pause_on	: 1;	// transmit pause on frame interrupt 
		unsigned char rx_pause_on	: 1;	// received pause on frame interrupt  
		unsigned char cnt_full 		: 1;	// MIB counters half full interrupt
		unsigned char reserved		: 1;	// 
#endif
	} _PACKED_ bits;
} _PACKED_ GMAC_INTR_T;
	
typedef union
{
	unsigned int bits32;
	struct bit_0060_2 
	{
#if (BIG_ENDIAN==1)
		GMAC_INTR_T		gmac1;
		GMAC_INTR_T		gmac0;
		unsigned int	class_qf_int: 14;	// bit 15:2 Classification Rx Queue13-0 Full Intr.
		unsigned int    hwfq_empty	: 1;	// bit 1	Hardware Free Queue Empty Intr.
		unsigned int    swfq_empty	: 1;	// bit 0	Software Free Queue Empty Intr.
#else
#endif
		unsigned int    swfq_empty	: 1;	// bit 0	Software Free Queue Empty Intr.
		unsigned int    hwfq_empty	: 1;	// bit 1	Hardware Free Queue Empty Intr.
		unsigned int	class_qf_int: 14;	// bit 15:2 Classification Rx Queue13-0 Full Intr.
		GMAC_INTR_T		gmac0;
		GMAC_INTR_T		gmac1;
	} bits;
} INTR_REG4_T;

#define GMAC1_RESERVED_INT_BIT		BIT(31)
#define GMAC1_MIB_INT_BIT			BIT(30)
#define GMAC1_RX_PAUSE_ON_INT_BIT	BIT(29)
#define GMAC1_TX_PAUSE_ON_INT_BIT	BIT(28)
#define GMAC1_RX_PAUSE_OFF_INT_BIT	BIT(27)
#define GMAC1_TX_PAUSE_OFF_INT_BIT	BIT(26)
#define GMAC1_RX_OVERRUN_INT_BIT	BIT(25)
#define GMAC1_STATUS_CHANGE_INT_BIT	BIT(24)
#define GMAC0_RESERVED_INT_BIT		BIT(23)
#define GMAC0_MIB_INT_BIT			BIT(22)
#define GMAC0_RX_PAUSE_ON_INT_BIT	BIT(21)
#define GMAC0_TX_PAUSE_ON_INT_BIT	BIT(20)
#define GMAC0_RX_PAUSE_OFF_INT_BIT	BIT(19)
#define GMAC0_TX_PAUSE_OFF_INT_BIT	BIT(18)
#define GMAC0_RX_OVERRUN_INT_BIT	BIT(17)
#define GMAC0_STATUS_CHANGE_INT_BIT	BIT(16)
#define CLASS_RX_FULL_INT_BIT(x)	BIT((x+2))
#define HWFQ_EMPTY_INT_BIT			BIT(1)
#define SWFQ_EMPTY_INT_BIT			BIT(0)

#if 1
#define GMAC0_INT_BITS				(GMAC0_MIB_INT_BIT)
#define GMAC1_INT_BITS				(GMAC1_MIB_INT_BIT)
#else
#define GMAC0_INT_BITS				(GMAC0_RESERVED_INT_BIT | GMAC0_MIB_INT_BIT | \
									 GMAC0_RX_PAUSE_ON_INT_BIT | GMAC0_TX_PAUSE_ON_INT_BIT |	\
									 GMAC0_RX_PAUSE_OFF_INT_BIT | GMAC0_TX_PAUSE_OFF_INT_BIT | 	\
									 GMAC0_RX_OVERRUN_INT_BIT | GMAC0_STATUS_CHANGE_INT_BIT)
#define GMAC1_INT_BITS				(GMAC1_RESERVED_INT_BIT | GMAC1_MIB_INT_BIT | \
									 GMAC1_RX_PAUSE_ON_INT_BIT | GMAC1_TX_PAUSE_ON_INT_BIT |	\
									 GMAC1_RX_PAUSE_OFF_INT_BIT | GMAC1_TX_PAUSE_OFF_INT_BIT | 	\
									 GMAC1_RX_OVERRUN_INT_BIT | GMAC1_STATUS_CHANGE_INT_BIT)
#endif
									 
#define CLASS_RX_FULL_INT_BITS		0xfffc									 

/**********************************************************************
 * GLOBAL_QUEUE_THRESHOLD_REG 	(offset 0x0070)
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_0070_2 
	{
#if (BIG_ENDIAN==1)
		unsigned int	toe_class	: 8;	// 31:24
		unsigned int	intrq		: 8;	// 23:16
		unsigned int    hwfq_empty	: 8;	// 15:8		Hardware Free Queue Empty Threshold
		unsigned int    swfq_empty	: 8;	//  7:0  	Software Free Queue Empty Threshold
#else
#endif
		unsigned int    swfq_empty	: 8;	//  7:0  	Software Free Queue Empty Threshold
		unsigned int    hwfq_empty	: 8;	// 15:8		Hardware Free Queue Empty Threshold
		unsigned int	intrq		: 8;	// 23:16
		unsigned int	toe_class	: 8;	// 31:24
	} bits;
} QUEUE_THRESHOLD_T;


/**********************************************************************
 * GMAC DMA Control Register	
 * GMAC0 offset 0x8000
 * GMAC1 offset 0xC000
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8000 
	{
#if (BIG_ENDIAN==1)
		unsigned int    rd_enable		: 1;	// bit 31	Rx DMA Enable
		unsigned int    td_enable		: 1;	// bit 30	Tx DMA Enable
		unsigned int    loopback		: 1;	// bit 29	Loopback TxDMA to RxDMA
		unsigned int    drop_small_ack	: 1;	// bit 28	1: Drop, 0: Accept
		unsigned int	reserved		: 10;	// bit 27:18
		unsigned int	rd_insert_bytes	: 2;	// bit 17:16
		unsigned int	rd_prot			: 4;	// bit 15:12 DMA Protection Control
		unsigned int	rd_burst_size	: 2;	// bit 11:10 DMA max burst size for every AHB request
		unsigned int	rd_bus		    : 2;	// bit 9:8 	Peripheral Bus Width
		unsigned int	td_prot			: 4;	// bit 7:4  TxDMA protection control
		unsigned int	td_burst_size	: 2;	// bit 3:2	TxDMA max burst size for every AHB request
		unsigned int	td_bus		    : 2;	// bit 1:0  Peripheral Bus Width
#else
		unsigned int	td_bus		    : 2;	// bit 1:0  Peripheral Bus Width
		unsigned int	td_burst_size	: 2;	// bit 3:2	TxDMA max burst size for every AHB request
		unsigned int	td_prot			: 4;	// bit 7:4  TxDMA protection control
		unsigned int	rd_bus		    : 2;	// bit 9:8 	Peripheral Bus Width
		unsigned int	rd_burst_size	: 2;	// bit 11:10 DMA max burst size for every AHB request
		unsigned int	rd_prot			: 4;	// bit 15:12 DMA Protection Control
		unsigned int	rd_insert_bytes	: 2;	// bit 17:16
		unsigned int	reserved		: 10;	// bit 27:18
		unsigned int    drop_small_ack	: 1;	// bit 28	1: Drop, 0: Accept
		unsigned int    loopback		: 1;	// bit 29	Loopback TxDMA to RxDMA
		unsigned int    td_enable		: 1;	// bit 30	Tx DMA Enable
		unsigned int    rd_enable		: 1;	// bit 31	Rx DMA Enable
#endif
	} bits;
} GMAC_DMA_CTRL_T;

/**********************************************************************
 * GMAC Tx Weighting Control Register 0	
 * GMAC0 offset 0x8004
 * GMAC1 offset 0xC004
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8004
	{
#if (BIG_ENDIAN==1)
		unsigned int    reserved		: 8;	// bit 31:24
		unsigned int    hw_tq3			: 6;	// bit 23:18	HW TX Queue 0
		unsigned int    hw_tq2			: 6;	// bit 17:12	HW TX Queue 1
		unsigned int    hw_tq1			: 6;	// bit 11:6		HW TX Queue 2
		unsigned int    hw_tq0			: 6;	// bit 5:0		HW TX Queue 3
#else
		unsigned int    hw_tq0			: 6;	// bit 5:0		HW TX Queue 3
		unsigned int    hw_tq1			: 6;	// bit 11:6		HW TX Queue 2
		unsigned int    hw_tq2			: 6;	// bit 17:12	HW TX Queue 1
		unsigned int    hw_tq3			: 6;	// bit 23:18	HW TX Queue 0
		unsigned int    reserved		: 8;	// bit 31:24
#endif
	} bits;
} GMAC_TX_WCR0_T;	// Weighting Control Register 0

/**********************************************************************
 * GMAC Tx Weighting Control Register 1	
 * GMAC0 offset 0x8008
 * GMAC1 offset 0xC008
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8008 
	{
#if (BIG_ENDIAN==1)
		unsigned int    reserved		: 2;	// bit 31:30
		unsigned int    sw_tq5			: 5;	// bit 29:25	SW TX Queue 5
		unsigned int    sw_tq4			: 5;	// bit 24:20	SW TX Queue 4
		unsigned int    sw_tq3			: 5;	// bit 19:15	SW TX Queue 3
		unsigned int    sw_tq2			: 5;	// bit 14:10	SW TX Queue 2
		unsigned int    sw_tq1			: 5;	// bit 9:5		SW TX Queue 1
		unsigned int    sw_tq0			: 5;	// bit 4:0		SW TX Queue 0
#else
		unsigned int    sw_tq0			: 5;	// bit 4:0		SW TX Queue 0
		unsigned int    sw_tq1			: 5;	// bit 9:5		SW TX Queue 1
		unsigned int    sw_tq2			: 5;	// bit 14:10	SW TX Queue 2
		unsigned int    sw_tq3			: 5;	// bit 19:15	SW TX Queue 3
		unsigned int    sw_tq4			: 5;	// bit 24:20	SW TX Queue 4
		unsigned int    sw_tq5			: 5;	// bit 29:25	SW TX Queue 5
		unsigned int    reserved		: 2;	// bit 31:30
#endif
	} bits;
} GMAC_TX_WCR1_T;	// Weighting Control Register 1

/**********************************************************************
 * Queue Read/Write Pointer
 * GMAC SW TX Queue 0~5 Read/Write Pointer register
 * GMAC0 offset 0x800C ~ 0x8020
 * GMAC1 offset 0xC00C ~ 0xC020
 * GMAC HW TX Queue 0~3 Read/Write Pointer register
 * GMAC0 offset 0x8024 ~ 0x8030
 * GMAC1 offset 0xC024 ~ 0xC030
 **********************************************************************/
// see DMA_RWPTR_T structure

/**********************************************************************
 * GMAC DMA Tx First Description Address Register
 * GMAC0 offset 0x8038
 * GMAC1 offset 0xC038
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8038
	{
#if (BIG_ENDIAN==1)
		unsigned int td_first_des_ptr	: 28;	// bit 31:4	first descriptor address 
		unsigned int td_busy			:  1;	// bit 3	1: TxDMA busy; 0: TxDMA idle
		unsigned int reserved			:  3;
#else
		unsigned int reserved			:  3;
		unsigned int td_busy			:  1;	// bit 3	1: TxDMA busy; 0: TxDMA idle
		unsigned int td_first_des_ptr	: 28;	// bit 31:4	first descriptor address 
#endif
	} bits;
} GMAC_TXDMA_FIRST_DESC_T;

/**********************************************************************
 * GMAC DMA Tx Current Description Address Register
 * GMAC0 offset 0x803C
 * GMAC1 offset 0xC03C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_803C
	{
#if (BIG_ENDIAN==1)
		unsigned int td_curr_desc_ptr	: 28;	// bit 31:4	current descriptor address 
		unsigned int reserved			:  4;
#else
		unsigned int reserved			:  4;
		unsigned int td_curr_desc_ptr	: 28;	// bit 31:4	current descriptor address 
#endif
	} bits;
} GMAC_TXDMA_CURR_DESC_T;

/**********************************************************************
 * GMAC DMA Tx Description Word 0 Register
 * GMAC0 offset 0x8040
 * GMAC1 offset 0xC040
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8040
	{
#if (BIG_ENDIAN==1)
		unsigned int reserved		: 1;	// bit 31
		unsigned int derr			: 1;	// bit 30	 data error during processing this descriptor
		unsigned int perr			: 1;	// bit 29	 protocol error during processing this descriptor
		unsigned int status_rvd		: 6;	// bit 28:23 Tx Status, Reserved bits
		unsigned int status_tx_ok	: 1;	// bit 22    Tx Status, 1: Successful 0: Failed
		unsigned int desc_count 	: 6;	// bit 21:16 number of descriptors used for the current frame
		unsigned int buffer_size 	: 16;	// bit 15:0  Transfer size
#else
		unsigned int buffer_size 	: 16;	// bit 15:0  Transfer size
		unsigned int desc_count 	: 6;	// bit 21:16 number of descriptors used for the current frame
		unsigned int status_tx_ok	: 1;	// bit 22    Tx Status, 1: Successful 0: Failed
		unsigned int status_rvd		: 6;	// bit 28:23 Tx Status, Reserved bits
		unsigned int perr			: 1;	// bit 29	 protocol error during processing this descriptor
		unsigned int derr			: 1;	// bit 30	 data error during processing this descriptor
		unsigned int reserved		: 1;	// bit 31
#endif
	} bits;
} GMAC_TXDESC_0_T;

/**********************************************************************
 * GMAC DMA Tx Description Word 1 Register
 * GMAC0 offset 0x8044
 * GMAC1 offset 0xC044
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct txdesc_word1
	{
#if (BIG_ENDIAN==1)
		unsigned int	reserved	: 9;	// bit 31:23 	Tx Flag, Reserved
		unsigned int	ip_fixed_len: 1;	// bit 22
		unsigned int	bypass_tss	: 1;	// bit 21
		unsigned int	udp_chksum	: 1;	// bit 20		UDP Checksum Enable
		unsigned int	tcp_chksum	: 1;	// bit 19		TCP Checksum Enable
		unsigned int	ipv6_enable	: 1;	// bit 18		IPV6 Tx Enable
		unsigned int	ip_chksum	: 1;	// bit 17		IPV4 Header Checksum Enable
		unsigned int	mtu_enable	: 1;	// bit 16		TSS segmentation use MTU setting
		unsigned int	byte_count	: 16;	// bit 15: 0	Tx Frame Byte Count
#else
		unsigned int	byte_count	: 16;	// bit 15: 0	Tx Frame Byte Count
		unsigned int	mtu_enable	: 1;	// bit 16		TSS segmentation use MTU setting
		unsigned int	ip_chksum	: 1;	// bit 17		IPV4 Header Checksum Enable
		unsigned int	ipv6_enable	: 1;	// bit 18		IPV6 Tx Enable
		unsigned int	tcp_chksum	: 1;	// bit 19		TCP Checksum Enable
		unsigned int	udp_chksum	: 1;	// bit 20		UDP Checksum Enable
		unsigned int	bypass_tss	: 1;	// bit 21
		unsigned int	ip_fixed_len: 1;	// bit 22
		unsigned int	reserved	: 9;	// bit 31:23 	Tx Flag, Reserved
#endif
	} bits;
} GMAC_TXDESC_1_T;

#define TSS_IP_FIXED_LEN_BIT	BIT(22)
#define TSS_UDP_CHKSUM_BIT		BIT(20)
#define TSS_TCP_CHKSUM_BIT		BIT(19)
#define TSS_IPV6_ENABLE_BIT		BIT(18)
#define TSS_IP_CHKSUM_BIT		BIT(17)
#define TSS_MTU_ENABLE_BIT		BIT(16)

/**********************************************************************
 * GMAC DMA Tx Description Word 2 Register
 * GMAC0 offset 0x8048
 * GMAC1 offset 0xC048
 **********************************************************************/
typedef union
{
	unsigned int	bits32;
	unsigned int 	buf_adr;
} GMAC_TXDESC_2_T;

/**********************************************************************
 * GMAC DMA Tx Description Word 3 Register
 * GMAC0 offset 0x804C
 * GMAC1 offset 0xC04C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct txdesc_word3
	{
#if (BIG_ENDIAN==1)
		unsigned int	sof_eof		: 2;	// bit 31:30 	11: only one, 10: first, 01: last, 00: linking
		unsigned int	eofie		: 1;	// bit 29		End of frame interrupt enable
		unsigned int	reserved	: 18;	// bit 28:11
		unsigned int	mtu_size	: 11;	// bit 10: 0	Tx Frame Byte Count
#else
		unsigned int	mtu_size	: 11;	// bit 10: 0	Tx Frame Byte Count
		unsigned int	reserved	: 18;	// bit 28:11
		unsigned int	eofie		: 1;	// bit 29		End of frame interrupt enable
		unsigned int	sof_eof		: 2;	// bit 31:30 	11: only one, 10: first, 01: last, 00: linking
#endif
	} bits;
} GMAC_TXDESC_3_T;
#define SOF_EOF_BIT_MASK	0x3fffffff
#define SOF_BIT				0x80000000
#define EOF_BIT				0x40000000
#define EOFIE_BIT			BIT(29)
#define MTU_SIZE_BIT_MASK	0x7ff	

/**********************************************************************
 * GMAC Tx Descriptor
 **********************************************************************/
typedef struct
{
	GMAC_TXDESC_0_T	word0;
	GMAC_TXDESC_1_T	word1;
	GMAC_TXDESC_2_T	word2;
	GMAC_TXDESC_3_T	word3;
} GMAC_TXDESC_T;


/**********************************************************************
 * GMAC DMA Rx First Description Address Register
 * GMAC0 offset 0x8058
 * GMAC1 offset 0xC058
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8058
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_first_des_ptr	: 28;	// bit 31:4 first descriptor address
		unsigned int rd_busy			:  1;	// bit 3	1-RxDMA busy; 0-RxDMA idle
		unsigned int reserved			:  3;	// bit 2:0
#else
		unsigned int reserved			:  3;	// bit 2:0
		unsigned int rd_busy			:  1;	// bit 3	1-RxDMA busy; 0-RxDMA idle
		unsigned int rd_first_des_ptr	: 28;	// bit 31:4 first descriptor address
#endif
	} bits;
} GMAC_RXDMA_FIRST_DESC_T;

/**********************************************************************
 * GMAC DMA Rx Current Description Address Register
 * GMAC0 offset 0x805C
 * GMAC1 offset 0xC05C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_805C
	{
#if (BIG_ENDIAN==1)
		unsigned int rd_curr_des_ptr	: 28;	// bit 31:4 current descriptor address
		unsigned int reserved			:  4;	// bit 3:0
#else
		unsigned int reserved			:  4;	// bit 3:0
		unsigned int rd_curr_des_ptr	: 28;	// bit 31:4 current descriptor address
#endif
	} bits;
} GMAC_RXDMA_CURR_DESC_T;

/**********************************************************************
 * GMAC DMA Rx Description Word 0 Register
 * GMAC0 offset 0x8060
 * GMAC1 offset 0xC060
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8060
	{
#if (BIG_ENDIAN==1)
		unsigned int drop			: 1;	// bit 31	 TOE/CIS Queue Full dropped packet to default queue
		unsigned int derr			: 1;	// bit 30	 data error during processing this descriptor
		unsigned int perr			: 1;	// bit 29	 protocol error during processing this descriptor
		unsigned int chksum_status	: 3;	// bit 28:26 Check Sum Status
		unsigned int status			: 4;	// bit 24:22 Status of rx frame
		unsigned int desc_count 	: 6;	// bit 21:16 number of descriptors used for the current frame
		unsigned int buffer_size 	: 16;	// bit 15:0  number of descriptors used for the current frame
#else
		unsigned int buffer_size 	: 16;	// bit 15:0  number of descriptors used for the current frame
		unsigned int desc_count 	: 6;	// bit 21:16 number of descriptors used for the current frame
		unsigned int status			: 4;	// bit 24:22 Status of rx frame
		unsigned int chksum_status	: 3;	// bit 28:26 Check Sum Status
		unsigned int perr			: 1;	// bit 29	 protocol error during processing this descriptor
		unsigned int derr			: 1;	// bit 30	 data error during processing this descriptor
		unsigned int drop			: 1;	// bit 31	 TOE/CIS Queue Full dropped packet to default queue
#endif
	} bits;
} GMAC_RXDESC_0_T;

#define		GMAC_RXDESC_0_T_derr				BIT(30)
#define		GMAC_RXDESC_0_T_perr				BIT(29)
#define		GMAC_RXDESC_0_T_chksum_status(x)	BIT((x+26))
#define		GMAC_RXDESC_0_T_status(x)			BIT((x+22))
#define		GMAC_RXDESC_0_T_desc_count(x)		BIT((x+16))

#define	RX_CHKSUM_IP_UDP_TCP_OK			0
#define	RX_CHKSUM_IP_OK_ONLY			1
#define	RX_CHKSUM_NONE					2
#define	RX_CHKSUM_IP_ERR_UNKNOWN		4
#define	RX_CHKSUM_IP_ERR				5
#define	RX_CHKSUM_TCP_UDP_ERR			6
#define RX_CHKSUM_NUM					8

#define RX_STATUS_GOOD_FRAME			0
#define RX_STATUS_TOO_LONG_GOOD_CRC		1
#define RX_STATUS_RUNT_FRAME			2
#define RX_STATUS_SFD_NOT_FOUND			3
#define RX_STATUS_CRC_ERROR				4
#define RX_STATUS_TOO_LONG_BAD_CRC		5
#define RX_STATUS_ALIGNMENT_ERROR		6
#define RX_STATUS_TOO_LONG_BAD_ALIGN	7
#define RX_STATUS_RX_ERR				8
#define RX_STATUS_DA_FILTERED			9
#define RX_STATUS_BUFFER_FULL			10
#define RX_STATUS_NUM					16


/**********************************************************************
 * GMAC DMA Rx Description Word 1 Register
 * GMAC0 offset 0x8064
 * GMAC1 offset 0xC064
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct rxdesc_word1
	{
#if (BIG_ENDIAN==1)
		unsigned int	sw_id		: 16;	// bit 31:16	Software ID
		unsigned int	byte_count	: 16;	// bit 15: 0	Rx Frame Byte Count
#else
		unsigned int	byte_count	: 16;	// bit 15: 0	Rx Frame Byte Count
		unsigned int	sw_id		: 16;	// bit 31:16	Software ID
#endif
	} bits;
} GMAC_RXDESC_1_T;

/**********************************************************************
 * GMAC DMA Rx Description Word 2 Register
 * GMAC0 offset 0x8068
 * GMAC1 offset 0xC068
 **********************************************************************/
typedef union
{
	unsigned int	bits32;
	unsigned int	buf_adr;
} GMAC_RXDESC_2_T;

#define RX_INSERT_NONE		0
#define RX_INSERT_1_BYTE	1
#define RX_INSERT_2_BYTE	2
#define RX_INSERT_3_BYTE	3

#define RX_INSERT_BYTES		RX_INSERT_2_BYTE
/**********************************************************************
 * GMAC DMA Rx Description Word 3 Register
 * GMAC0 offset 0x806C
 * GMAC1 offset 0xC06C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct rxdesc_word3
	{
#if (BIG_ENDIAN==1)
		unsigned int	sof_eof		: 2;	// bit 31:30 	11: only one, 10: first, 01: last, 00: linking
		unsigned int	eofie		: 1;	// bit 29		End of frame interrupt enable
		unsigned int	ctrl_flag	: 1;	// bit 28 		Control Flag is present
		unsigned int	out_of_seq	: 1;	// bit 27		Out of Sequence packet
		unsigned int	option		: 1;	// bit 26		IPV4 option or IPV6 extension header
		unsigned int	abnormal	: 1;	// bit 25		abnormal case found
		unsigned int	dup_ack		: 1;	// bit 24		Duplicated ACK detected
		unsigned int	l7_offset	: 8;	// bit 23: 16	L7 data offset
		unsigned int	l4_offset	: 8;	// bit 15: 8	L4 data offset
		unsigned int	l3_offset	: 8;	// bit 7: 0		L3 data offset
#else
		unsigned int	l3_offset	: 8;	// bit 7: 0		L3 data offset
		unsigned int	l4_offset	: 8;	// bit 15: 8	L4 data offset
		unsigned int	l7_offset	: 8;	// bit 23: 16	L7 data offset
		unsigned int	dup_ack		: 1;	// bit 24		Duplicated ACK detected
		unsigned int	abnormal	: 1;	// bit 25		abnormal case found
		unsigned int	option		: 1;	// bit 26		IPV4 option or IPV6 extension header
		unsigned int	out_of_seq	: 1;	// bit 27		Out of Sequence packet
		unsigned int	ctrl_flag	: 1;	// bit 28 		Control Flag is present
		unsigned int	eofie		: 1;	// bit 29		End of frame interrupt enable
		unsigned int	sof_eof		: 2;	// bit 31:30 	11: only one, 10: first, 01: last, 00: linking
#endif
	} bits;
} GMAC_RXDESC_3_T;

/**********************************************************************
 * GMAC Rx Descriptor
 **********************************************************************/
typedef struct
{
	GMAC_RXDESC_0_T	word0;
	GMAC_RXDESC_1_T	word1;
	GMAC_RXDESC_2_T	word2;
	GMAC_RXDESC_3_T	word3;
} GMAC_RXDESC_T;

/**********************************************************************
 * GMAC Hash Engine Enable/Action Register 0 Offset Register
 * GMAC0 offset 0x8070
 * GMAC1 offset 0xC070
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8070
	{
#if (BIG_ENDIAN==1)
		unsigned int	mr1en		: 1;	// bit 31		Enable Matching Rule 1
		unsigned int	reserved1	: 1;	// bit 30	
		unsigned int	timing		: 3;	// bit 29:27			
		unsigned int	mr1_action	: 5;	// bit 26:22	Matching Rule 1 action offset
		unsigned int	mr1hel		: 6;	// bit 21:16	match rule 1 hash entry size
		unsigned int	mr0en		: 1;	// bit 15		Enable Matching Rule 0
		unsigned int	reserved0	: 4;	// bit 14:11	
		unsigned int	mr0_action	: 5;	// bit 10:6		Matching Rule 0 action offset
		unsigned int	mr0hel		: 6;	// bit 5:0		match rule 0 hash entry size
#else
		unsigned int	mr0hel		: 6;	// bit 5:0		match rule 0 hash entry size
		unsigned int	mr0_action	: 5;	// bit 10:6		Matching Rule 0 action offset
		unsigned int	reserved0	: 4;	// bit 14:11	
		unsigned int	mr0en		: 1;	// bit 15		Enable Matching Rule 0
		unsigned int	mr1hel		: 6;	// bit 21:16	match rule 1 hash entry size
		unsigned int	mr1_action	: 5;	// bit 26:22	Matching Rule 1 action offset
		unsigned int	timing		: 3;	// bit 29:27	
		unsigned int	reserved1	: 1;	// bit 30		
		unsigned int	mr1en		: 1;	// bit 31		Enable Matching Rule 1
#endif
	} bits;
} GMAC_HASH_ENABLE_REG0_T;
 
/**********************************************************************
 * GMAC Hash Engine Enable/Action Register 1 Offset Register
 * GMAC0 offset 0x8074
 * GMAC1 offset 0xC074
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8074
	{
#if (BIG_ENDIAN==1)
		unsigned int	mr3en		: 1;	// bit 31		Enable Matching Rule 3
		unsigned int	reserved3	: 4;	// bit 30:27	
		unsigned int	mr3_action	: 5;	// bit 26:22	Matching Rule 3 action offset
		unsigned int	mr3hel		: 6;	// bit 21:16	match rule 3 hash entry size
		unsigned int	mr2en		: 1;	// bit 15		Enable Matching Rule 2
		unsigned int	reserved2	: 4;	// bit 14:11	
		unsigned int	mr2_action	: 5;	// bit 10:6		Matching Rule 2 action offset
		unsigned int	mr2hel		: 6;	// bit 5:0		match rule 2 hash entry size
#else
		unsigned int	mr2hel		: 6;	// bit 5:0		match rule 2 hash entry size
		unsigned int	mr2_action	: 5;	// bit 10:6		Matching Rule 2 action offset
		unsigned int	reserved2	: 4;	// bit 14:11	
		unsigned int	mr2en		: 1;	// bit 15		Enable Matching Rule 2
		unsigned int	mr3hel		: 6;	// bit 21:16	match rule 3 hash entry size
		unsigned int	mr3_action	: 5;	// bit 26:22	Matching Rule 3 action offset
		unsigned int	reserved1	: 4;	// bit 30:27	
		unsigned int	mr3en		: 1;	// bit 31		Enable Matching Rule 3
#endif
	} bits;
} GMAC_HASH_ENABLE_REG1_T;
 
 
/**********************************************************************
 * GMAC Matching Rule Control Register 0
 * GMAC0 offset 0x8078
 * GMAC1 offset 0xC078
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_8078
	{
#if (BIG_ENDIAN==1)
		unsigned int	l2			: 1;	// bit 31		L2 matching enable
		unsigned int	l3			: 1;	// bit 30		L3 matching enable
		unsigned int	l4			: 1;	// bit 29		L4 matching enable
		unsigned int	l7			: 1;	// bit 28		L7 matching enable
		unsigned int	port		: 1;	// bit 27		PORT ID matching enable
		unsigned int	priority	: 3;	// bit 26:24	priority if multi-rules matched
		unsigned int	da			: 1;	// bit 23		MAC DA enable
		unsigned int	sa			: 1;	// bit 22		MAC SA enable
		unsigned int	ether_type	: 1;	// bit 21		Ethernet type enable
		unsigned int	vlan		: 1;	// bit 20		VLAN ID enable
		unsigned int	pppoe		: 1;	// bit 19		PPPoE Session ID enable
		unsigned int	reserved1	: 3;	// bit 18:16
		unsigned int	ip_version	: 1;	// bit 15		0: IPV4, 1: IPV6
		unsigned int	ip_hdr_len	: 1;	// bit 14		IPV4 Header length
		unsigned int	flow_lable	: 1;	// bit 13		IPV6 Flow label
		unsigned int	tos_traffic	: 1;	// bit 12		IPV4 TOS or IPV6 Traffice Class
		unsigned int	reserved2	: 4;	// bit 11:8		
		unsigned int	sprx		: 8;	// bit 7:0		Support Protocol Register 7:0
#else
		unsigned int	sprx		: 8;	// bit 7:0		Support Protocol Register 7:0
		unsigned int	reserved2	: 4;	// bit 11:8		
		unsigned int	tos_traffic	: 1;	// bit 12		IPV4 TOS or IPV6 Traffice Class
		unsigned int	flow_lable	: 1;	// bit 13		IPV6 Flow label
		unsigned int	ip_hdr_len	: 1;	// bit 14		IPV4 Header length
		unsigned int	ip_version	: 1;	// bit 15		0: IPV4, 1: IPV6
		unsigned int	reserved1	: 3;	// bit 18:16
		unsigned int	pppoe		: 1;	// bit 19		PPPoE Session ID enable
		unsigned int	vlan		: 1;	// bit 20		VLAN ID enable
		unsigned int	ether_type	: 1;	// bit 21		Ethernet type enable
		unsigned int	sa			: 1;	// bit 22		MAC SA enable
		unsigned int	da			: 1;	// bit 23		MAC DA enable
		unsigned int	priority	: 3;	// bit 26:24	priority if multi-rules matched
		unsigned int	port		: 1;	// bit 27		PORT ID matching enable
		unsigned int	l7			: 1;	// bit 28		L7 matching enable
		unsigned int	l4			: 1;	// bit 29		L4 matching enable
		unsigned int	l3			: 1;	// bit 30		L3 matching enable
		unsigned int	l2			: 1;	// bit 31		L2 matching enable
#endif
	} bits;
} GMAC_MRxCR0_T;

#define MR_L2_BIT			BIT(31)
#define MR_L3_BIT			BIT(30)
#define MR_L4_BIT			BIT(29)
#define MR_L7_BIT			BIT(28)
#define MR_PORT_BIT			BIT(27)
#define MR_PRIORITY_BIT		BIT(26)
#define MR_DA_BIT			BIT(23)
#define MR_SA_BIT			BIT(22)
#define MR_ETHER_TYPE_BIT	BIT(21)
#define MR_VLAN_BIT			BIT(20)
#define MR_PPPOE_BIT		BIT(19)
#define MR_IP_VER_BIT		BIT(15)
#define MR_IP_HDR_LEN_BIT	BIT(14)
#define MR_FLOW_LABLE_BIT	BIT(13)
#define MR_TOS_TRAFFIC_BIT	BIT(12)
#define MR_SPR_BIT(x)		BIT(x)
#define MR_SPR_BITS		0xff

/**********************************************************************
 * GMAC Matching Rule Control Register 1
 * GMAC0 offset 0x807C
 * GMAC1 offset 0xC07C
 **********************************************************************/
 typedef union
{
	unsigned int bits32;
	struct bit_807C
	{
#if (BIG_ENDIAN==1)
		unsigned int	sip			: 1;	// bit 31		Srce IP
		unsigned int	sip_netmask	: 7;	// bit 30:24	Srce IP net mask, number of mask bits
		unsigned int	dip			: 1;	// bit 23		Dest IP
		unsigned int	dip_netmask	: 7;	// bit 22:16	Dest IP net mask, number of mask bits
		unsigned int    l4_byte0_15	: 16;	// bit 15: 0  
#else
		unsigned int    l4_byte0_15	: 16;	// bit 15: 0  
		unsigned int	dip_netmask	: 7;	// bit 22:16	Dest IP net mask, number of mask bits
		unsigned int	dip			: 1;	// bit 23		Dest IP
		unsigned int	sip_netmask	: 7;	// bit 30:24	Srce IP net mask, number of mask bits
		unsigned int	sip			: 1;	// bit 31		Srce IP
#endif
	} bits;
} GMAC_MRxCR1_T;

/**********************************************************************
 * GMAC Matching Rule Control Register 2
 * GMAC0 offset 0x8080
 * GMAC1 offset 0xC080
 **********************************************************************/
 typedef union
{
	unsigned int bits32;
	struct bit_8080
	{
#if (BIG_ENDIAN==1)
		unsigned int    l4_byte16_24: 8;	// bit 31: 24  
		unsigned int    l7_byte0_23	: 24;	// bit 23:0  
#else
		unsigned int    l7_byte0_23	: 24;	// bit 23:0  
		unsigned int    l4_byte16_24: 8;	// bit 31: 24  
#endif
	} bits;
} GMAC_MRxCR2_T;

 
/**********************************************************************
 * GMAC Support registers
 * GMAC0 offset 0x80A8
 * GMAC1 offset 0xC0A8
 **********************************************************************/
 typedef union
{
	unsigned int bits32;
	struct bit_80A8
	{
#if (BIG_ENDIAN==1)
		unsigned int    reserved: 21;	// bit 31:11	  
		unsigned int    swap	: 3;	// bit 10:8		Swap 
		unsigned int    protocol: 8;	// bit 7:0		Supported protocol  
#else
		unsigned int    protocol: 8;	// bit 7:0		Supported protocol  
		unsigned int    swap	: 3;	// bit 10:8		Swap 
		unsigned int    reserved: 21;	// bit 31:11	  
#endif
	} bits;
} GMAC_SPR_T;

/**********************************************************************
 * GMAC_AHB_WEIGHT registers
 * GMAC0 offset 0x80C8
 * GMAC1 offset 0xC0C8
 **********************************************************************/
 typedef union
{
	unsigned int bits32;
	struct bit_80C8
	{
#if (BIG_ENDIAN==1)
		unsigned int    reserved		: 7;	// 31:25	  
		unsigned int    tqDV_threshold	: 5;	// 24:20 DMA TqCtrl to Start tqDV FIFO Threshold
		unsigned int    pre_req			: 5;	// 19:15 Rx Data Pre Request FIFO Threshold
		unsigned int    tx_weight		: 5;	// 14:10
		unsigned int    rx_weight		: 5;	// 9:5
		unsigned int    hash_weight		: 5;	// 4:0
#else
		unsigned int    hash_weight		: 5;	// 4:0
		unsigned int    rx_weight		: 5;	// 9:5
		unsigned int    tx_weight		: 5;	// 14:10
		unsigned int    pre_req			: 5;	// 19:15 Rx Data Pre Request FIFO Threshold
		unsigned int    tqDV_threshold	: 5;	// 24:20 DMA TqCtrl to Start tqDV FIFO Threshold
		unsigned int    reserved		: 7;	// 31:25	  
#endif
	} bits;
} GMAC_AHB_WEIGHT_T; 
/**********************************************************************
 * the register structure of GMAC
 **********************************************************************/

/**********************************************************************
 * GMAC RX FLTR
 * GMAC0 Offset 0xA00C
 * GMAC1 Offset 0xE00C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_000c
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 27;
		unsigned int error			:  1;	/* enable receive of all error frames */
		unsigned int promiscuous	:  1;   /* enable receive of all frames */
		unsigned int broadcast		:  1;	/* enable receive of broadcast frames */
		unsigned int multicast		:  1;	/* enable receive of multicast frames that pass multicast filter */
		unsigned int unicast		:  1;	/* enable receive of unicast frames that are sent to STA address */
#else
		unsigned int unicast		:  1;	/* enable receive of unicast frames that are sent to STA address */
		unsigned int multicast		:  1;	/* enable receive of multicast frames that pass multicast filter */
		unsigned int broadcast		:  1;	/* enable receive of broadcast frames */
		unsigned int promiscuous	:  1;   /* enable receive of all frames */
		unsigned int error			:  1;	/* enable receive of all error frames */
		unsigned int 				: 27;
#endif
	} bits;
} GMAC_RX_FLTR_T;

/**********************************************************************
 * GMAC Configuration 0
 * GMAC0 Offset 0xA018
 * GMAC1 Offset 0xE018
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_0018
	{
#if (BIG_ENDIAN==1)
		unsigned int reserved		:  2;	// 31
		unsigned int port1_chk_classq :  1;	// 29
		unsigned int port0_chk_classq :  1;	// 28
		unsigned int port1_chk_toeq	:  1;	// 27
		unsigned int port0_chk_toeq	:  1;	// 26
		unsigned int port1_chk_hwq	:  1;	// 25
		unsigned int port0_chk_hwq	:  1;	// 24
		unsigned int rx_err_detect  :  1;	// 23
		unsigned int ipv6_exthdr_order: 1;	// 22
		unsigned int rxc_inv		:  1;	// 21
		unsigned int rgmm_edge		:  1;	// 20
        unsigned int rx_tag_remove  :  1;   /* 19: Remove Rx VLAN tag */
        unsigned int ipv6_rx_chksum :  1;   /* 18: IPv6 RX Checksum enable */
        unsigned int ipv4_rx_chksum :  1;   /* 17: IPv4 RX Checksum enable */
        unsigned int rgmii_en       :  1;   /* 16: RGMII in-band status enable */
		unsigned int tx_fc_en		:  1;	/* 15: TX flow control enable */
		unsigned int rx_fc_en		:  1;	/* 14: RX flow control enable */
		unsigned int sim_test		:  1;	/* 13: speed up timers in simulation */
		unsigned int dis_col		:  1;	/* 12: disable 16 collisions abort function */
		unsigned int dis_bkoff		:  1;	/* 11: disable back-off function */
		unsigned int max_len		:  3;	/* 8-10 maximum receive frame length allowed */
		unsigned int adj_ifg		:  4;	/* 4-7: adjust IFG from 96+/-56 */
        unsigned int flow_ctrl      :  1;   /* 3: flow control also trigged by Rx queues */
		unsigned int loop_back		:  1;	/* 2: transmit data loopback enable */
		unsigned int dis_rx			:  1;	/* 1: disable receive */
		unsigned int dis_tx			:  1;	/* 0: disable transmit */
#else
		unsigned int dis_tx			:  1;	/* 0: disable transmit */
		unsigned int dis_rx			:  1;	/* 1: disable receive */
		unsigned int loop_back		:  1;	/* 2: transmit data loopback enable */
        unsigned int flow_ctrl      :  1;   /* 3: flow control also trigged by Rx queues */
		unsigned int adj_ifg		:  4;	/* 4-7: adjust IFG from 96+/-56 */
		unsigned int max_len		:  3;	/* 8-10 maximum receive frame length allowed */
		unsigned int dis_bkoff		:  1;	/* 11: disable back-off function */
		unsigned int dis_col		:  1;	/* 12: disable 16 collisions abort function */
		unsigned int sim_test		:  1;	/* 13: speed up timers in simulation */
		unsigned int rx_fc_en		:  1;	/* 14: RX flow control enable */
		unsigned int tx_fc_en		:  1;	/* 15: TX flow control enable */
        unsigned int rgmii_en       :  1;   /* 16: RGMII in-band status enable */
        unsigned int ipv4_rx_chksum :  1;   /* 17: IPv4 RX Checksum enable */
        unsigned int ipv6_rx_chksum :  1;   /* 18: IPv6 RX Checksum enable */
        unsigned int rx_tag_remove  :  1;   /* 19: Remove Rx VLAN tag */
		unsigned int rgmm_edge		:  1;	// 20
		unsigned int rxc_inv		:  1;	// 21
		unsigned int ipv6_exthdr_order: 1;	// 22
		unsigned int rx_err_detect  :  1;	// 23
		unsigned int port0_chk_hwq	:  1;	// 24
		unsigned int port1_chk_hwq	:  1;	// 25
		unsigned int port0_chk_toeq	:  1;	// 26
		unsigned int port1_chk_toeq	:  1;	// 27
		unsigned int port0_chk_classq :  1;	// 28
		unsigned int port1_chk_classq :  1;	// 29
		unsigned int reserved		:  2;	// 31
#endif
	} bits;
} GMAC_CONFIG0_T;

/**********************************************************************
 * GMAC Configuration 1
 * GMAC0 Offset 0xA01C
 * GMAC1 Offset 0xE01C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_001c
	{
#if (BIG_ENDIAN==1)
		unsigned int reserved		: 16;
		unsigned int rel_threshold	: 8;	/* flow control release threshold */
		unsigned int set_threshold	: 8; 	/* flow control set threshold */
#else
		unsigned int set_threshold	: 8; 	/* flow control set threshold */
		unsigned int rel_threshold	: 8;	/* flow control release threshold */
		unsigned int reserved		: 16;
#endif
	} bits;
} GMAC_CONFIG1_T;

#define GMAC_FLOWCTRL_SET_MAX		32
#define GMAC_FLOWCTRL_SET_MIN		0
#define GMAC_FLOWCTRL_RELEASE_MAX	32
#define GMAC_FLOWCTRL_RELEASE_MIN	0

/**********************************************************************
 * GMAC Configuration 2
 * GMAC0 Offset 0xA020
 * GMAC1 Offset 0xE020
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_0020
	{
#if (BIG_ENDIAN==1)
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
#else
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
#endif
	} bits;
} GMAC_CONFIG2_T;

/**********************************************************************
 * GMAC Configuration 3
 * GMAC0 Offset 0xA024
 * GMAC1 Offset 0xE024
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_0024
	{
#if (BIG_ENDIAN==1)
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
#else
		unsigned int set_threshold	: 16; 	/* flow control set threshold */
		unsigned int rel_threshold	: 16;	/* flow control release threshold */
#endif
	} bits;
} GMAC_CONFIG3_T;


/**********************************************************************
 * GMAC STATUS
 * GMAC0 Offset 0xA02C
 * GMAC1 Offset 0xE02C
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit1_002c
	{
#if (BIG_ENDIAN==1)
		unsigned int 				: 25;
		unsigned int mii_rmii		:  2;   /* PHY interface type */
		unsigned int reserved		:  1;	
		unsigned int duplex			:  1;	/* duplex mode */
		unsigned int speed			:  2;	/* link speed(00->2.5M 01->25M 10->125M) */
		unsigned int link			:  1;	/* link status */
#else
		unsigned int link			:  1;	/* link status */
		unsigned int speed			:  2;	/* link speed(00->2.5M 01->25M 10->125M) */
		unsigned int duplex			:  1;	/* duplex mode */
		unsigned int reserved		:  1;	
		unsigned int mii_rmii		:  2;   /* PHY interface type */
		unsigned int 				: 25;
#endif
	} bits;
} GMAC_STATUS_T;

#define GMAC_SPEED_10			0
#define GMAC_SPEED_100			1
#define GMAC_SPEED_1000			2

#define GMAC_PHY_MII			0
#define GMAC_PHY_GMII			1
#define GMAC_PHY_RGMII_100		2
#define GMAC_PHY_RGMII_1000		3

/**********************************************************************
 * Queue Header
 *	(1) TOE Queue Header
 *	(2) Non-TOE Queue Header
 *	(3) Interrupt Queue Header
 *
 * memory Layout
 *	TOE Queue Header
 *	 0x60003000 +---------------------------+ 0x0000	
 *				|     TOE Queue 0 Header	|
 *				|         8 * 4 Bytes	    |
 *				+---------------------------+ 0x0020
 *				|     TOE Queue 1 Header  	|
 *				|         8 * 4 Bytes		|
 *				+---------------------------+ 0x0040
 *				|   	......  			|
 *				|      						|
 *				+---------------------------+ 
 *
 *	Non TOE Queue Header
 *	 0x60002000 +---------------------------+ 0x0000	
 *				|   Default Queue 0 Header  |
 *				|         2 * 4 Bytes		|
 *				+---------------------------+ 0x0008
 *				|   Default Queue 1 Header	|
 *				|         2 * 4 Bytes		|
 *				+---------------------------+ 0x0010
 *				|   Classification Queue 0	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 
 *				|   Classification Queue 1	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ (n * 8 + 0x10)
 *				|   		...				|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ (13 * 8 + 0x10)
 *				|   Classification Queue 13	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 0x80
 * 				|      Interrupt Queue 0	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 
 * 				|      Interrupt Queue 1	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 
 * 				|      Interrupt Queue 2	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 
 * 				|      Interrupt Queue 3	|
 *				|      	  2 * 4 Bytes		|
 *				+---------------------------+ 
 *
 **********************************************************************/
#define TOE_QUEUE_HDR_ADDR(n)		(TOE_TOE_QUE_HDR_BASE + n * 32)
#define TOE_Q_HDR_AREA_END			(TOE_QUEUE_HDR_ADDR(TOE_TOE_QUEUE_MAX+1))
#define TOE_DEFAULT_Q0_HDR_BASE		(TOE_NONTOE_QUE_HDR_BASE + 0x00)
#define TOE_DEFAULT_Q1_HDR_BASE		(TOE_NONTOE_QUE_HDR_BASE + 0x08)
#define TOE_CLASS_Q_HDR_BASE		(TOE_NONTOE_QUE_HDR_BASE + 0x10)
#define TOE_INTR_Q_HDR_BASE			(TOE_NONTOE_QUE_HDR_BASE + 0x80)
#define INTERRUPT_QUEUE_HDR_ADDR(n)	(TOE_INTR_Q_HDR_BASE + n * 8)
#define NONTOE_Q_HDR_AREA_END		(INTERRUPT_QUEUE_HDR_ADDR(TOE_INTR_QUEUE_MAX+1))
/**********************************************************************
 * TOE Queue Header Word 0
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int base_size;
} TOE_QHDR0_T;

#define TOE_QHDR0_BASE_MASK 	(~0x0f)

/**********************************************************************
 * TOE Queue Header Word 1
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_qhdr1
	{
#if (BIG_ENDIAN==1)
		
		unsigned int wptr			: 16;	// bit 31:16
		unsigned int rptr			: 16;	// bit 15:0
#else
		unsigned int rptr			: 16;	// bit 15:0	
		unsigned int wptr			: 16;	// bit 31:16
#endif
	} bits;
} TOE_QHDR1_T;

/**********************************************************************
 * TOE Queue Header Word 2
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_qhdr2
	{
#if (BIG_ENDIAN==1)
		
		unsigned int usd			: 1;	// bit 31		0: if no data assembled yet
		unsigned int ctl			: 1;	// bit 30		1: have control flag bits (except ack)
		unsigned int osq			: 1;	// bit 29		1: out of sequence
		unsigned int sat			: 1;	// bit 28		1: SeqCnt > SeqThreshold, or AckCnt > AckThreshold
		unsigned int ip_opt			: 1;	// bit 27		1: have IPV4 option or IPV6 Extension header
		unsigned int tcp_opt		: 1;	// bit 26		1: Have TCP option
		unsigned int abn			: 1;	// bit 25		1: Abnormal case Found
		unsigned int dack			: 1;	// bit 24		1: Duplicated ACK
		unsigned int reserved		: 7;	// bit 23:17
		unsigned int TotalPktSize	: 17;	// bit 16: 0	Total packet size 
#else
		unsigned int TotalPktSize	: 17;	// bit 16: 0	Total packet size 
		unsigned int reserved		: 7;	// bit 23:17
		unsigned int dack			: 1;	// bit 24		1: Duplicated ACK
		unsigned int abn			: 1;	// bit 25		1: Abnormal case Found
		unsigned int tcp_opt		: 1;	// bit 26		1: Have TCP option
		unsigned int ip_opt			: 1;	// bit 27		1: have IPV4 option or IPV6 Extension header
		unsigned int sat			: 1;	// bit 28		1: SeqCnt > SeqThreshold, or AckCnt > AckThreshold
		unsigned int osq			: 1;	// bit 29		1: out of sequence
		unsigned int ctl			: 1;	// bit 30		1: have control flag bits (except ack)
		unsigned int usd			: 1;	// bit 31		0: if no data assembled yet
#endif
	} bits;
} TOE_QHDR2_T;

/**********************************************************************
 * TOE Queue Header Word 3
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int seq_num;
} TOE_QHDR3_T;

/**********************************************************************
 * TOE Queue Header Word 4
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int ack_num;
} TOE_QHDR4_T;

/**********************************************************************
 * TOE Queue Header Word 5
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_qhdr5
	{
#if (BIG_ENDIAN==1)
		
		unsigned int SeqCnt		: 16;	// bit 31:16
		unsigned int AckCnt		: 16;	// bit 15:0
#else
		unsigned int AckCnt		: 16;	// bit 15:0
		unsigned int SeqCnt		: 16;	// bit 31:16
#endif
	} bits;
} TOE_QHDR5_T;

/**********************************************************************
 * TOE Queue Header Word 6
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_qhdr6
	{
#if (BIG_ENDIAN==1)
		
		unsigned int MaxPktSize	: 14;	// bit 31:18
		unsigned int iq_num		: 2;	// bit 17:16
		unsigned int WinSize	: 16;	// bit 15:0
#else
		unsigned int WinSize	: 16;	// bit 15:0
		unsigned int iq_num		: 2;	// bit 17:16
		unsigned int MaxPktSize	: 14;	// bit 31:18
#endif
	} bits;
} TOE_QHDR6_T;

/**********************************************************************
 * TOE Queue Header Word 7
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_qhdr7
	{
#if (BIG_ENDIAN==1)
		
		unsigned int SeqThreshold	: 16;	// bit 31:16
		unsigned int AckThreshold	: 16;	// bit 15:0
#else
		unsigned int AckThreshold	: 16;	// bit 15:0
		unsigned int SeqThreshold	: 16;	// bit 31:16
#endif
	} bits;
} TOE_QHDR7_T;

/**********************************************************************
 * TOE Queue Header
 **********************************************************************/
typedef struct
{
	TOE_QHDR0_T		word0;
	TOE_QHDR1_T		word1;
	TOE_QHDR2_T		word2;
	TOE_QHDR3_T		word3;
	TOE_QHDR4_T		word4;
	TOE_QHDR5_T		word5;
	TOE_QHDR6_T		word6;
	TOE_QHDR7_T		word7;
} TOE_QHDR_T;
 
/**********************************************************************
 * NONTOE Queue Header Word 0
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int base_size;
} NONTOE_QHDR0_T;

#define NONTOE_QHDR0_BASE_MASK 	(~0x0f)

/**********************************************************************
 * NONTOE Queue Header Word 1
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_nonqhdr1
	{
#if (BIG_ENDIAN==1)
		
		unsigned int wptr			: 16;	// bit 31:16
		unsigned int rptr			: 16;	// bit 15:0
#else
		unsigned int rptr			: 16;	// bit 15:0	
		unsigned int wptr			: 16;	// bit 31:16
#endif
	} bits;
} NONTOE_QHDR1_T;

/**********************************************************************
 * Non-TOE Queue Header
 **********************************************************************/
typedef struct
{
	NONTOE_QHDR0_T		word0;
	NONTOE_QHDR1_T		word1;
} NONTOE_QHDR_T;
 
/**********************************************************************
 * Interrupt Queue Header Word 0
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_intrqhdr0
	{
#if (BIG_ENDIAN==1)
		
		unsigned int wptr		: 16;	// bit 31:16	Write Pointer where hw stopped
		unsigned int win_size	: 16;	// bit 15:0 	Descriptor Ring Size
#else
		unsigned int win_size	: 16;	// bit 15:0 	Descriptor Ring Size
		unsigned int wptr		: 16;	// bit 31:16	Write Pointer where hw stopped
#endif
	} bits;
} INTR_QHDR0_T;

/**********************************************************************
 * Interrupt Queue Header Word 1
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_intrqhdr1
	{
#if (BIG_ENDIAN==1)
		
		unsigned int ctl			: 1;	// bit 31		1: have control flag bits (except ack)
		unsigned int osq			: 1;	// bit 30		1: out of sequence
		unsigned int sat			: 1;	// bit 29		1: SeqCnt > SeqThreshold, or AckCnt > AckThreshold
		unsigned int ip_opt			: 1;	// bit 28		1: have IPV4 option or IPV6 Extension header
		unsigned int tcp_opt		: 1;	// bit 27		1: Have TCP option
		unsigned int abn			: 1;	// bit 26		1: Abnormal case Found
		unsigned int dack			: 1;	// bit 25		1: Duplicated ACK
		unsigned int tcp_qid		: 8;	// bit 24:17	TCP Queue ID
		unsigned int TotalPktSize	: 17;	// bit 16: 0	Total packet size 
#else
		unsigned int TotalPktSize	: 17;	// bit 16: 0	Total packet size 
		unsigned int tcp_qid		: 8;	// bit 24:17	TCP Queue ID
		unsigned int dack			: 1;	// bit 25		1: Duplicated ACK
		unsigned int abn			: 1;	// bit 26		1: Abnormal case Found
		unsigned int tcp_opt		: 1;	// bit 27		1: Have TCP option
		unsigned int ip_opt			: 1;	// bit 28		1: have IPV4 option or IPV6 Extension header
		unsigned int sat			: 1;	// bit 29		1: SeqCnt > SeqThreshold, or AckCnt > AckThreshold
		unsigned int osq			: 1;	// bit 30		1: out of sequence
		unsigned int ctl			: 1;	// bit 31		1: have control flag bits (except ack)
#endif
	} bits;
} INTR_QHDR1_T;

/**********************************************************************
 * Interrupt Queue Header Word 2
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int seq_num;
} INTR_QHDR2_T;

/**********************************************************************
 * Interrupt Queue Header Word 3
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	unsigned int ack_num;
} INTR_QHDR3_T;

/**********************************************************************
 * Interrupt Queue Header Word 4
 **********************************************************************/
typedef union
{
	unsigned int bits32;
	struct bit_intrqhdr4
	{
#if (BIG_ENDIAN==1)
		
		unsigned int SeqCnt		: 16;	// bit 31:16	Seq# change since last seq# intr.
		unsigned int AckCnt		: 16;	// bit 15:0     Ack# change since last ack# intr.
#else
		unsigned int AckCnt		: 16;	// bit 15:0		Ack# change since last ack# intr.
		unsigned int SeqCnt		: 16;	// bit 31:16	Seq# change since last seq# intr.
#endif
	} bits;
} INTR_QHDR4_T;

/**********************************************************************
 * Interrupt Queue Header
 **********************************************************************/
typedef struct
{
	INTR_QHDR0_T		word0;
	INTR_QHDR1_T		word1;
	INTR_QHDR2_T		word2;
	INTR_QHDR3_T		word3;
	INTR_QHDR4_T		word4;
	unsigned int		word5;
	unsigned int		word6;
	unsigned int		word7;
} INTR_QHDR_T;
 
/**********************************************************************
 * GMAC Conf
 **********************************************************************/
typedef struct gmac_conf {
	struct net_device *dev;
	int portmap;
	int vid;
	int flag;     /* 1: active  0: non-active */
} sys_gmac_conf;

/**********************************************************************
 * GMAC private data
 **********************************************************************/
typedef struct {
	unsigned int		rwptr_reg;
	unsigned int		desc_base;
	unsigned int		total_desc_num;
	unsigned short		finished_idx;
	GMAC_TXDESC_T		*curr_tx_desc;
	GMAC_TXDESC_T		*curr_finished_desc;
	struct sk_buff		*tx_skb[TX_DESC_NUM];
	unsigned long		total_sent;
	unsigned long		total_finished;
	unsigned long		intr_cnt;
} GMAC_SWTXQ_T;	

typedef struct {
	unsigned int		desc_base;
	unsigned long 		eof_cnt;
} GMAC_HWTXQ_T;	
	
typedef struct gmac_private{
	struct net_device	*dev;
	unsigned int		existed;
	unsigned int		port_id;	// 0 or 1
	unsigned int		base_addr;
	unsigned int		dma_base_addr;
	unsigned char		*mac_addr1;
	unsigned char		*mac_addr2;
	unsigned int		swtxq_desc_base;
	unsigned int		hwtxq_desc_base;
	GMAC_SWTXQ_T		swtxq[TOE_SW_TXQ_NUM];
	GMAC_HWTXQ_T		hwtxq[TOE_HW_TXQ_NUM];
	NONTOE_QHDR_T		*default_qhdr;
	unsigned int		default_desc_base;
	unsigned int		default_desc_num;
	unsigned int		rx_curr_desc;
	DMA_RWPTR_T			rx_rwptr; 
	struct sk_buff		*curr_rx_skb;
	dma_addr_t			default_desc_base_dma;
	dma_addr_t			swtxq_desc_base_dma;
	dma_addr_t			hwtxq_desc_base_dma;
	unsigned int		irq;
	unsigned int		flow_control_enable	;
	unsigned int		pre_phy_status;
	unsigned int		full_duplex_cfg;
	unsigned int		speed_cfg;
	unsigned int		auto_nego_cfg;
	unsigned int		full_duplex_status;
	unsigned int		speed_status;
	unsigned int		phy_mode;	/* 0->MII 1->GMII 2->RGMII(10/100) 3->RGMII(1000) */
	unsigned int		phy_addr;
	unsigned int		intr0_enabled;	// 1: enabled
	unsigned int		intr1_enabled;	// 1: enabled
	unsigned int		intr2_enabled;	// 1: enabled
	unsigned int		intr3_enabled;	// 1: enabled
	unsigned int		intr4_enabled;	// 1: enabled
//	unsigned int		intr4_enabled_1;	// 1: enabled
	unsigned int		intr0_selected;	// 1: selected
	unsigned int		intr1_selected;	// 1: selected
	unsigned int		intr2_selected;	// 1: selected
	unsigned int		intr3_selected;	// 1: selected
	unsigned int		intr4_selected;	// 1: selected
	// void 				(*gmac_rcv_handler)(struct sk_buff *, int);
	struct net_device_stats ifStatics;
	unsigned long		txDerr_cnt[GMAC_NUM];
	unsigned long		txPerr_cnt[GMAC_NUM];
	unsigned long		RxDerr_cnt[GMAC_NUM];
	unsigned long		RxPerr_cnt[GMAC_NUM];
	unsigned int		isr_rx_cnt;
	unsigned int		isr_tx_cnt;
	unsigned long		rx_discard;
	unsigned long		rx_error;
	unsigned long		rx_mcast;
	unsigned long		rx_bcast;
	unsigned long		rx_status_cnt[8];
	unsigned long		rx_chksum_cnt[8];
	unsigned long		rx_sta1_ucast;	// for STA 1 MAC Address
	unsigned long		rx_sta2_ucast;	// for STA 2 MAC Address
	unsigned long		mib_full_cnt;
	unsigned long		rx_pause_on_cnt;
	unsigned long		tx_pause_on_cnt;
	unsigned long		rx_pause_off_cnt;
	unsigned long		tx_pause_off_cnt;
	unsigned long		rx_overrun_cnt;
	unsigned long		status_changed_cnt;
	unsigned long		default_q_cnt;
	unsigned long		hw_fq_empty_cnt;
	unsigned long		sw_fq_empty_cnt;
	unsigned long		default_q_intr_cnt;
	pid_t               thr_pid;
	wait_queue_head_t   thr_wait;
	struct completion   thr_exited;
    spinlock_t          lock;
    int                 time_to_die;
    int					operation;
    struct work_struct tx_timeout_task;
#ifdef SL351x_GMAC_WORKAROUND    
    unsigned long		short_frames_cnt;
#endif

	unsigned int		poll_bits;
	unsigned int		poll_defaultq;
	unsigned int		poll_classq;
}GMAC_INFO_T ;

typedef struct toe_private {
	unsigned int	swfq_desc_base;
	unsigned int	hwfq_desc_base;
	unsigned int	hwfq_buf_base;
//	unsigned int	toe_desc_base[TOE_TOE_QUEUE_NUM];
//	unsigned int	toe_desc_num;
	unsigned int	class_desc_base;	// for IPSEC-VPN
	unsigned int	class_desc_num;		// for IPSEC-VPN
//	unsigned int	intr_desc_base;
//	unsigned int	intr_desc_num;
//	unsigned int	intr_buf_base;
	DMA_RWPTR_T		fq_rx_rwptr;
	GMAC_INFO_T		gmac[GMAC_NUM];
	dma_addr_t		sw_freeq_desc_base_dma;
	dma_addr_t		hw_freeq_desc_base_dma;
	dma_addr_t		hwfq_buf_base_dma;
	dma_addr_t		hwfq_buf_end_dma;
//	dma_addr_t		toe_desc_base_dma[TOE_TOE_QUEUE_NUM];
	dma_addr_t		class_desc_base_dma;	// for IPSEC-VPN
//	dma_addr_t		intr_desc_base_dma;
//	dma_addr_t		intr_buf_base_dma;
//	unsigned long	toe_iq_intr_full_cnt[TOE_INTR_QUEUE_NUM];
//	unsigned long	toe_iq_intr_cnt[TOE_INTR_QUEUE_NUM];
//	unsigned long	toe_q_intr_full_cnt[TOE_TOE_QUEUE_NUM];
//	unsigned long	class_q_intr_full_cnt[TOE_CLASS_QUEUE_NUM];
//	unsigned long	class_q_intr_cnt[TOE_CLASS_QUEUE_NUM];
} TOE_INFO_T;

extern TOE_INFO_T toe_private_data;

#define GMAC_PORT0	0
#define GMAC_PORT1	1
/**********************************************************************
 * PHY Definition
 **********************************************************************/
#define HPHY_ADDR   			0x01
#define GPHY_ADDR   			0x02

enum phy_state
{
    LINK_DOWN   = 0,
    LINK_UP     = 1
};

#define	NETIF_F_HWNAT	0x80000000
/* transmit timeout value */

#endif //_GMAC_SL351x_H
