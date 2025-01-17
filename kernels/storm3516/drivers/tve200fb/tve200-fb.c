#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/syscalls.h>
#include <linux/delay.h>
#include <linux/mm.h>

#include <asm/io.h>
#include <asm/uaccess.h>

//debug_Aaron
//#include <asm/arch/spec.h>
//#include <asm/arch/intc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sl2312.h>
#include <asm/sizes.h>

#define TVE_BASE_ADDR       (IO_ADDRESS(SL2312_TVE_BASE))
#define TVE_END_ADDR       (IO_ADDRESS(SL2312_TVE_BASE+SZ_512K))


#include <asm/irq.h>
#include <asm/fiq.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>

#include "ffb.h"
#include "tve200-fb.h"
#include "pic.h"//marco
#include "status_bar.h"
//#include "log_raw.h"//marco
//debug_Aaron
//#define CS4954	1

//#define FFBDBG_LV   3
#include "debug.h"

#include "tve-info.c"

//debug_Aaron, those function can not be found 
int soft_cursor(struct fb_info *info, struct fb_cursor *cursor);
void cs4954_init(void);

int g_screen_size=640*480*3;//marco
int MAX_X = 720;           
int MAX_Y = 576;           
int FB_BPP = 24;
char *FB_MODE = "";
module_param (MAX_X, int, 0);
module_param (MAX_Y, int, 0);
module_param (FB_BPP, int, 0);
module_param (FB_MODE, charp, 0);
MODULE_PARM_DESC (MAX_X, "tve200fb: Maximum X pixels");
MODULE_PARM_DESC (MAX_Y, "tve200fb: Maximum Y pixels");
MODULE_PARM_DESC (FB_BPP, "tve200fb: Bit per Pixel");
MODULE_PARM_DESC (FB_MODE, "tve200fb: Color mode");

static u32 ft_get_version(void)
{
	return 0;
}

#if 0
char *fmem_alloc(int size, u32 *dma_handle, u32 offset)
{
	unsigned char *buf_ptr;

	buf_ptr = dma_alloc_coherent(NULL, size, dma_handle, GFP_ATOMIC);
	printk("%s: size=%d, buf_ptr=0x%x, dma_handle=0x%x\r\n", __func__, size, buf_ptr, dma_handle);
	return buf_ptr;
}

void *fmem_free(size_t size, void *cpu_addr, dma_addr_t handle)
{
	//debug_Aaron
	DBGPRINT(3, "%s: size=%d, cpu_addr=0x%x, handle=0x%x\r\n", __func__, size, cpu_addr, handle);	
	dma_free_coherent(NULL, size, cpu_addr, handle);
}
#endif


unsigned int IP_va_base[1] = {TVE_BASE_ADDR};

#if defined(CONFIG_MODE_NTSC)
#define DEFAULT_MODE  FFB_MODE_NTSC
#elif defined(CONFIG_MODE_PAL)
#define DEFAULT_MODE  FFB_MODE_PAL
#else
#error "Please defined default mode in tve-config.h"
#endif

#if defined(CONFIG_COLOR_RGB)
#define DEFAULT_COLOR  FFB_MODE_RGB
#elif defined(CONFIG_COLOR_YUV422)
#define DEFAULT_COLOR  FFB_MODE_YUV422
#elif defined(CONFIG_COLOR_YUV420)
#define DEFAULT_COLOR  FFB_MODE_YUV420
#else
#error "Please defined default color in tve-config.h"
#endif

//debug_Aaron
//#define FFB_DEFAULT_MODE  (DEFAULT_MODE | DEFAULT_COLOR)
unsigned int FFB_DEFAULT_MODE = (DEFAULT_MODE | DEFAULT_COLOR);


#if (CONFIG_MAX_FRAME_NO < CONFIG_PP_FRAME_NO)
#error "CONFIG_PP_FRAME_NO is large than CONFIG_MAX_FRAME_NO"
#endif

const unsigned int config_fb_number = CONFIG_PP_FRAME_NO;

#define TVE_INT_GEN 0x40

#if defined(CONFIG_USE_FIQ)

#include "tve200-fiq.h"

static u32 *fiq_data = 0;
#endif

static inline void TVE200BaseSet(volatile TVE200_Register * ptve,
				 u32 ybase, u32 ubase, u32 vbase)
{
	// UPBase address.
//printk("Set at %d\n",jiffies);
	ptve->Ybase = (u32) ybase;
	ptve->Ubase = (u32) ubase;
	ptve->Vbase = (u32) vbase;

	//debug_Aaron
	DBGPRINT(3, "%s: ybase=0x%x, ubase=0x%x, vbase=0x%x\r\n", __func__, ybase, ubase, vbase);
}

struct frame_ctrl_info {
	int do_updating;
	int old_fbnum;
	int miss_num;
};

static struct frame_ctrl_info *g_fci = 0;

#if defined(CONFIG_FB_SIGNAL)
extern pid_t mpid;
#endif

#ifdef CONFIG_CT656_VBI
struct vbi_ctrl_info {
	struct ct656_vbi_ins_info act_vbi_info[CONFIG_PP_FRAME_NO];
	struct ct656_vbi_ins_info vbi_info;
	unsigned int vbi_line;	//NTSC 0-22 // 0-31
};

static struct vbi_ctrl_info *g_vci = 0;

#endif

