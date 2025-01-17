/*
 * Copyright (C) 2001-2004 by David Brownell
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/* this file is part of ehci-hcd.c */

/*-------------------------------------------------------------------------*/

/*
 * EHCI Root Hub ... the nonsharable stuff
 *
 * Registers don't need cpu_to_le32, that happens transparently
 */

/*-------------------------------------------------------------------------*/
#include <linux/usb_otg.h>
#ifdef	CONFIG_PM

static int ehci_bus_suspend_1 (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	int			port;

	if (time_before (jiffies, ehci->next_statechange))
		msleep(5);

	port = HCS_N_PORTS (ehci->hcs_params);
	spin_lock_irq (&ehci->lock);

	/* stop schedules, clean any completed work */
	if (HC_IS_RUNNING(hcd->state)) {
		ehci_quiesce (ehci);
		hcd->state = HC_STATE_QUIESCING;
	}
	ehci->command = readl (&ehci->regs->command);
	if (ehci->reclaim)
		ehci->reclaim_ready = 1;
	ehci_work(ehci, NULL);

	/* suspend any active/unsuspended ports, maybe allow wakeup */
	while (port--) {
		u32 __iomem	*reg = &ehci->regs->port_status [port];
		u32		t1 = readl (reg) & ~PORT_RWC_BITS;
		u32		t2 = t1;

		if ((t1 & PORT_PE) && !(t1 & PORT_OWNER))
			t2 |= PORT_SUSPEND;
		if (hcd->remote_wakeup)
			t2 |= PORT_WKOC_E|PORT_WKDISC_E|PORT_WKCONN_E;
		else
			t2 &= ~(PORT_WKOC_E|PORT_WKDISC_E|PORT_WKCONN_E);

		if (t1 != t2) {
			ehci_vdbg (ehci, "port %d, %08x -> %08x\n",
				port + 1, t1, t2);
			writel (t2, reg);
		}
	}

	/* turn off now-idle HC */
	del_timer_sync (&ehci->watchdog);
	ehci_halt (ehci);
	hcd->state = HC_STATE_SUSPENDED;

	ehci->next_statechange = jiffies + msecs_to_jiffies(10);
	spin_unlock_irq (&ehci->lock);
	return 0;
}


/* caller has locked the root hub, and should reset/reinit on error */
static int ehci_bus_resume_1 (struct usb_hcd *hcd)
{
	struct ehci_hcd		*ehci = hcd_to_ehci (hcd);
	u32			temp;
	int			i;
	int			intr_enable;

	if (time_before (jiffies, ehci->next_statechange))
		msleep(5);
	spin_lock_irq (&ehci->lock);

	/* Ideally and we've got a real resume here, and no port's power
	 * was lost.  (For PCI, that means Vaux was maintained.)  But we
	 * could instead be restoring a swsusp snapshot -- so that BIOS was
	 * the last user of the controller, not reset/pm hardware keeping
	 * state we gave to it.
	 */

	/* re-init operational registers in case we lost power */
	if (readl (&ehci->regs->intr_enable) == 0) {
 		/* at least some APM implementations will try to deliver
		 * IRQs right away, so delay them until we're ready.
 		 */
 		intr_enable = 1;
		writel (0, &ehci->regs->segment);
		writel (ehci->periodic_dma, &ehci->regs->frame_list);
		writel ((u32)ehci->async->qh_dma, &ehci->regs->async_next);
	} else
		intr_enable = 0;
	ehci_dbg(ehci, "resume root hub%s\n",
			intr_enable ? " after power loss" : "");

	/* restore CMD_RUN, framelist size, and irq threshold */
	writel (ehci->command, &ehci->regs->command);

	/* take ports out of suspend */
	i = HCS_N_PORTS (ehci->hcs_params);
	while (i--) {
		temp = readl (&ehci->regs->port_status [i]);
		temp &= ~(PORT_RWC_BITS
			| PORT_WKOC_E | PORT_WKDISC_E | PORT_WKCONN_E);
		if (temp & PORT_SUSPEND) {
			ehci->reset_done [i] = jiffies + msecs_to_jiffies (20);
			temp |= PORT_RESUME;
		}
		writel (temp, &ehci->regs->port_status [i]);
	}
	i = HCS_N_PORTS (ehci->hcs_params);
	mdelay (20);
	while (i--) {
		temp = readl (&ehci->regs->port_status [i]);
		if ((temp & PORT_SUSPEND) == 0)
			continue;
		temp &= ~(PORT_RWC_BITS | PORT_RESUME);
		writel (temp, &ehci->regs->port_status [i]);
		ehci_vdbg (ehci, "resumed port %d\n", i + 1);
	}
	(void) readl (&ehci->regs->command);

	/* maybe re-activate the schedule(s) */
	temp = 0;
	if (ehci->async->qh_next.qh)
		temp |= CMD_ASE;
	if (ehci->periodic_sched)
		temp |= CMD_PSE;
	if (temp) {
		ehci->command |= temp;
		writel (ehci->command, &ehci->regs->command);
	}

	ehci->next_statechange = jiffies + msecs_to_jiffies(5);
	hcd->state = HC_STATE_RUNNING;

	/* Now we can safely re-enable irqs */
	if (intr_enable)
		writel (INTR_MASK, &ehci->regs->intr_enable);

	spin_unlock_irq (&ehci->lock);
	return 0;
}