#ifndef CONFIG_USE_FIQ
irqreturn_t ffb_handle_irq_VerE(int irq, void *dev_id, struct pt_regs *regs)
{
	struct ffb_info *fbi = (struct ffb_info *) dev_id;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	unsigned int status = ptve->INTSts;

	if (likely(status & 0x40)) {
		if (fbi->ppfb_num == 0) {
			g_fci->miss_num = 0;
			g_fci->do_updating++;
			g_fci->do_updating &= 0x01;
			goto end;
		}

		if (g_fci->do_updating) {
			g_fci->do_updating = 0;

			if (fbi->ppfb_num == g_fci->old_fbnum) {
				++(g_fci->miss_num);
				DBGPRINT(2,
					 "FTVE miss 0x%x frames with %d!\n",
					 (int) g_fci->miss_num,
					 (int) fbi->ppfb_num);
#if defined(CONFIG_FB_SIGNAL)
				if (mpid)
				{
printk("        kill at %d\n",jiffies);
					sys_kill(mpid, SIGUSR1);
				}
#endif
				goto end;
			}

#ifdef CONFIG_CT656_VBI
			ct656_put_vbi(g_vci->vbi_line,
				      &g_vci->act_vbi_info[fbi->ppfb_num].field[0]);
#endif

			g_fci->old_fbnum = fbi->ppfb_num;
			TVE200BaseSet(ptve,
				      fbi->ppfb_dma[fbi->ppfb_num] ,
				      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->u_offset,
				      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->v_offset);
#if defined(FB_SIGNAL)
			if (mpid)
			{
printk("        kill at %d\n",jiffies);
				sys_kill(mpid, SIGUSR1);
			}
#endif
		} else {
			g_fci->do_updating = 1;
#ifdef CONFIG_CT656_VBI
			ct656_put_vbi(g_vci->vbi_line + 1,
				      &g_vci->act_vbi_info[fbi->ppfb_num].field[1]);
#endif
		}
	}

      end:
	// clear interrupt status
	ptve->INTClr = status;
	return IRQ_HANDLED;
}

irqreturn_t ffb_handle_irq(int irq, void *dev_id, struct pt_regs *regs)
{
	struct ffb_info *fbi = (struct ffb_info *) dev_id;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	unsigned int status = ptve->INTSts;
	unsigned int tmp;
printk("<R>");
	if (likely(status & 0x40)) {
		tmp = ptve->Ctrl1;

		if (!(tmp & 0x30)) { // Vertical Sync.
			tmp |= 0x20;
			ptve->Ctrl1 = tmp;
			if ((mach_data->ctrl1 & 0x30) != 0)
				goto end;
		} else {  //Active Image
			tmp &= (~0x30);
			ptve->Ctrl1 = tmp;
			if ((mach_data->ctrl1 & 0x30) == 0)
				goto end;
		}

		if (fbi->ppfb_num == 0) {
			g_fci->miss_num = 0;
			g_fci->do_updating++;
			g_fci->do_updating &= 0x01;
			goto end;
		}

		if (g_fci->do_updating) {
			g_fci->do_updating = 0;

			if (fbi->ppfb_num == g_fci->old_fbnum) {
				++(g_fci->miss_num);
				DBGPRINT(2,
					 "FTVE miss 0x%x frames with %d!\n",
					 (int) g_fci->miss_num,
					 (int) fbi->ppfb_num);
#if defined(CONFIG_FB_SIGNAL)
				if (mpid)
				{
printk("        kill at %d\n",jiffies);
					sys_kill(mpid, SIGUR1);
				}
#endif
				goto end;
			}

#ifdef CONFIG_CT656_VBI
			ct656_put_vbi(g_vci->vbi_line,
				      &g_vci->act_vbi_info[fbi->ppfb_num].field[0]);
#endif

			g_fci->old_fbnum = fbi->ppfb_num;
			TVE200BaseSet(ptve,
				      fbi->ppfb_dma[fbi->ppfb_num] ,
				      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->u_offset,
				      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->v_offset);
#if defined(FB_SIGNAL)
			if (mpid)
			{
printk("        kill at %d\n",jiffies);
				sys_kill(mpid, SIGUSR1);
			}
#endif
		} else {
			g_fci->do_updating = 1;
#ifdef CONFIG_CT656_VBI
			ct656_put_vbi(g_vci->vbi_line + 1,
				      &g_vci->act_vbi_info[fbi->ppfb_num].field[1]);
#endif
		}
	}

      end:
	// clear interrupt status
	ptve->INTClr = status;
	return IRQ_HANDLED;
}
#endif

static int tve_activate_var(struct fb_var_screeninfo *var,
			    struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	int ret = 0;


	DBGENTER(1);

	//debug_Aaron
	DBGPRINT(1, "%s: fbi->io_base=0x%x\r\n", __func__, fbi->io_base);

	DBGPRINT(2, "var: xres=%d hslen=%d lm=%d rm=%d\n",
		 var->xres, var->hsync_len,
		 var->left_margin, var->right_margin);
	DBGPRINT(2, "var: yres=%d vslen=%d um=%d bm=%d\n",
		 var->yres, var->vsync_len,
		 var->upper_margin, var->lower_margin);

	if ((ptve->Ctrl1 != mach_data->ctrl1)
	    || (ptve->Ctrl3 != mach_data->ctrl3))
		ret = 1;

	DBGLEAVE(1);
	return ret;
}

static inline void tve200_enable_controller(struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;

	DBGPRINT(2, "Enabling TVE controller\n");
	ptve->Ctrl1 = mach_data->ctrl1 & ~0x01;
	ptve->Ctrl3 = mach_data->ctrl3;

	TVE200BaseSet(ptve,
		      fbi->screen_dma,
		      fbi->screen_dma + mach_data->u_offset,
		      fbi->screen_dma + mach_data->v_offset);

	ptve->Ctrl1 |= 0x0001;

	DBGPRINT(2, "Ctrl1   = %#08x\n", ptve->Ctrl1);
	DBGPRINT(2, "Ctrl3   = %#08x\n", ptve->Ctrl3);
}

static inline void tve200_disable_controller(struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	DECLARE_WAITQUEUE(wait, current);

	DBGPRINT(2, "Disabling LCD controller\n");

	set_current_state(TASK_UNINTERRUPTIBLE);
	add_wait_queue(&fbi->ctrlr_wait, &wait);

	mach_data->ctrl1 &= ~0x0001;
	ptve->Ctrl1 = mach_data->ctrl1;

	schedule_timeout(1 * HZ / 1000);//marco ,orginal is 20 Hz
	remove_wait_queue(&fbi->ctrlr_wait, &wait);
}

static inline void tve200_enable_int(struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;

	ptve->INTClr = 0xff;
	ptve->INTEn = mach_data->int_mask;
}

static inline void tve200_disable_int(struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;

	ptve->INTEn = 0;
}

static inline void tve200_controller_reset(struct ffb_info *fbi)
{
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	unsigned int timeout = 30000;
	unsigned int retry = 5;

	DBGPRINT(3, "Reset Controller:");
	ptve->Ctrl4 = 0x01;
	while (retry--) {
		int i = 0;
		while ((ptve->Ctrl4 & 0x01) && --timeout) {
			udelay(1);
			DBGPRINT(3, "...");
		};
		if (timeout)
			break;
		else
			err("Time Out!(%d)", i);
		i++;
	}

	g_fci->do_updating = 0;
	DBGPRINT(3, "OK!\n");
}

void tve_controller_enable(struct ffb_info *fbi)
{
//debug_Aaron no PMU support
	//tve_platform_pmu_on();

	tve200_controller_reset(fbi);

//debug_Aaron, no gm8180 support
	//tve_platform_setup_i2c(fbi->smode);

	tve200_enable_controller(fbi);
	tve200_enable_int(fbi);
}

void tve_controller_disable(struct ffb_info *fbi)
{
	tve200_disable_int(fbi);
	tve200_disable_controller(fbi);
//debug_Aaron no PMU support
	//tve_platform_pmu_off();
}

static int tve_get_mach_param(struct tve_mach_info *inf,
			      tve_mach_timer_t * timer, u_long smode)
{
	ffb_hw_param_t *tparam;
	ffb_hw_res_t *tres;

	if (!(inf->support_modes & smode & 0xff00)) {
		err("No support mode");
		return -1;
	}

	if (!(inf->support_modes & smode & 0x00ff)) {
		err("No support color space");
		return -1;
	}

	tparam = ffb_get_hw_param(inf->ctrl1, inf->num_ctrl1, smode);
	if (!tparam) {
		err("No ctrl1");
		return -1;
	}
	timer->ctrl1 = tparam->value;

	tparam = ffb_get_hw_param(inf->ctrl3, inf->num_ctrl3, smode);
	if (!tparam) {
		err("No ctrl3");
		return -1;
	}
	timer->ctrl3 = tparam->value;

	tres = ffb_get_hw_res(inf->res, inf->num_res, smode);
	if (!tres) {
		err("No res");
		return -1;
	}
	timer->xres = tres->xres;
	timer->yres = tres->yres;
	return 0;
}

static int tve_set_var(struct ffb_info *fbi, u_long type)
{
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	tve_mach_timer_t timer;
	u_long t_smode = 0;

	if (!type) {
		t_smode = fbi->smode;
	} else {
		t_smode = type;
	}

	if (tve_get_mach_param(&pannel_info, &timer, t_smode)) {
		err("Error mode value = %#lx\n", t_smode);
		return -1;
	}

	fbi->fb.var.xres = timer.xres;
	fbi->fb.var.xres_virtual = timer.xres;
	fbi->fb.var.yres = timer.yres;
	fbi->fb.var.yres_virtual = timer.yres;

	mach_data->ctrl1 = timer.ctrl1;
	mach_data->ctrl3 = timer.ctrl3;

	mach_data->ctrl1 &= ~0x0e00;
	switch (t_smode & 0xff00) {
	case FFB_MODE_NTSC:
		//debug_Aaron
		//mach_data->ctrl1 |= 0xa00;
		mach_data->ctrl1 |= 0x600;
		break;
	case FFB_MODE_PAL:
		mach_data->ctrl1 |= 0x800;
		break;
	}

	mach_data->ctrl1 &= ~0x800080c2;

//debug_Aaron
//printk("%s: t_smode=0x%x\r\n", __func__, t_smode);
	switch (t_smode & 0xff) {
	case FFB_MODE_YUV420:
		mach_data->ctrl1 |= 0x80000080;
		//mach_data->u_offset =
		//	 ((fbi->fb.var.xres * fbi->fb.var.yres +
                  //    0xffff) & 0xffff0000);
		mach_data->u_offset = fbi->fb.var.xres * fbi->fb.var.yres;
		mach_data->v_offset = (mach_data->u_offset * 5 / 4);
//debug_Aaron
//printk("%s: mach_data->u_offset=%d, mach_data->v_offset=%d\r\n", __func__, mach_data->u_offset, mach_data->v_offset);
		break;
	case FFB_MODE_YUV422:
		mach_data->ctrl1 |= 0x000000c0;
		mach_data->u_offset = 0;
		mach_data->v_offset = 0;
		break;
	case FFB_MODE_RGB:
		//mach_data->ctrl1 |= 0x00008040;
		mach_data->ctrl1 |= 0x00008080;  /* default RGB 888 */

		//debug_Aaron
		DBGPRINT(3, "%s: mach_data->ctrl1=0x%x\r\n", __func__, mach_data->ctrl1);

		mach_data->u_offset = 0;
		mach_data->v_offset = 0;
		break;
	default:
		err("Color Mode error");
		return -1;
	}
#ifdef CONFIG_CT656_VBI
	mach_data->int_mask = TVE_INT_GEN;
#else
	mach_data->int_mask = 0;
#endif

	//debug_Aaron on 05/14/2008, set input resolution according to MAX_X 
        mach_data->ctrl1 &= ~0xC00;
	if (MAX_X > 352 && MAX_X < 720)
                mach_data->ctrl1 |= (0x01 << 10);  /* VGA */
	else if (MAX_X >= 720)
                mach_data->ctrl1 |= (0x02 << 10);   /* D1 */
//	mach_data->ctrl1 |= 0x2;//marco BGR
	mach_data->ctrl1 &= ~0x8000;//marco BE 8000
	
//debug_Aaron
 DBGPRINT(3, "%s: mach_data->ctrl1 = 0x%x\r\n", __func__, mach_data->ctrl1);

	return 0;
}