#else

#define ehci_bus_suspend_1	NULL
#define ehci_bus_resume_1		NULL

#endif	/* CONFIG_PM */

/*-------------------------------------------------------------------------*/

static int check_reset_complete (
	struct ehci_hcd	*ehci,
	int		index,
	int		port_status
) {
	if (!(port_status & PORT_CONNECT)) {
		ehci->reset_done [index] = 0;
		return port_status;
	}
//#ifndef CONFIG_USB_SL2312 //######################### For FEHCI
//#ifndef CONFIG_USB_SL2312_1 //######################### For FEHCI
//	/* if reset finished and it's still not enabled -- handoff */
//	if (!(port_status & PORT_PE)) {

//		/* with integrated TT, there's nobody to hand it to! */
//		if (ehci_is_TDI(ehci)) {
//			ehci_dbg (ehci,
//				"Failed to enable port %d on root hub TT\n",
//				index+1);
//			return port_status;
//		}

//		ehci_dbg (ehci, "port %d full speed --> companion\n",
//			index + 1);

		// what happens if HCS_N_CC(params) == 0 ?
//		port_status |= PORT_OWNER;
//		port_status &= ~PORT_RWC_BITS;
//		writel (port_status, &ehci->regs->port_status [index]);

//	} else
//		ehci_dbg (ehci, "port %d high speed\n", index + 1);
//#endif
//#endif
	return port_status;
}

/*-------------------------------------------------------------------------*/


/* build "status change" packet (one or two bytes) from HC registers */