static int tve_set_input_resolution(struct ffb_info *fbi,
				    unsigned int type)
{
	int ret = 0;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	mach_data->ctrl1 &= ~0xC00;

	//debug_Aaron
	DBGPRINT(3, "%s: type = %d\r\n", __func__, type);
	
	switch (type) {
	case TVE_INPUT_RES_CIF:
		break;
	case TVE_INPUT_RES_VGA:
		mach_data->ctrl1 |= (0x01 << 10);
		break;
	case TVE_INPUT_RES_D1:
	default:
		mach_data->ctrl1 |= (0x02 << 10);
		break;
	}

	tve_controller_disable(fbi);
	tve_controller_enable(fbi);
	return ret;
}

//debug_Aaron support RGB ordering
static int tve_set_RGB_order(struct ffb_info *fbi,
                                    unsigned int order)
{
        int ret = 0;
        struct tve_mach_data *mach_data =
            ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;

        mach_data->ctrl1 &= ~0xC2;

	//debug_Aaron
	DBGPRINT(3, "%s: order = %d\r\n", __func__, order);
        switch (order) 
	{
       		 case TVE_RGB_ORDER_555:
			//fbi->max_bpp = 16;
                	break;
       		 case TVE_RGB_ORDER_565:
			//fbi->max_bpp = 16;
                	mach_data->ctrl1 |= 0x40;
                	break;
       		 case TVE_RGB_ORDER_888:
        	 default:
			//fbi->max_bpp = 24;
                	mach_data->ctrl1 |= 0x80;
                	break;
        }
        tve_controller_disable(fbi);
        tve_controller_enable(fbi);
        return ret;
}

static int tve_set_buf_format(struct ffb_info *fbi, unsigned int type)
{
	int ret = 0;
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;
	struct tve_mach_data *mach_data =
		((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;

	mach_data->ctrl1 &= ~0x100;
	if(type == FFB_BUFFMT_INTERLACE) {
		mach_data->ctrl1 |= 0x100;
	}
	tve_controller_disable(fbi);
	tve_controller_enable(fbi);
	return ret;
}

static int tve_get_buf_format(struct ffb_info *fbi, unsigned int *type)
{
	int ret = 0;
	struct tve_mach_data *mach_data =
		((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	
	if(mach_data->ctrl1 & 0x100)
		*type = FFB_BUFFMT_INTERLACE;
	else
		*type = FFB_BUFFMT_NONINTERLACE;

	return ret;
}

static int tve_ioctl(struct inode *inode, struct file *file,
		     unsigned int cmd, unsigned long arg,
		     struct fb_info *info)
{
	struct ffb_info *fbi = (struct ffb_info *) info;
	struct tve_mach_data *mach_data =
	    ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;
	int ret = 0;
	volatile TVE200_Register *ptve = (TVE200_Register *) fbi->io_base;

	DBGENTER(1);

//printk("tve_ioctl at %d cmd=0x%x FLCD_SET_FB_NUM=0x%x FFB_SET_FB_NUM=0x%x\n",jiffies,cmd,FLCD_SET_FB_NUM,FFB_SET_FB_NUM);
	switch (cmd) {
	case FLCD_SET_FB_NUM:	//0,1.. n
	case FFB_SET_FB_NUM:
		{
			unsigned int fbnum;
			int b_en_irq = 0;
			DBGPRINT(3, "FFB_SET_FB_NUM:\n");
			if (copy_from_user
			    (&fbnum, (unsigned int *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
			if (fbnum >= config_fb_number) {
				ret = -EFAULT;
				break;
			}
			if (fbi->ppfb_num == 0 && fbnum != 0)
				b_en_irq = 1;

#ifdef CONFIG_CT656_VBI
			memcpy(&g_vci->act_vbi_info[fbnum], &g_vci->vbi_info, sizeof(struct ct656_vbi_ins_info));
#endif
			fbi->ppfb_num = fbnum;

			DBGPRINT(3, "Display %ld 0x%x\n", fbi->ppfb_num,
				 fbi->ppfb_dma[fbi->ppfb_num]);

#ifndef CONFIG_CT656_VBI
			if (fbi->ppfb_num == 0) {
				ptve->INTEn = 0;
				TVE200BaseSet(ptve,
					      fbi->screen_dma,
					      fbi->screen_dma +
					      mach_data->u_offset,
					      fbi->screen_dma +
					      mach_data->v_offset);
			} else if (b_en_irq) {
//printk("set %d at %d\n",fbnum,jiffies);
				TVE200BaseSet(ptve,
					      fbi->ppfb_dma[fbi->ppfb_num],
					      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->u_offset,
					      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->v_offset);
				ptve->INTClr = 0xff;
				ptve->INTEn = TVE_INT_GEN;
			}
//if(g_fci->miss_num)
    //printk("g_fci->miss_num=%d\n",g_fci->miss_num);
//printk("set %d at %d\n",fbnum,jiffies);
#endif
			break;
		}
	case FLCD_SET_SPECIAL_FB:	//0,1.. n
	case FFB_SET_SPECIAL_FB:
		{
			unsigned int fbnum;

			DBGPRINT(3, "FFB_SET_SPECIAL_FB:\n");
			if (copy_from_user
			    (&fbnum, (unsigned int *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
			if (fbnum >= config_fb_number) {
				ret = -EFAULT;
				break;
			}

			fbi->ppfb_num = fbnum;

			DBGPRINT(3, "Display %ld 0x%x\n", fbi->ppfb_num,
				 fbi->ppfb_dma[fbi->ppfb_num]);

			if (fbi->ppfb_num == 0) {
				TVE200BaseSet(ptve,
					      fbi->screen_dma,
					      fbi->screen_dma +
					      mach_data->u_offset,
					      fbi->screen_dma +
					      mach_data->v_offset);
			} else {
//printk("set2 %d at %d\n",fbnum,jiffies);
				TVE200BaseSet(ptve,
					      fbi->ppfb_dma[fbi->ppfb_num],
					      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->u_offset,
					      fbi->ppfb_dma[fbi->ppfb_num] + mach_data->v_offset);
			}

			break;
		}

	case FLCD_BYPASS:
	case FFB_BYPASS:
		{
			unsigned int tmp;
			DBGPRINT(3, "FFB_BYPASS:\n");
			if (copy_from_user
			    (&tmp, (unsigned int *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
//debug_aaron
//			ret = ct656_bypass_on(tmp);
			break;
		}
	case FFB_BYPASS_SRC:
		{
			unsigned int tmp;
			DBGPRINT(3, "FFB_BYPASS_SRC:\n");
			if (copy_from_user
			    (&tmp, (unsigned int *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
//debug_Aaron
	//		ret = ct656_bypass_sel_src(tmp);
			break;
		}
	case FFB_INPUT_RES:
		{
			unsigned int tmp;
			DBGPRINT(3, "FFB_INPUT_RES:\n");
			if (copy_from_user
			    (&tmp, (unsigned int *) arg,
			     sizeof(unsigned int))) {

				//debug_Aaron
				printk("%s: arg 1 error!!!\r\n", __func__);

				ret = -EFAULT;
				break;
			}

			//debug_Aaron
			DBGPRINT(3, "%s: tmp=%d, call tve_set_input_resolution()\r\n", __func__, tmp);

			ret = tve_set_input_resolution(fbi, tmp);
			break;
		}
#ifdef CONFIG_CT656_VBI
	case CT656_VBI_INS_ENABLE:
		{
			unsigned int tmp;
			DBGPRINT(3, "CT656_VBI_INS_ENABLE:\n");
			if (copy_from_user
			    (&tmp, (void __user *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
			if (tmp > 1) {
				ret = -EINVAL;
				break;
			}

			ret = ct656_vbi_ins_enable(tmp);
			break;
		}

	case CT656_VBI_INS_DISABLE:
		DBGPRINT(3, "CT656_VBI_INS_DISABLE:\n");
		ct656_vbi_ins_disable();
		break;

	case CT656_VBI_IOCSDATA:
		DBGPRINT(3, "CT656_VBI_IOCSDATA:\n");
		if (copy_from_user
		    (&g_vci->vbi_info, (void __user *) arg,
		     sizeof(g_vci->vbi_info))) {
			ret = -EFAULT;
			break;
		}
		break;
	case CT656_VBI_IOCSLINE:
		{
			unsigned int tmp;
			DBGPRINT(3, "CT656_VBI_IOCSLINE:\n");
			if (copy_from_user
			    (&tmp, (void __user *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
			if (tmp < 32)
				g_vci->vbi_line = tmp;
			else {
				err("The VBI insertion line index must less than 31.");
				ret = -EFAULT;
			}
			break;
		}
#endif
//debug_Aaron
#if 0
	case CT656_CT_CTRL:
		{
			unsigned int tmp;
			DBGPRINT(3, "CT656_CT_CTRL:\n");
			if (copy_from_user
			    (&tmp, (void __user *) arg,
			     sizeof(unsigned int))) {
				ret = -EFAULT;
				break;
			}
			ret = ct656_color_transform_on(tmp);
			break;
		}
	case CT656_CT_MODE:
		{
			struct ct656_CT_mode tmp;
			DBGPRINT(3, "CT656_CT_MODE:\n");
			if (copy_from_user
			    (&tmp, (void __user *) arg,
			     sizeof(struct ct656_CT_mode))) {
				ret = -EFAULT;
				break;
			}
			ret = ct656_color_transform_mode(&tmp);
			break;
		}
	case CT656_CT_REGION:
		{
			struct ct656_CT_rect tmp;
			DBGPRINT(3, "CT656_CT_REGION:\n");
			if (copy_from_user
			    (&tmp, (void __user *) arg,
			     sizeof(struct ct656_CT_rect))) {
				ret = -EFAULT;
				break;
			}
			ret = ct656_color_transform_region(&tmp);
			break;
		}
#endif
	case FFB_IOSBUFFMT:
	{
		unsigned int tmp;
		DBGPRINT(3, "FFB_IOSBUFFMT:\n");
		if (copy_from_user
		    (&tmp, (unsigned int *) arg,
		     sizeof(unsigned int))) {
			ret = -EFAULT;
			break;
		}
		ret = tve_set_buf_format(fbi, tmp);

		break;
	}
	case FFB_IOGBUFFMT:
	{
		unsigned int tmp;
		DBGPRINT(3, "FFB_IOGBUFFMT:\n");
		ret = tve_get_buf_format(fbi, &tmp);
		if(ret < 0)
			break;

		ret = copy_to_user((void __user *) arg, &tmp,
				   sizeof(tmp)) ? -EFAULT : 0;
		break;
	}

//debug_Aaron support set RGB ordering
 	case FFB_SET_RGB_ORDER:
        {       
                unsigned int tmp;
                DBGPRINT(3, "FFB_SET_RGB_ORDER:\n");
                        if (copy_from_user
                            (&tmp, (unsigned int *) arg,
                             sizeof(unsigned int))) {

                                //debug_Aaron
                                printk("%s: arg 1 error!!!\r\n", __func__);

                                ret = -EFAULT;
                                break;
                        }

                        //debug_Aaron
                        DBGPRINT(3, "%s: tmp=%d, call tve_set_RGB_order()\r\n", __func__, tmp);

                        ret = tve_set_RGB_order(fbi, tmp);
                        break;
                }
//debug_Aaron support set RGB ordering
        case FFB_GET_CTRL1_VAL:
        {
		 struct tve_mach_data *mach_data =
            ((struct ffb_mach_info *) fbi->dev->platform_data)->mach_data;

        	unsigned int tmp = mach_data->ctrl1;

                DBGPRINT(3, "FFB_GET_CTRL1_VAL:\n");
                DBGPRINT(3, "%s: CTRL1=0x%x\r\n", __func__, tmp);

		ret = copy_to_user((void __user *) arg, &tmp,
                                   sizeof(tmp)) ? -EFAULT : 0;
                break;
                }
	default:
		ret = ffb_ioctl(inode, file, cmd, arg, info);
		break;
	}
	DBGLEAVE(1);
	return ret;
}

static struct fb_ops tve_ops = {
	.owner = THIS_MODULE,
	.fb_check_var = ffb_check_var,
	.fb_set_par = ffb_set_par,
	.fb_setcolreg = ffb_setcolreg,

//debug_Aaron, not support yet
//	.fb_fillrect = cfb_fillrect,
//	.fb_copyarea = cfb_copyarea,
//	.fb_imageblit = cfb_imageblit,

	.fb_blank = ffb_blank,

//debug_Aaron, not support yet
//	.fb_cursor = soft_cursor,

	.fb_mmap = ffb_mmap,
	.fb_ioctl = tve_ioctl,
};

static int tve_dev_init(struct ffb_info *fbi)
{
	struct ffb_mach_info *minfo;
	int ret = 0;

	fbi->io_base = IP_va_base[0];
	minfo = kmalloc(sizeof(struct ffb_mach_info), GFP_KERNEL);
	if (!minfo) {
		ret = -ENOMEM;
		goto fail;
	}

	minfo->mach_data =
	    kmalloc(sizeof(struct tve_mach_data), GFP_KERNEL);
	if (!minfo->mach_data) {
		ret = -ENOMEM;
		goto fail;
	}

	minfo->support_modes = pannel_info.support_modes;
	fbi->dev->platform_data = minfo;
	/*
	 * People just don't seem to get this.  We don't support
	 * anything but correct entries now, so panic if someone
	 * does something stupid.
	 */

	strcpy(fbi->fb.fix.id, "ftve");

	fbi->max_xres = pannel_info.max_xres;
	fbi->max_yres = pannel_info.max_yres;
	fbi->max_bpp = pannel_info.max_bpp;
	fbi->fb.var.bits_per_pixel = pannel_info.max_bpp;
	fbi->fb.var.pixclock = pannel_info.pixclock;
	fbi->fb.var.sync = pannel_info.sync;
	fbi->fb.var.grayscale = pannel_info.cmap_grayscale;
	fbi->cmap_inverse = pannel_info.cmap_inverse;
	fbi->cmap_static = pannel_info.cmap_static;

//debug_Aaron, no CT656 support
#if 0
	if (pannel_info.cmap_invert_clock)
		ct656_pixel_clock_invert(1);
	else
		ct656_pixel_clock_invert(0);
#endif

	fbi->smode = FFB_DEFAULT_MODE;
	fbi->fb.fbops = &tve_ops;

	return ret;
 fail:
	if(minfo) {
		if (minfo->mach_data)
			kfree(minfo->mach_data);
		kfree(minfo);
	}
	return ret;
}

static u64 tve_dmamask = ~(u32) 0;

static void tve_platform_release(struct device *dev)
{
	kfree(((struct ffb_mach_info *) dev->platform_data)->mach_data);
	kfree(dev->platform_data);
}

static struct resource tve_resource[] = {
	[0] = {
	       .start = TVE_BASE_ADDR,
	       .end = TVE_END_ADDR,
	       .flags = IORESOURCE_MEM,
	       },
	[1] = {
	       .start = TVE_FTTVEC200_IRQ,
	       .end = TVE_FTTVEC200_IRQ,
	       .flags = IORESOURCE_IRQ,
	       }
};

//debug_Aaron
#if 1 
static struct platform_device tve_device = {
	.name = "ftve",
	.id = -1,
	.num_resources = ARRAY_SIZE(tve_resource),
	.resource = tve_resource,
	.dev = {
		.dma_mask = &tve_dmamask,
		.coherent_dma_mask = 0xffffffff,
		.release = tve_platform_release,
		}
};
#endif
//static struct platform_device tve_device;

//debug_Aaron
struct ffb_info *g_fbi = NULL;

static int tve_probe(struct device *dev)
{
	struct ffb_info *fbi;
	int ret = 0;
	char tbuf[20];
	int i;

	infos("Probe:");

	ret = ffb_probe(dev);
	if (ret < 0)
	{
		//debug_Aaron
		printk("%s: ffb_probe(), ret=%d\r\n", __func__, ret);

		goto err;
	}

	fbi = dev_get_drvdata(dev);

	//debug_Aaron
	g_fbi = fbi;

	i = ffb_identify_mode(tbuf, 19, fbi->smode);
	tbuf[i] = '\0';
	infoe("%s!", tbuf);

	return ret;
      err:
	infoe("Fault(%d)!", ret);
	return ret;
}

static int tve_remove(struct device *dev)
{
	ffb_remove(dev);
	return 0;
}

static struct device_driver tve_driver = {
	.owner = THIS_MODULE,
	.name = "ftve",
	.bus = &platform_bus_type,
	.probe = tve_probe,
	.remove = tve_remove,
	.suspend = ffb_suspend,
	.resume = ffb_resume,
};

#if defined(CONFIG_USE_FIQ)

void prepare_fiq_handler(struct ffb_info *fbi, int irq)
{
	struct pt_regs FIQ_regs;

	fiq_data[TVE_CTRL_BASE]   = fbi->io_base;
	fiq_data[MACH_DATA_BASE]  = (u32)((struct ffb_mach_info *)fbi->dev->platform_data)->mach_data;
	fiq_data[CT656_CTRL_BASE] = CT656_FTCT656_0_VA_BASE;
	fiq_data[INT_CTRL_BASE]   = INTC_FTINTC010_VA_BASE;
	fiq_data[FFB_FBI_BASE]    = (u32)fbi;

	fiq_data[FFB_DO_UPDATING] = (u32)&(g_fci->do_updating);
	fiq_data[FFB_OLD_FB]      = (u32)&(g_fci->old_fbnum);
	fiq_data[FFB_MISS]        = (u32)&(g_fci->miss_num);
#ifdef CONFIG_CT656_VBI
	fiq_data[ACT_VBI_INFO]    = (u32)&(g_vci->act_vbi_info);
	fiq_data[G_VBI_INFO]      = (u32)&(g_vci->vbi_info);
	fiq_data[G_VBI_LINE]      = (u32)&(g_vci->vbi_line);
#endif
	fiq_data[TVE_FIQ_NUM]     = irq;

	FIQ_regs.ARM_sp = (u32)fiq_data;

	set_fiq_regs(&FIQ_regs);

	return;
}

static struct fiq_handler fh = {0};

static int tve_fiq_init(struct platform_device *pdev, struct ffb_info *fbi)
{
	int irq;
	int ret = 0;
	extern unsigned char fiq_handler, end_fiq_handler;
	extern unsigned char fiq_handler_E, end_fiq_handler_E;
	void *fiqhandler_start;
	unsigned int fiqhandler_length;
	u32 chip_ver = ft_get_version();

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		ret = -EINVAL;
		goto end;
	}


	fiq_data = kmalloc(sizeof(u32)*TVE_DATA_LEN, GFP_KERNEL);
	if(!fiq_data) {
		err("Alloc memory fail!\n");
		ret = -ENOMEM;
		goto end;
	}

	fh.name = pdev->name;

	if (claim_fiq(&fh)) {
		err("couldn't claim FIQ.\n");
		ret = -EINVAL;
		goto end;
	}

	if((chip_ver&0xf)>2) {
		fiqhandler_start = &fiq_handler_E;
		fiqhandler_length = &end_fiq_handler_E - &fiq_handler_E;
	}
	else {
		fiqhandler_start = &fiq_handler;
		fiqhandler_length = &end_fiq_handler - &fiq_handler;
	}
	prepare_fiq_handler(fbi, irq);

	set_fiq_handler(fiqhandler_start, fiqhandler_length);

	/* Clean FIQ */
	outl(1<<fiq_data[TVE_FIQ_NUM], INTC_FTINTC010_VA_BASE+FIQ_CLEAR_REG);
	/* Enable FIQ IRQ handler */
	outl(1<<fiq_data[TVE_FIQ_NUM], INTC_FTINTC010_VA_BASE+FIQ_MASK_REG);
 end:
	return ret;
}

static void tve_fiq_release(struct platform_device *pdev, struct ffb_info *fbi)
{
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		goto end;
	}
	
	outl(0, INTC_FTINTC010_VA_BASE+FIQ_MASK_REG);
	/* Clean FIQ */
	outl(1<<irq, INTC_FTINTC010_VA_BASE+FIQ_CLEAR_REG);

        if(fh.name)
		release_fiq(&fh);

	if(fiq_data) {
		kfree(fiq_data);
		fiq_data = 0;
	}
 end:
	return;
}
#else
static int tve_irq_init(struct platform_device *pdev, struct ffb_info *fbi)
{
	int irq;
	int ret = 0;
	u32 chip_ver = ft_get_version();
	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		ret = -EINVAL;
		goto end;
	}

	if((chip_ver&0xf)>2)
		ret = request_irq(irq, ffb_handle_irq_VerE, SA_INTERRUPT,
				  pdev->name, fbi);
	else
		ret = request_irq(irq, ffb_handle_irq, SA_INTERRUPT,
				  pdev->name, fbi);
	if (ret) {
		err("request_irq failed: %d\n", ret);
		goto end;
	}	
 end:
	return ret;
}

static void tve_irq_release(struct platform_device *pdev, struct ffb_info *fbi)
{
	int irq;

	irq = platform_get_irq(pdev, 0);
	if (irq <= 0) {
		goto end;
	}
	
	free_irq(irq, fbi);
 end:
	return;
}
#endif

static struct ffb_mach_ops tve_mach_ops = {
	.ffb_dev_init           = tve_dev_init,
	.ffb_activate_var       = tve_activate_var,
	.ffb_set_var            = tve_set_var,
	.ffb_controller_enable  = tve_controller_enable,
	.ffb_controller_disable = tve_controller_disable,
#if defined(CONFIG_USE_FIQ)
	.ffb_isr_init           = tve_fiq_init,
	.ffb_isr_release        = tve_fiq_release,
#else
	.ffb_isr_init           = tve_irq_init,
	.ffb_isr_release        = tve_irq_release,
#endif
};

static struct timer_list key_autorepeat_timer ;
char g_status_bar_color[]={230,40,40,0};//b,g,r,a
#define STATUS_BAR_START_X 0
#define STATUS_BAR_LEN 222
#define STATUS_BAR_START_Y 340
#define STATUS_BAR_END_Y 350
extern int g_fb_open;    

void coloring(char *addr,int b,int g , int r, int a)
{
	addr[0] = b;//blue
	addr[1] = g;//green
	addr[2] = r;//red
	addr[3] = a;//alpha	  
}
void mv_status_bar()
{
	static int x_axis=STATUS_BAR_START_X;
	int y,x,i=0;
	char *addr;
	addr=g_fbi->fb.screen_base;
	for(y=STATUS_BAR_START_Y; y<STATUS_BAR_END_Y;y++)
	{
		for(x=x_axis;x<=x_axis+STATUS_BAR_LEN;x++)
		{
			addr=g_fbi->fb.screen_base+ (y * MAX_X * 4) + (x%640 * 4);
			coloring(addr,g_status_bar[i],g_status_bar[i+1], g_status_bar[i+2], 0);
			i+=3;
		}	
		
    }
    x_axis+=30;
   	x_axis=x_axis%640;
    if(g_fb_open==0)//frame buffer is not open ye
    {
		key_autorepeat_timer.expires=jiffies+10;
		key_autorepeat_timer.function=&mv_status_bar;
		mod_timer(&key_autorepeat_timer,jiffies + 10);
	}
	else
		del_timer(&key_autorepeat_timer);//marco		
}
void show_img()
{

	char *addr;
	int i=0,z;
	int x =0, y = 480;
	//bitmap pixels are stored "upside-down" with respect to normal image
	addr=g_fbi->fb.screen_base;
	extern char g_src_file[];
	for (z=0; z < g_screen_size; z+=3)
	{
		addr=g_fbi->fb.screen_base+ (y * MAX_X * 4) + (x * 4);
#if 0//marco
		if(y>=217 && y<=262)
#endif
               	if(y>=111 && y<=293)
		{
			coloring(addr,g_src_file[i],g_src_file[i+1], g_src_file[i+2], 0);
	        i+=3;
        }
        else
        {
			coloring(addr,0,0,0,0);
        }	
        x += 1;
		if ( x >= 640)
		{
			x = 0;
			y--;
		}
	}
	init_timer(&key_autorepeat_timer);
	key_autorepeat_timer.expires=jiffies+50;
	key_autorepeat_timer.function=&mv_status_bar;
	add_timer(&key_autorepeat_timer);
	//reg. timer
}
/* Register both the driver and the device */
int __init tve_init(void)
{
	
	int ret = 0;
	gpio0_16low();
//	DBGENTER(1);
	
//debug_Aaron
	printk("%s: MAX_X=%d\r\n", __func__, MAX_X);
	printk("%s: MAX_Y=%d\r\n", __func__, MAX_Y);
	printk("%s: FB_BBP=%d\r\n", __func__, FB_BPP);	
	printk("%s: FB_MODE=%s\r\n", __func__, FB_MODE);	

	//FFB_MAX_XRES = MAX_X;
	//FFB_MAX_RES = MAX_Y;
	res[0].xres = res[1].xres = MAX_X;
	res[0].yres = res[1].yres = MAX_Y;
	pannel_info.max_xres = MAX_X;
	pannel_info.max_yres = MAX_Y;
	pannel_info.max_bpp = FB_BPP;

	if (!strcmp(FB_MODE, "YUV420"))
	{
		FFB_DEFAULT_MODE = DEFAULT_MODE | FFB_MODE_YUV420;  	
	}	
	else if (!strcmp(FB_MODE, "YUV422"))
	{
		FFB_DEFAULT_MODE = DEFAULT_MODE | FFB_MODE_YUV422;  	
	}
	else if (!strcmp(FB_MODE, "RGB"))
	{
		FFB_DEFAULT_MODE = DEFAULT_MODE | FFB_MODE_RGB;  	
	}
	
	

 //debug_Aaron
#ifdef CS4954
        cs4954_init();
#else
	ILI9322_init();
#endif

	g_fci = kmalloc(sizeof(struct frame_ctrl_info), GFP_KERNEL);
	if(!g_fci) {
		ret = -ENOMEM;
		goto err_free;
	}

	memset(g_fci, 0, sizeof(struct frame_ctrl_info));

#ifdef CONFIG_CT656_VBI
	g_vci = kmalloc(sizeof(struct vbi_ctrl_info), GFP_KERNEL);
	if(!g_vci) {
		ret = -ENOMEM;
		goto err_free;
	}

	memset(g_vci, 0, sizeof(struct vbi_ctrl_info));

	g_vci->vbi_line = 12;
#endif
	ffb_device_register(&tve_mach_ops);
	/* Register the device with LDM */
	if (platform_device_register(&tve_device)) {
		err("failed to register device for TVE\n");
		ret = -ENODEV;
		goto err_free;
	}
	
	/* Register the driver with LDM */
	if (driver_register(&tve_driver)) {
		err("failed to register driver for TVE\n");
		platform_device_unregister(&tve_device);
		ret = -ENODEV;
		goto exit;
	}


	//debug_Aaron on 06/12/2008 check whether probe is success
	if (g_fbi == NULL)
	{
		driver_unregister(&tve_driver);
		ret = -ENOMEM;
		goto exit;
	}	
	//debug_Aaron
//	cs4954_init();
	tve_controller_disable(g_fbi);
        tve_controller_enable(g_fbi);
      
	info("[ver:%s]INIT OK!", FFB_VERSION);
	//marco
	unsigned int value;
	#define GLOBAL_BASE_ADDR    (IO_ADDRESS(SL2312_GLOBAL_BASE))
#if 1	
	//value=readl(0x40000010);
	value=readl(GLOBAL_BASE_ADDR+0x10);
	info("[====@@ver:]INIT value %x!",value);
	value = 0x5f0fff;
	value=value|(1<<25)|(1<<27);
	info("[====@@after@@ver:]INIT value %x!",value);
	//writel(value,0x40000010);
	writel(value,GLOBAL_BASE_ADDR+0x10);
	value=readl(GLOBAL_BASE_ADDR+0x10);
	info("[====@@ver:]INIT value %x!",value);
#else
	writel(0x5f0fff,GLOBAL_BASE_ADDR+0x10);
#endif	
//	gpio0_16low();
    show_img();
	gpio0_16high();
	DBGLEAVE(1);	
	return ret;
 exit:
	platform_device_unregister(&tve_device);
 err_free:
	if(g_fci)
		kfree(g_fci);

#ifdef CONFIG_CT656_VBI
	if(g_vci)
		kfree(g_vci);
#endif
	info("[ver:%s]INIT FAIL!", FFB_VERSION);
	DBGLEAVE(1);
	return ret;
}

static void __exit tve_cleanup(void)
{
	DBGENTER(1);

	driver_unregister(&tve_driver);
	platform_device_unregister(&tve_device);

	if(g_fci)
		kfree(g_fci);

#ifdef CONFIG_CT656_VBI
	if(g_vci)
		kfree(g_vci);
#endif
	del_timer(&key_autorepeat_timer);//marco
	DBGLEAVE(1);
}

module_init(tve_init);
module_exit(tve_cleanup);

MODULE_DESCRIPTION("GM TVE driver");
MODULE_AUTHOR("GM Technology Corp.");
MODULE_LICENSE("GPL");