static int
ehci_hub_status_data_1 (struct usb_hcd *hcd, char *buf)
{
	struct ehci_hcd	*ehci = hcd_to_ehci (hcd);
	u32		temp, status = 0;
	int		ports, i, retval = 1;
	unsigned long	flags;

	/* if !USB_SUSPEND, root hub timers won't get shut down ... */
	if (!HC_IS_RUNNING(hcd->state))
		return 0;

	/* init status to no-changes */
	buf [0] = 0;
	ports = HCS_N_PORTS (ehci->hcs_params);
	if (ports > 7) {
		buf [1] = 0;
		retval++;
	}
	
	/* no hub change reports (bit 0) for now (power, ...) */

	/* port N changes (bit N)? */
	spin_lock_irqsave (&ehci->lock, flags);
	for (i = 0; i < ports; i++) {
		temp = readl (&ehci->regs->port_status [i]);
//#ifndef CONFIG_USB_SL2312 //######################### For FEHCI
//#ifndef CONFIG_USB_SL2312_1 //######################### For FEHCI
//		if (temp & PORT_OWNER) {
//			/* don't report this in GetPortStatus */
//			if (temp & PORT_CSC) {
//				temp &= ~PORT_RWC_BITS;
//				temp |= PORT_CSC;
//				writel (temp, &ehci->regs->port_status [i]);
//			}
//			continue;
//		}
//#endif
//#endif
		if (!(temp & PORT_CONNECT))
			ehci->reset_done [i] = 0;
//<3>.To detect event of the connect-status-change  & port-enable-change			
//#ifdef CONFIG_USB_SL2312//John_Update
		if ((temp & (PORT_CSC | PORT_PEC )) != 0) {
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//                if ((temp & (PORT_CSC | PORT_PEC | PORT_OCC)) != 0) { //John Remove PORT_OCC
//#else
//                if ((temp & (PORT_CSC | PORT_PEC )) != 0) {  
//#endif
//#endif
//				// PORT_STAT_C_SUSPEND?
//				|| ((temp & PORT_RESUME) != 0
//					&& time_after (jiffies,
//						ehci->reset_done [i]))) {
			if (i < 7)
			    buf [0] |= 1 << (i + 1);
			else
			    buf [1] |= 1 << (i - 7);
			status = STS_PCD;
		}
	}
	/* FIXME autosuspend idle root hubs */
	spin_unlock_irqrestore (&ehci->lock, flags);
	return status ? retval : 0;
}

/*-------------------------------------------------------------------------*/
#ifdef USB_BUILD_MODULE
int restart_modules(void);
int restart_modules(void)
{
	int ret = 0;
	char *argv[] = {"/usr/sbin/restart_usb", "2",NULL };
	char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

	/* last parameter: 1 -> wait until execution has finished, 0 go ahead without waiting*/
	/* returns 0 if usermode process was started successfully, errorvalue otherwise*/
	/* no possiblity to get return value of usermode process*/
	ret = call_usermodehelper("/usr/sbin/restart_usb", argv, envp, 0);
	if (ret != 0)
		printk("error in call to usermodehelper: %i\n", ret);
	else
		printk("everything all right\n");
        return 0;
}
#endif
static void
ehci_hub_descriptor (
	struct ehci_hcd			*ehci,
	struct usb_hub_descriptor	*desc
) {
	int		ports = HCS_N_PORTS (ehci->hcs_params);
	u16		temp;

	desc->bDescriptorType = 0x29;
	desc->bPwrOn2PwrGood = 10;	/* ehci 1.0, 2.3.9 says 20ms max */
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = ports;
	temp = 1 + (ports / 8);
	desc->bDescLength = 7 + 2 * temp;

	/* two bitmaps:  ports removable, and usb 1.0 legacy PortPwrCtrlMask */
	memset (&desc->bitmap [0], 0, temp);
	memset (&desc->bitmap [temp], 0xff, temp);

	temp = 0x0008;			/* per-port overcurrent reporting */
	if (HCS_PPC (ehci->hcs_params))
		temp |= 0x0001;		/* per-port power control */
	else
		temp |= 0x0002;		/* no power switching */
#if 0
// re-enable when we support USB_PORT_FEAT_INDICATOR below.
	if (HCS_INDICATOR (ehci->hcs_params))
		temp |= 0x0080;		/* per-port indicators (LEDs) */
#endif
	desc->wHubCharacteristics = (__force __u16)cpu_to_le16 (temp);
}

/*-------------------------------------------------------------------------*/

#define	PORT_WAKE_BITS 	(PORT_WKOC_E|PORT_WKDISC_E|PORT_WKCONN_E)

static int ehci_hub_control_1 (
	struct usb_hcd	*hcd,
	u16		typeReq,
	u16		wValue,
	u16		wIndex,
	char		*buf1,
	u16		wLength
) {
	struct ehci_hcd	*ehci = hcd_to_ehci (hcd);
	int		ports = HCS_N_PORTS (ehci->hcs_params);
	u32		temp, status;
	unsigned long	flags;
	unsigned long timeout ;
	int		retval = 0;
	

//#ifdef CONFIG_USB_SL2312 //Bruce;;   
        int     val=0;     
//#else
//#ifdef CONFIG_USB_SL2312_1 //Bruce;;   
//        int     val=0;     
//#endif
//#endif

	/*
	 * FIXME:  support SetPortFeatures USB_PORT_FEAT_INDICATOR.
	 * HCS_INDICATOR may say we can change LEDs to off/amber/green.
	 * (track current state ourselves) ... blink for diagnostics,
	 * power, "this is the one", etc.  EHCI spec supports this.
	 */

	spin_lock_irqsave (&ehci->lock, flags);
	switch (typeReq) {
	case ClearHubFeature:
		switch (wValue) {
		case C_HUB_LOCAL_POWER:
		case C_HUB_OVER_CURRENT:
			/* no hub-wide feature/status flags */
			break;
		default:
			goto error;
		}
		break;
	case ClearPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		temp = readl (&ehci->regs->port_status [wIndex]);
//#ifndef CONFIG_USB_SL2312
//#ifndef CONFIG_USB_SL2312_1
//		if (temp & PORT_OWNER)
//			break;
//#endif
//#endif
		switch (wValue) {
		case USB_PORT_FEAT_ENABLE:
			writel (temp & ~PORT_PE,
				&ehci->regs->port_status [wIndex]);
			break;
		case USB_PORT_FEAT_C_ENABLE:
			writel((temp & ~PORT_RWC_BITS) | PORT_PEC,
				&ehci->regs->port_status [wIndex]);
			break;
		case USB_PORT_FEAT_SUSPEND:
			if (temp & PORT_RESET)
				goto error;
			if (temp & PORT_SUSPEND) {
				if ((temp & PORT_PE) == 0)
					goto error;
				/* resume signaling for 20 msec */
				temp &= ~(PORT_RWC_BITS | PORT_WAKE_BITS);
				writel (temp | PORT_RESUME,
					&ehci->regs->port_status [wIndex]);
				ehci->reset_done [wIndex] = jiffies
						+ msecs_to_jiffies (20);
			}
			break;
		case USB_PORT_FEAT_C_SUSPEND:
			/* we auto-clear this feature */
			break;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1
//		case USB_PORT_FEAT_POWER:
//			if (HCS_PPC (ehci->hcs_params))
//				writel (temp & ~(PORT_RWC_BITS | PORT_POWER),
//					&ehci->regs->port_status [wIndex]);
//			break;
//#endif
//#endif
		case USB_PORT_FEAT_C_CONNECTION:
			writel((temp & ~PORT_RWC_BITS) | PORT_CSC,
				&ehci->regs->port_status [wIndex]);
			break;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1
//		case USB_PORT_FEAT_C_OVER_CURRENT:
//			writel((temp & ~PORT_RWC_BITS) | PORT_OCC,
//				&ehci->regs->port_status [wIndex]);
//			break;
//#endif
//#endif
		case USB_PORT_FEAT_C_RESET:
			/* GetPortStatus clears reset */
			break;
		default:
			goto error;
		}
		readl (&ehci->regs->command);	/* unblock posted write */
		break;
	case GetHubDescriptor:
		ehci_hub_descriptor (ehci, (struct usb_hub_descriptor *)
			buf1);
		break;
	case GetHubStatus:
		/* no hub-wide feature/status flags */
		memset (buf1, 0, 4);
		//cpu_to_le32s ((u32 *) buf);
		break;
	case GetPortStatus:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		status = 0;
		
		temp = readl (&ehci->regs->port_status [wIndex]);
//		printk("ehci->regs %x\n",ehci->regs);
//                printk("GetPortStatus temp %x wIndex %x\n",temp,wIndex);
		// wPortChange bits
		if (temp & PORT_CSC)
			status |= 1 << USB_PORT_FEAT_C_CONNECTION;
		if (temp & PORT_PEC)
			status |= 1 << USB_PORT_FEAT_C_ENABLE;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//		if (temp & PORT_OCC)
//			status |= 1 << USB_PORT_FEAT_C_OVER_CURRENT;
//#endif
//#endif
		/* whoever resumes must GetPortStatus to complete it!! */
		if ((temp & PORT_RESUME)&& time_after (jiffies,ehci->reset_done [wIndex]))
		 {
//			printk("PORT_RESUME\n");
			status |= 1 << USB_PORT_FEAT_C_SUSPEND;
			ehci->reset_done [wIndex] = 0;

			/* stop resume signaling */
			temp = readl (&ehci->regs->port_status [wIndex]);
			writel (temp & ~ PORT_RESUME,
				&ehci->regs->port_status [wIndex]);
			retval = handshake (
					&ehci->regs->port_status [wIndex],
					PORT_RESUME, 0, 2000 /* 2msec */);
			if (retval != 0) {
				printk("port %d resume error %d\n",wIndex + 1, retval);
				goto error;
			}
			temp &= ~(PORT_SUSPEND|PORT_RESUME|(3<<10));
		 }

		/* whoever resets must GetPortStatus to complete it!! */
		if ((temp & PORT_RESET)&& time_after (jiffies,ehci->reset_done [wIndex]))
		 {
			status |= 1 << USB_PORT_FEAT_C_RESET;
			ehci->reset_done [wIndex] = 0;

			/* force reset to complete */
			writel (temp & ~PORT_RESET,&ehci->regs->port_status [wIndex]);
			/* REVISIT:  some hardware needs 550+ usec to clear
			 * this bit; seems too long to spin routinely...
			 */
			timeout = 0;
	                do { //stone 2.6.15
				temp = readl (&ehci->regs->port_status [wIndex]);
				udelay (10);
				timeout++;
			} while ((temp & PORT_RESET)&& timeout<10000); // 100 ms		 
			 
			retval = handshake (
					&ehci->regs->port_status [wIndex],
					PORT_RESET, 0, 750);
			if (retval != 0) {
				//ehci_err (ehci, "port %d reset error %d\n",
				//	wIndex + 1, retval);
				printk("ehci error!\n");
				goto error;
			}

			/* see what we found out */
			temp = check_reset_complete (ehci, wIndex,readl (&ehci->regs->port_status [wIndex]));
//#ifdef CONFIG_USB_SL2312
//                         //re-Start schedule
//                        writel(readl(&ehci->regs->command)|(0x1),&ehci->regs->command);
//                        ehci->hcd.state = HC_STATE_RUNNING;
     
//                        val=FEHCI_ReadSpeed();

//                        if (val == 2 )
//                         {
//                           printk("### >>> High Speed Device detect...\n");
//                          }
//                        else if (val== 1 )
//                         {
//                          printk("### >>> Low Speed Device detect...\n");
//                          }
//                        else if (val==0)
//                         {
//       	                   printk("### >>> Full Speed Device detect...\n");
//       	                  }
//       	                 else printk("### >>> Speed detect Error ...\n");      
//#else
#if defined  (CONFIG_USB_SL2312_1) || defined (CONFIG_USB_SL2312_1_MODULE)
                         //re-Start schedule
                        writel(readl(&ehci->regs->command)|(0x1),&ehci->regs->command);
                        ehci->hcd.state = HC_STATE_RUNNING;
     
                        val=FEHCI_ReadSpeed_1();

                        if (val == 2 )
                         {
                           printk("### >>> High Speed Device detect...\n");
                          }
                        else if (val== 1 )
                         {
                          printk("### >>> Low Speed Device detect...\n");
                          }
                        else if (val==0)
                         {
       	                   printk("### >>> Full Speed Device detect...\n");
       	                  }
       	                 else printk("### >>> Speed detect Error ...\n");   
#endif

//#endif	

		  }

		// don't show wPortStatus if it's owned by a companion hc
//#ifndef CONFIG_USB_SL2312
//#ifndef CONFIG_USB_SL2312_1
//		if (!(temp & PORT_OWNER)) {
//#else
                if(1)  {
//#endif
//#else
//                if(1)  {
//#endif
//#ifndef CONFIG_USB_SL2312
//#ifdef CONFIG_USB_SL2312_1
//                        DBG_HOST_TEMP("### ehci-hub.c-->ehci_hub_control_1()-->Detect the Speed ...\n");    
                      if (temp & PORT_CONNECT) 
                         {
                           status |= 1 << USB_PORT_FEAT_CONNECTION;
                            //Check spedd here via OTG controller

                           if (FEHCI_ReadSpeed_1() == 2 )
                           {
                            status |= 1 << USB_PORT_FEAT_HIGHSPEED;
                           }
                           else if (FEHCI_ReadSpeed_1() == 1 )
                             { 
                              status |= 1 << USB_PORT_FEAT_LOWSPEED;
                              }
                         }
//#else
//			if (temp & PORT_CONNECT) {
//				status |= 1 << USB_PORT_FEAT_CONNECTION;
				// status may be from integrated TT
//				status |= ehci_port_speed(ehci, temp);
//			}
//#endif
//#else
//     DBG_HOST_TEMP("### ehci-hub.c-->ehci_hub_control_1()-->Detect the Speed ...\n");    
//                     if (temp & PORT_CONNECT) 
//                         {
//                           status |= 1 << USB_PORT_FEAT_CONNECTION;
//                            //Check spedd here via OTG controller

//                           if (FEHCI_ReadSpeed() == 2 )
//                           {
//                            status |= 1 << USB_PORT_FEAT_HIGHSPEED;
//                           }
//                           else if (FEHCI_ReadSpeed() == 1 )
//                             { 
//                              status |= 1 << USB_PORT_FEAT_LOWSPEED;
//                              }
//                         }
    
//#endif 
		      if (temp & PORT_PE)
				status |= 1 << USB_PORT_FEAT_ENABLE;
		      if (temp & (PORT_SUSPEND|PORT_RESUME))
				status |= 1 << USB_PORT_FEAT_SUSPEND;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//		       if (temp & PORT_OC)
//				status |= 1 << USB_PORT_FEAT_OVER_CURRENT;
//#endif
//#endif
		        if (temp & PORT_RESET)
				status |= 1 << USB_PORT_FEAT_RESET;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//			if (temp & PORT_POWER)
//				status |= 1 << USB_PORT_FEAT_POWER;
//#endif
//#endif
		}//if(1) end

#ifndef	EHCI_VERBOSE_DEBUG
	if (status & ~0xffff)	/* only if wPortChange is interesting */
#endif
               
//		dbg_port (ehci, "GetStatus", wIndex + 1, temp);
		// we "know" this alignment is good, caller used kmalloc()...
		buf1[0]=0; //2.6.15 buf1 error bug
		buf1[1]=0;
		buf1[2]=0;
		buf1[3]=0;
                *((__le32 *) buf1) = cpu_to_le32 (status);
//		printk("temp %x status %x buf %x\n",temp,status,buf1);
		break;
	case SetHubFeature:
		switch (wValue) {
		case C_HUB_LOCAL_POWER:
		case C_HUB_OVER_CURRENT:
			/* no hub-wide feature/status flags */
			break;
		default:
			goto error;
		}
		break;
	case SetPortFeature:
		if (!wIndex || wIndex > ports)
			goto error;
		wIndex--;
		temp = readl (&ehci->regs->port_status [wIndex]);
//#ifndef CONFIG_USB_SL2312
//#ifndef CONFIG_USB_SL2312_1
//		if (temp & PORT_OWNER)
//			break;
//#endif
//#endif
		temp &= ~PORT_RWC_BITS;
		switch (wValue) {
		case USB_PORT_FEAT_SUSPEND:
			if ((temp & PORT_PE) == 0
					|| (temp & PORT_RESET) != 0)
				goto error;
			if (hcd->remote_wakeup)
				temp |= PORT_WAKE_BITS;
			writel (temp | PORT_SUSPEND,
				&ehci->regs->port_status [wIndex]);
			break;
		case USB_PORT_FEAT_POWER:
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//			if (HCS_PPC (ehci->hcs_params))
//				writel (temp | PORT_POWER,
//					&ehci->regs->port_status [wIndex]);
//#endif
//#endif

			break;
		case USB_PORT_FEAT_RESET:
//#ifndef CONFIG_USB_SL2312
//#ifndef CONFIG_USB_SL2312_1
//			if (temp & PORT_RESUME)
//				goto error;
			/* line status bits may report this as low speed,
			 * which can be fine if this root hub has a
			 * transaction translator built in.
			 */
//			if ((temp & (PORT_PE|PORT_CONNECT)) == PORT_CONNECT
//					&& !ehci_is_TDI(ehci)
//					&& PORT_USB11 (temp)) {
//				ehci_dbg (ehci,
//					"port %d low speed --> companion\n",
//					wIndex + 1);
//				temp |= PORT_OWNER;
//			} else 
//#endif
//#endif
			{
				ehci_vdbg (ehci, "port %d reset\n", wIndex + 1);
				temp |= PORT_RESET;
//#ifdef CONFIG_USB_SL2312//John_Update
//#else
//#ifndef CONFIG_USB_SL2312_1//John_Update
//				temp &= ~PORT_PE;
//#endif
//#endif

				/*
				 * caller must wait, then call GetPortStatus
				 * usb 2.0 spec says 50 ms resets on root
				 */
				ehci->reset_done [wIndex] = jiffies
						+ msecs_to_jiffies (50);
			}
//#ifdef CONFIG_USB_SL2312


       //Force Speed 
       //FOTG2XX will skip the speed parameter, control by the linux-ap
       //FUSB2XX will use these parameter
//        FEHCI_ForceSpeed(2);//Force High speed
//        FEHCI_ForceSpeed(0);//Force High speed for new PHY test auto detect speed!!
//        FEHCI_ForceSpeed(1);//Force Full/Low Speed
       


      //Stop schedule
//      writel(readl(&ehci->regs->command)&(~(CMD_RUN|CMD_ASE|CMD_PSE)),&ehci->regs->command);
//      while((readl(&ehci->regs->status)&STS_HALT) == 0);
//#else
//#ifdef CONFIG_USB_SL2312_1

        FEHCI_ForceSpeed_1(0);//Force High speed for new PHY test auto detect speed!!

      writel(readl(&ehci->regs->command)&(~(CMD_RUN|CMD_ASE|CMD_PSE)),&ehci->regs->command);
#ifdef USB_BUILD_MODULE
      //Stone try to reset host control  
              int i=0,temp1;
               do
                 {
                     i++;
                    	if (i>10)
                   	{
//                          temp1 = ehci_reset(ehci);
//                          temp1 = ehci_halt(ehci); 
                          USB_driver_Flag_1=1;
                          mdelay(100);
                          restart_modules();
         		              break;
                          }
                    mdelay(10);
                  }while((readl(&ehci->regs->status)&STS_HALT) == 0);
#else
      while((readl(&ehci->regs->status)&STS_HALT) == 0);
#endif
//#endif
//#endif
			writel (temp, &ehci->regs->port_status [wIndex]);
			break;
		default:
			goto error;
		}
		readl (&ehci->regs->command);	/* unblock posted writes */
		break;

	default:
error:
		/* "stall" on error */
		retval = -EPIPE;
	}
	spin_unlock_irqrestore (&ehci->lock, flags);
	return retval;
}
