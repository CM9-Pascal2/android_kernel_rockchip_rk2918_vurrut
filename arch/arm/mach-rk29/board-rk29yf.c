/* arch/arm/mach-rk29/board-rk29.c
 *
 * Copyright (C) 2010 ROCKCHIP, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/skbuff.h>
#include <linux/spi/spi.h>
#include <linux/mmc/host.h>
#include <linux/android_pmem.h>
#ifdef CONFIG_USB_ANDROID
#include <linux/usb/android_composite.h>
#endif
#include <linux/ion.h>

#include <mach/hardware.h>
#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/flash.h>
#include <asm/hardware/gic.h>

#include <mach/iomux.h>
#include <mach/gpio.h>
#include <mach/irqs.h>
#include <mach/rk29_iomap.h>
#include <mach/board.h>
#include <mach/rk29_nand.h>
#include <mach/rk29_camera.h>                          /* ddl@rock-chips.com : camera support */
#include <media/soc_camera.h>                               /* ddl@rock-chips.com : camera support */
#include <mach/vpu_mem.h>
#include <mach/sram.h>
#include <mach/ddr.h>
#include <mach/cpufreq.h>
#include <mach/rk29_smc.h>

#include <linux/regulator/rk29-pwm-regulator.h>
#ifdef CONFIG_TPS65910_CORE
#include <linux/regulator/machine-old.h>
#include <linux/i2c/tps65910.h>
#else
#include <linux/regulator/machine.h>
#endif

#include <linux/regulator/act8891.h>

#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <linux/i2c-gpio.h>
#include <linux/mpu.h>
#include "devices.h"
#include "../../../drivers/input/touchscreen/xpt2046_cbn_ts.h"

#ifdef CONFIG_BU92747GUW_CIR
#include "../../../drivers/cir/bu92747guw_cir.h"
#endif
/*---------------- Camera Sensor Configuration Macro End------------------------*/
#include "../../../drivers/media/video/rk29_camera.c"
/*---------------- Camera Sensor Macro Define End  ------------------------*/


/* Set memory size of pmem */
#ifdef CONFIG_RK29_MEM_SIZE_M
#define SDRAM_SIZE          (CONFIG_RK29_MEM_SIZE_M * SZ_1M)
#else
#define SDRAM_SIZE          SZ_512M
#endif
#define PMEM_GPU_SIZE       SZ_64M

#if (H_VD == 1280)  && (V_VD == 800)
#define PMEM_UI_SIZE        ((32+64)* SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */
#elif (H_VD == 1024) && (V_VD == 768)
#define PMEM_UI_SIZE        ((26+48) * SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */
#elif (H_VD == 1024) && (V_VD == 600)
#define PMEM_UI_SIZE        (60 * SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */
#elif (H_VD == 800) && (V_VD == 600)
#define PMEM_UI_SIZE        ((16+32) * SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */
#elif (H_VD == 800) && (V_VD == 480)
#define PMEM_UI_SIZE        ((12+32) * SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */
#else
#error "not support lcd"
#endif
//#define PMEM_UI_SIZE        (48 * SZ_1M) /* 1280x800: 64M 1024x768: 48M ... */

#define PMEM_VPU_SIZE       SZ_64M
#define PMEM_SKYPE_SIZE     0
#define PMEM_CAM_SIZE       PMEM_CAM_NECESSARY
#ifdef CONFIG_VIDEO_RK29_WORK_IPP
#define MEM_CAMIPP_SIZE     PMEM_CAMIPP_NECESSARY
#else
#define MEM_CAMIPP_SIZE     0
#endif
#define MEM_FB_SIZE         (9*SZ_1M)
#ifdef CONFIG_FB_WORK_IPP
#ifdef CONFIG_FB_SCALING_OSD_1080P
#define MEM_FBIPP_SIZE      SZ_16M   //1920 x 1080 x 2 x 2  //RGB565 = x2;RGB888 = x4
#else
#define MEM_FBIPP_SIZE      SZ_8M   //1920 x 1080 x 2 x 2  //RGB565 = x2;RGB888 = x4
#endif
#else
#define MEM_FBIPP_SIZE      0
#endif
#if SDRAM_SIZE > SZ_512M
#define PMEM_GPU_BASE       (RK29_SDRAM_PHYS + SZ_512M - PMEM_GPU_SIZE)
#else
#define PMEM_GPU_BASE       (RK29_SDRAM_PHYS + SDRAM_SIZE - PMEM_GPU_SIZE)
#endif
#define PMEM_UI_BASE        (PMEM_GPU_BASE - PMEM_UI_SIZE)
#define PMEM_VPU_BASE       (PMEM_UI_BASE - PMEM_VPU_SIZE)
#define PMEM_CAM_BASE       (PMEM_VPU_BASE - PMEM_CAM_SIZE)
#define MEM_CAMIPP_BASE     (PMEM_CAM_BASE - MEM_CAMIPP_SIZE)
#define MEM_FB_BASE         (MEM_CAMIPP_BASE - MEM_FB_SIZE)
#define MEM_FBIPP_BASE      (MEM_FB_BASE - MEM_FBIPP_SIZE)
#define PMEM_SKYPE_BASE     (MEM_FBIPP_BASE - PMEM_SKYPE_SIZE)
#define LINUX_SIZE          (PMEM_SKYPE_BASE - RK29_SDRAM_PHYS)

#define PREALLOC_WLAN_SEC_NUM           4
#define PREALLOC_WLAN_BUF_NUM           160
#define PREALLOC_WLAN_SECTION_HEADER    24

#define WLAN_SECTION_SIZE_0     (PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_1     (PREALLOC_WLAN_BUF_NUM * 128)
#define WLAN_SECTION_SIZE_2     (PREALLOC_WLAN_BUF_NUM * 512)
#define WLAN_SECTION_SIZE_3     (PREALLOC_WLAN_BUF_NUM * 1024)

#define WLAN_SKB_BUF_NUM        16

static struct sk_buff *wlan_static_skb[WLAN_SKB_BUF_NUM];
//----------------------------------------------------------------------------------------------------------------
static int sound_card_exist = 0;
void set_sound_card_exist(int i) 
{
	sound_card_exist=i;
}
EXPORT_SYMBOL(set_sound_card_exist);

int get_sound_card_exist() 
{
	return sound_card_exist;
}
EXPORT_SYMBOL(get_sound_card_exist);

typedef void (*fp_codec_set_spk)(bool on);
static fp_codec_set_spk fp_set_spk=0;
void set_codec_set_spk(fp_codec_set_spk fp)
{
    fp_set_spk=fp;
}
EXPORT_SYMBOL_GPL(set_codec_set_spk);
void codec_set_spk(bool on)
{
    if(fp_set_spk) {
            fp_set_spk(on);
    }
}
EXPORT_SYMBOL_GPL(codec_set_spk);
//---------------------------------------------------------------------------------------------------------------- 
struct wifi_mem_prealloc {
        void *mem_ptr;
        unsigned long size;
};

extern struct sys_timer rk29_timer;

static int rk29_nand_io_init(void)
{
    return 0;
}

struct rk29_nand_platform_data rk29_nand_data = {
    .width      = 1,     /* data bus width in bytes */
    .hw_ecc     = 1,     /* hw ecc 0: soft ecc */
    .num_flash    = 1,
    .io_init   = rk29_nand_io_init,
};

#define TOUCH_SCREEN_STANDBY_VALUE        GPIO_HIGH
#define TOUCH_SCREEN_DISPLAY_PIN          INVALID_GPIO
#define TOUCH_SCREEN_DISPLAY_VALUE        GPIO_HIGH

#ifdef CONFIG_FB_RK29
/*****************************************************************************************
 * lcd  devices
 * author: zyw@rock-chips.com
 *****************************************************************************************/
//#ifdef  CONFIG_LCD_TD043MGEA1
#define LCD_TXD_PIN          INVALID_GPIO
#define LCD_CLK_PIN          INVALID_GPIO
#define LCD_CS_PIN           INVALID_GPIO
/*****************************************************************************************
* frame buffe  devices
* author: zyw@rock-chips.com
*****************************************************************************************/

static int rk29_lcd_io_init(void)
{
    int ret = 0;
    return ret;
}

static int rk29_lcd_io_deinit(void)
{
    int ret = 0;
    return ret;
}

static struct rk29lcd_info rk29_lcd_info = {
    .txd_pin  = LCD_TXD_PIN,
    .clk_pin = LCD_CLK_PIN,
    .cs_pin = LCD_CS_PIN,
    .io_init   = rk29_lcd_io_init,
    .io_deinit = rk29_lcd_io_deinit,
};

int rk29_fb_io_enable(void)
{
#if defined(LVDS_PWR_CTRL_PIN)
	gpio_direction_output(LVDS_PWR_CTRL_PIN, 1);
	gpio_set_value(LVDS_PWR_CTRL_PIN, LVDS_PWR_CTRL_VALUE);
#endif
    if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_DISPLAY_ON_PIN, 0);
        gpio_set_value(FB_DISPLAY_ON_PIN, FB_DISPLAY_ON_VALUE);              
    }
	
    if(FB_LCD_STANDBY_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_LCD_STANDBY_PIN, 0);
        gpio_set_value(FB_LCD_STANDBY_PIN, FB_LCD_STANDBY_VALUE);             
    }
#if defined(LCD_RST_PIN)
	if (LCD_RST_PIN != INVALID_GPIO) 
	{
	#ifdef TIANMA_LCD_USE
		gpio_set_value(LCD_RST_PIN, 1);
	#else
		gpio_direction_output(LCD_RST_PIN, 0);
		gpio_pull_updown(LCD_RST_PIN, 0);
		gpio_set_value(LCD_RST_PIN, 1);
		msleep(5);
		gpio_set_value(LCD_RST_PIN, 0);
		msleep(5);
		gpio_set_value(LCD_RST_PIN, 1);
	#endif
	}
#endif
    return 0;
}

int rk29_fb_io_disable(void)
{
    if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_DISPLAY_ON_PIN, 0);
        gpio_set_value(FB_DISPLAY_ON_PIN, !FB_DISPLAY_ON_VALUE);              
    }

    if(FB_LCD_STANDBY_PIN != INVALID_GPIO)
    {
        gpio_direction_output(FB_LCD_STANDBY_PIN, 0);
        gpio_set_value(FB_LCD_STANDBY_PIN, !FB_LCD_STANDBY_VALUE);             
    }
	
#if defined(LCD_RST_PIN)
	if (LCD_RST_PIN != INVALID_GPIO) 
	{
		gpio_set_value(LCD_RST_PIN, 0);
	}
#endif
	
#if defined(LVDS_PWR_CTRL_PIN)
	gpio_direction_output(LVDS_PWR_CTRL_PIN, 0);
	gpio_set_value(LVDS_PWR_CTRL_PIN, !LVDS_PWR_CTRL_VALUE);
#endif
    return 0;
}

static int rk29_fb_io_init(struct rk29_fb_setting_info *fb_setting)
{
    int ret = 0;
    if(fb_setting->mcu_fmk_en && (FB_MCU_FMK_PIN != INVALID_GPIO))
    {
        ret = gpio_request(FB_MCU_FMK_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_MCU_FMK_PIN);
            printk(">>>>>> FB_MCU_FMK_PIN gpio_request err \n ");
        }
        gpio_direction_input(FB_MCU_FMK_PIN);
    }
#if defined(LVDS_PWR_CTRL_PIN)  
#if defined(LVDS_PWR_CTRL_MUX_NAME)
	rk29_mux_api_set(LVDS_PWR_CTRL_MUX_NAME, LVDS_PWR_CTRL_MUX_MODE);
#endif
	ret = gpio_request(LVDS_PWR_CTRL_PIN, NULL);
	if(ret != 0)
	{
		gpio_free(LVDS_PWR_CTRL_PIN);
		printk(">>>>>> LVDS_PWR_CTRL_PIN gpio_request err \n ");
	}
	gpio_direction_output(LVDS_PWR_CTRL_PIN, LVDS_PWR_CTRL_VALUE);
	gpio_set_value(LVDS_PWR_CTRL_PIN, LVDS_PWR_CTRL_VALUE);
#endif

    if(fb_setting->disp_on_en)
    {
        if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
        {
			/*
			ret = gpio_request(FB_DISPLAY_ON_PIN, NULL);
			if(ret != 0)
			{
				gpio_free(FB_DISPLAY_ON_PIN);
				printk(">>>>>> FB_DISPLAY_ON_PIN gpio_request err \n ");
			}
			*/
        }
        else if (TOUCH_SCREEN_DISPLAY_PIN != INVALID_GPIO)
        {
             ret = gpio_request(TOUCH_SCREEN_DISPLAY_PIN, NULL);
             if(ret != 0)
             {
                 gpio_free(TOUCH_SCREEN_DISPLAY_PIN);
                 printk(">>>>>> TOUCH_SCREEN_DISPLAY_PIN gpio_request err \n ");
             }
             gpio_direction_output(TOUCH_SCREEN_DISPLAY_PIN, 0);
             gpio_set_value(TOUCH_SCREEN_DISPLAY_PIN, TOUCH_SCREEN_DISPLAY_VALUE);
        }
    }

    if(fb_setting->disp_on_en)
    {
        if(FB_LCD_STANDBY_PIN != INVALID_GPIO)
        {
             ret = gpio_request(FB_LCD_STANDBY_PIN, NULL);
             if(ret != 0)
             {
                 gpio_free(FB_LCD_STANDBY_PIN);
                 printk(">>>>>> FB_LCD_STANDBY_PIN gpio_request err \n ");
             }
        }
    }

    if(FB_LCD_CABC_EN_PIN != INVALID_GPIO)
    {
        ret = gpio_request(FB_LCD_CABC_EN_PIN, NULL);
        if(ret != 0)
        {
            gpio_free(FB_LCD_CABC_EN_PIN);
            printk(">>>>>> FB_LCD_CABC_EN_PIN gpio_request err \n ");
        }
        gpio_direction_output(FB_LCD_CABC_EN_PIN, 0);
        gpio_set_value(FB_LCD_CABC_EN_PIN, GPIO_LOW);
    }
#ifdef TIANMA_LCD_USE
    mdelay(300);
#endif
    rk29_fb_io_enable();   //enable it

    return ret;
}


static struct rk29fb_info rk29_fb_info = {
    .fb_id   = FB_ID,
    .mcu_fmk_pin = FB_MCU_FMK_PIN,
    .lcd_info = &rk29_lcd_info,
    .io_init   = rk29_fb_io_init,
    .io_enable = rk29_fb_io_enable,
    .io_disable = rk29_fb_io_disable,
};

/* rk29 fb resource */
static struct resource rk29_fb_resource[] = {
	[0] = {
        .name  = "lcdc reg",
		.start = RK29_LCDC_PHYS,
		.end   = RK29_LCDC_PHYS + RK29_LCDC_SIZE - 1,
		.flags = IORESOURCE_MEM,
	},
	[1] = {
	    .name  = "lcdc irq",
		.start = IRQ_LCDC,
		.end   = IRQ_LCDC,
		.flags = IORESOURCE_IRQ,
	},
	[2] = {
	    .name   = "win1 buf",
        .start  = MEM_FB_BASE,
        .end    = MEM_FB_BASE + MEM_FB_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    #ifdef CONFIG_FB_WORK_IPP
    [3] = {
	    .name   = "win1 ipp buf",
        .start  = MEM_FBIPP_BASE,
        .end    = MEM_FBIPP_BASE + MEM_FBIPP_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    #endif
};

/*platform_device*/
struct platform_device rk29_device_fb = {
	.name		  = "rk29-fb",
	.id		  = 4,
	.num_resources	  = ARRAY_SIZE(rk29_fb_resource),
	.resource	  = rk29_fb_resource,
	.dev            = {
		.platform_data  = &rk29_fb_info,
	}
};

struct platform_device rk29_device_dma_cpy = {
	.name		  = "dma_memcpy",
	.id		  = 4,

};

#endif

#if defined(CONFIG_RK29_GPIO_SUSPEND)
static void key_gpio_pullupdown_enable(void)
{
	gpio_pull_updown(RK29_PIN6_PA0, 0);
	gpio_pull_updown(RK29_PIN6_PA1, 0);
	gpio_pull_updown(RK29_PIN6_PA2, 0);
	gpio_pull_updown(RK29_PIN6_PA3, 0);
	gpio_pull_updown(RK29_PIN6_PA4, 0);
	gpio_pull_updown(RK29_PIN6_PA5, 0);
	gpio_pull_updown(RK29_PIN6_PA6, 0);
}

static void key_gpio_pullupdown_disable(void)
{
	gpio_pull_updown(RK29_PIN6_PA0, 1);
	gpio_pull_updown(RK29_PIN6_PA1, 1);
	gpio_pull_updown(RK29_PIN6_PA2, 1);
	gpio_pull_updown(RK29_PIN6_PA3, 1);
	gpio_pull_updown(RK29_PIN6_PA4, 1);
	gpio_pull_updown(RK29_PIN6_PA5, 1);
	gpio_pull_updown(RK29_PIN6_PA6, 1);
}

void rk29_setgpio_suspend_board(void)
{
	key_gpio_pullupdown_enable();
}

void rk29_setgpio_resume_board(void)
{
	key_gpio_pullupdown_disable();
}
#endif


static struct android_pmem_platform_data android_pmem_pdata = {
	.name		= "pmem",
	.start		= PMEM_UI_BASE,
	.size		= PMEM_UI_SIZE,
	.no_allocator	= 1,
	.cached		= 1,
};

static struct platform_device android_pmem_device = {
	.name		= "android_pmem",
	.id		= 0,
	.dev		= {
		.platform_data = &android_pmem_pdata,
	},
};


static struct vpu_mem_platform_data vpu_mem_pdata = {
	.name		= "vpu_mem",
	.start		= PMEM_VPU_BASE,
	.size		= PMEM_VPU_SIZE,
	.cached		= 1,
};

static struct platform_device rk29_vpu_mem_device = {
	.name		= "vpu_mem",
	.id		    = 2,
	.dev		= {
	.platform_data = &vpu_mem_pdata,
	},
};

#if PMEM_SKYPE_SIZE > 0
static struct android_pmem_platform_data android_pmem_skype_pdata = {
	.name		= "pmem_skype",
	.start		= PMEM_SKYPE_BASE,
	.size		= PMEM_SKYPE_SIZE,
	.no_allocator	= 0,
	.cached		= 0,
};

static struct platform_device android_pmem_skype_device = {
	.name		= "android_pmem",
	.id		= 3,
	.dev		= {
		.platform_data = &android_pmem_skype_pdata,
	},
};
#endif

#ifdef CONFIG_ION
static struct ion_platform_data rk29_ion_pdata = {
	.nr = 1,
	.heaps = {
		{
			.type = ION_HEAP_TYPE_CARVEOUT,
			.id = 0,
			.name = "ui",
			.base = PMEM_UI_BASE,
			.size = PMEM_UI_SIZE,
		}
	},
};

static struct platform_device rk29_ion_device = {
	.name = "ion-rockchip",
	.id = 0,
	.dev = {
		.platform_data = &rk29_ion_pdata,
	},
};
#endif

#ifdef CONFIG_VIDEO_RK29XX_VOUT
static struct platform_device rk29_v4l2_output_devce = {
	.name		= "rk29_vout",
};
#endif


#ifdef CONFIG_GS_KXTF9
#include <linux/kxtf9.h>
#define KXTF9_DEVICE_MAP 1
#define KXTF9_MAP_X (KXTF9_DEVICE_MAP-1)%2
#define KXTF9_MAP_Y KXTF9_DEVICE_MAP%2
#define KXTF9_NEG_X (KXTF9_DEVICE_MAP/2)%2
#define KXTF9_NEG_Y (KXTF9_DEVICE_MAP+1)/4
#define KXTF9_NEG_Z (KXTF9_DEVICE_MAP-1)/4
struct kxtf9_platform_data kxtf9_pdata = {
	.min_interval = 1,
	.poll_interval = 20,
	.g_range = KXTF9_G_2G,
	.axis_map_x = KXTF9_MAP_X,
	.axis_map_y = KXTF9_MAP_Y,
	.axis_map_z = 2,
	.negate_x = KXTF9_NEG_X,
	.negate_y = KXTF9_NEG_Y,
	.negate_z = KXTF9_NEG_Z,
	//.ctrl_regc_init = KXTF9_G_2G | ODR50F,
	//.ctrl_regb_init = ENABLE,
};
#endif /* CONFIG_GS_KXTF9 */


/*MMA8452 gsensor*/
#if defined (CONFIG_GS_MMA8452)

static int mma8452_init_platform_hw(void)
{

    if(gpio_request(MMA8452_INT_PIN,NULL) != 0){
      gpio_free(MMA8452_INT_PIN);
      printk("mma8452_init_platform_hw gpio_request error\n");
      return -EIO;
    }
    gpio_pull_updown(MMA8452_INT_PIN, 1);
    return 0;
}


static struct mma8452_platform_data mma8452_info = {
  .model= 8452,
  .swap_xy = 0,
  .init_platform_hw= mma8452_init_platform_hw,

};
#endif
/*mpu3050*/
#if defined (CONFIG_MPU_SENSORS_MPU3050)
static struct mpu_platform_data mpu3050_data = {
	.int_config = 0x10,
	.orientation = { 1, 0, 0,0, 1, 0, 0, 0, 1 },
};
#endif

/* accel */
#if defined (CONFIG_MPU_SENSORS_KXTF9)
static struct ext_slave_platform_data inv_mpu_kxtf9_data = {
	.bus         = EXT_SLAVE_BUS_SECONDARY,
	.adapt_num = 0,
	.orientation = {1, 0, 0, 0, 1, 0, 0, 0, 1},
};
#endif

/* compass */
#if defined (CONFIG_MPU_SENSORS_AK8975)
static struct ext_slave_platform_data inv_mpu_ak8975_data = {
	.bus         = EXT_SLAVE_BUS_PRIMARY,
	.adapt_num = 0,
	.orientation = {0, 1, 0, -1, 0, 0, 0, 0, 1},
};
#endif


/*************************************PMU ACT8891****************************************/

#if defined (CONFIG_REGULATOR_ACT8891) 
		/*dcdc mode*/
/*act8891 in REGULATOR_MODE_STANDBY mode is said DCDC is in PMF mode is can save power,when in REGULATOR_MODE_NORMAL 
mode is said DCDC is in PWM mode , General default is in REGULATOR_MODE_STANDBY mode*/
		/*ldo mode */
/*act8891 in REGULATOR_MODE_STANDBY mode is said LDO is in low power mode is can save power,when in REGULATOR_MODE_NORMAL 
mode is said DCDC is in nomal mode , General default is in REGULATOR_MODE_STANDBY mode*/
/*set dcdc and ldo voltage by regulator_set_voltage()*/
static struct act8891 *act8891;
int act8891_set_init(struct act8891 *act8891)
{
	int tmp = 0;
	struct regulator *act_ldo1,*act_ldo2,*act_ldo3,*act_ldo4;
	struct regulator *act_dcdc1,*act_dcdc2,*act_dcdc3;

	/*init ldo1*/
	act_ldo1 = regulator_get(NULL, "act_ldo1");
	regulator_enable(act_ldo1); 
	regulator_set_voltage(act_ldo1,1800000,1800000);
	tmp = regulator_get_voltage(act_ldo1);
	regulator_set_mode(act_ldo1,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_ldo1,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: ldo1 vcc =%d\n",tmp);
	regulator_put(act_ldo1);
	 
	/*init ldo2*/
	act_ldo2 = regulator_get(NULL, "act_ldo2");
	regulator_enable(act_ldo2);
	regulator_set_voltage(act_ldo2,1200000,1200000);
	tmp = regulator_get_voltage(act_ldo2);
	regulator_set_mode(act_ldo2,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_ldo2,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: ldo2 vcc =%d\n",tmp);
	regulator_put(act_ldo2);

	/*init ldo3*/
	act_ldo3 = regulator_get(NULL, "act_ldo3");
	regulator_enable(act_ldo3);
	regulator_set_voltage(act_ldo3,3300000,3300000);
	tmp = regulator_get_voltage(act_ldo3);
	regulator_set_mode(act_ldo3,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_ldo3,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: ldo3 vcc =%d\n",tmp);
	regulator_put(act_ldo3);

	/*init ldo4*/
	act_ldo4 = regulator_get(NULL, "act_ldo4");
	regulator_enable(act_ldo4);
	regulator_set_voltage(act_ldo4,2500000,2500000);
	tmp = regulator_get_voltage(act_ldo4);
	regulator_set_mode(act_ldo4,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_ldo4,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: ldo4 vcc =%d\n",tmp);
	regulator_put(act_ldo4);

	/*init dcdc1*/
	act_dcdc1 = regulator_get(NULL, "act_dcdc1");
	regulator_enable(act_dcdc1);
	regulator_set_voltage(act_dcdc1,3000000,3000000);
	tmp = regulator_get_voltage(act_dcdc1);
	regulator_set_mode(act_dcdc1,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_dcdc1,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: dcdc1 vcc =%d\n",tmp); 
	regulator_put(act_dcdc1);

	/*init dcdc2*/
	act_dcdc2 = regulator_get(NULL, "act_dcdc2");
	regulator_enable(act_dcdc2);
	regulator_set_voltage(act_dcdc2,1500000,1500000);
	tmp = regulator_get_voltage(act_dcdc2);
	regulator_set_mode(act_dcdc2,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_dcdc2,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: dcdc2 vcc =%d\n",tmp);
	regulator_put(act_dcdc2);

		/*init dcdc3*/
	act_dcdc3 = regulator_get(NULL, "act_dcdc3");
	regulator_enable(act_dcdc3);
	regulator_set_voltage(act_dcdc3,1200000,1200000);
	tmp = regulator_get_voltage(act_dcdc3);
	regulator_set_mode(act_dcdc3,REGULATOR_MODE_STANDBY);
	//regulator_set_mode(act_dcdc3,REGULATOR_MODE_NORMAL);
	printk("***regulator_set_init: dcdc3 vcc =%d\n",tmp);
	regulator_put(act_dcdc3);

	return(0);
}

static struct regulator_consumer_supply act8891_ldo1_consumers[] = {
	{
		.supply = "act_ldo1",
	}
};

static struct regulator_init_data act8891_ldo1_data = {
	.constraints = {
		.name = "ACT_LDO1",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,		
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_ldo1_consumers),
	.consumer_supplies = act8891_ldo1_consumers,
};

/**/
static struct regulator_consumer_supply act8891_ldo2_consumers[] = {
	{
		.supply = "act_ldo2",
	}
};

static struct regulator_init_data act8891_ldo2_data = {
	.constraints = {
		.name = "ACT_LDO2",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,	
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_ldo2_consumers),
	.consumer_supplies = act8891_ldo2_consumers,
};

/*ldo3 VCC_NAND WIFI/BT/FM_BCM4325*/
static struct regulator_consumer_supply act8891_ldo3_consumers[] = {
	{
		.supply = "act_ldo3",
	}
};

static struct regulator_init_data act8891_ldo3_data = {
	.constraints = {
		.name = "ACT_LDO3",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_ldo3_consumers),
	.consumer_supplies = act8891_ldo3_consumers,
};

/*ldo4 VCCA CODEC_WM8994*/
static struct regulator_consumer_supply act8891_ldo4_consumers[] = {
	{
		.supply = "act_ldo4",
	}
};

static struct regulator_init_data act8891_ldo4_data = {
	.constraints = {
		.name = "ACT_LDO4",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_ldo4_consumers),
	.consumer_supplies = act8891_ldo4_consumers,
};
/*buck1 vcc Core*/
static struct regulator_consumer_supply act8891_dcdc1_consumers[] = {
	{
		.supply = "act_dcdc1",
	}
};

static struct regulator_init_data act8891_dcdc1_data = {
	.constraints = {
		.name = "ACT_DCDC1",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		//.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_dcdc1_consumers),
	.consumer_supplies = act8891_dcdc1_consumers
};

/*buck2 VDDDR MobileDDR VCC*/
static struct regulator_consumer_supply act8891_dcdc2_consumers[] = {
	{
		.supply = "act_dcdc2",
	}
};

static struct regulator_init_data act8891_dcdc2_data = {
	.constraints = {
		.name = "ACT_DCDC2",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		//.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_dcdc2_consumers),
	.consumer_supplies = act8891_dcdc2_consumers
};

/*buck3 vdd Core*/
static struct regulator_consumer_supply act8891_dcdc3_consumers[] = {
	{
		.supply = "act_dcdc3",
	}
};

static struct regulator_init_data act8891_dcdc3_data = {
	.constraints = {
		.name = "ACT_DCDC3",
		.min_uV = 600000,
		.max_uV = 3900000,
		.apply_uV = 1,
		//.always_on = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE | REGULATOR_CHANGE_MODE,
		.valid_modes_mask = REGULATOR_MODE_STANDBY | REGULATOR_MODE_NORMAL,
	},
	.num_consumer_supplies = ARRAY_SIZE(act8891_dcdc3_consumers),
	.consumer_supplies = act8891_dcdc3_consumers
};

struct act8891_regulator_subdev act8891_regulator_subdev_id[] = {
	{
		.id=0,
		.initdata=&act8891_ldo1_data,		
	 },

	{
		.id=1,
		.initdata=&act8891_ldo2_data,		
	 },

	{
		.id=2,
		.initdata=&act8891_ldo3_data,		
	 },

	{
		.id=3,
		.initdata=&act8891_ldo4_data,		
	 },

	{
		.id=4,
		.initdata=&act8891_dcdc1_data,		
	 },

	{
		.id=5,
		.initdata=&act8891_dcdc2_data,		
	 },
	{
		.id=6,
		.initdata=&act8891_dcdc3_data,		
	 },

};

struct act8891_platform_data act8891_data={
	.set_init=act8891_set_init,
	.num_regulators=7,
	.regulators=act8891_regulator_subdev_id,
	
};
#endif

#if defined (CONFIG_TPS65910_CORE)
/* VDD1 */
static struct regulator_consumer_supply rk29_vdd1_supplies[] = {
	{
		.supply = "vcore",	//	set name vcore for all platform 
	},
};

/* VDD1 DCDC */
static struct regulator_init_data rk29_regulator_vdd1 = {
	.constraints = {
		.min_uV = 950000,
		.max_uV = 1400000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE,
		.always_on = true,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vdd1_supplies),
	.consumer_supplies = rk29_vdd1_supplies,
};

/* VDD2 */
static struct regulator_consumer_supply rk29_vdd2_supplies[] = {
	{
		.supply = "vdd2",
	},
};

/* VDD2 DCDC */
static struct regulator_init_data rk29_regulator_vdd2 = {
	.constraints = {
		.min_uV = 1500000,
		.max_uV = 1500000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = true,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vdd2_supplies),
	.consumer_supplies = rk29_vdd2_supplies,
};

/* VIO */
static struct regulator_consumer_supply rk29_vio_supplies[] = {
	{
		.supply = "vio",
	},
};

/* VIO  LDO */
static struct regulator_init_data rk29_regulator_vio = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = true,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vio_supplies),
	.consumer_supplies = rk29_vio_supplies,
};

/* VAUX1 */
static struct regulator_consumer_supply rk29_vaux1_supplies[] = {
	{
		.supply = "vaux1",
	},
};

/* VAUX1  LDO */
static struct regulator_init_data rk29_regulator_vaux1 = {
	.constraints = {
		.min_uV = 2800000,
		.max_uV = 2800000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vaux1_supplies),
	.consumer_supplies = rk29_vaux1_supplies,
};

/* VAUX2 */
static struct regulator_consumer_supply rk29_vaux2_supplies[] = {
	{
		.supply = "vaux2",
	},
};

/* VAUX2  LDO */
static struct regulator_init_data rk29_regulator_vaux2 = {
	.constraints = {
		.min_uV = 2900000,
		.max_uV = 2900000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vaux2_supplies),
	.consumer_supplies = rk29_vaux2_supplies,
};

/* VDAC */
static struct regulator_consumer_supply rk29_vdac_supplies[] = {
	{
		.supply = "vdac",
	},
};

/* VDAC  LDO */
static struct regulator_init_data rk29_regulator_vdac = {
	.constraints = {
		.min_uV = 1800000,
		.max_uV = 1800000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vdac_supplies),
	.consumer_supplies = rk29_vdac_supplies,
};

/* VAUX33 */
static struct regulator_consumer_supply rk29_vaux33_supplies[] = {
	{
		.supply = "vaux33",
	},
};

/* VAUX33  LDO */
static struct regulator_init_data rk29_regulator_vaux33 = {
	.constraints = {
		.min_uV = 3300000,
		.max_uV = 3300000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vaux33_supplies),
	.consumer_supplies = rk29_vaux33_supplies,
};

/* VMMC */
static struct regulator_consumer_supply rk29_vmmc_supplies[] = {
	{
		.supply = "vmmc",
	},
};

/* VMMC  LDO */
static struct regulator_init_data rk29_regulator_vmmc = {
	.constraints = {
		.min_uV = 3000000,
		.max_uV = 3000000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vmmc_supplies),
	.consumer_supplies = rk29_vmmc_supplies,
};

/* VPLL */
static struct regulator_consumer_supply rk29_vpll_supplies[] = {
	{
		.supply = "vpll",
	},
};

/* VPLL  LDO */
static struct regulator_init_data rk29_regulator_vpll = {
	.constraints = {
		.min_uV = 2500000,
		.max_uV = 2500000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = true,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vpll_supplies),
	.consumer_supplies = rk29_vpll_supplies,
};

/* VDIG1 */
static struct regulator_consumer_supply rk29_vdig1_supplies[] = {
	{
		.supply = "vdig1",
	},
};

/* VDIG1  LDO */
static struct regulator_init_data rk29_regulator_vdig1 = {
	.constraints = {
		.min_uV = 2700000,
		.max_uV = 2700000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vdig1_supplies),
	.consumer_supplies = rk29_vdig1_supplies,
};

/* VDIG2 */
static struct regulator_consumer_supply rk29_vdig2_supplies[] = {
	{
		.supply = "vdig2",
	},
};

/* VDIG2  LDO */
static struct regulator_init_data rk29_regulator_vdig2 = {
	.constraints = {
		.min_uV = 1200000,
		.max_uV = 1200000,
		.valid_modes_mask = REGULATOR_MODE_NORMAL,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on = true,
		.apply_uV = true,
	},
	.num_consumer_supplies = ARRAY_SIZE(rk29_vdig2_supplies),
	.consumer_supplies = rk29_vdig2_supplies,
};

static int rk29_tps65910_config(struct tps65910_platform_data *pdata)
{
	u8 val 	= 0;
	int i 	= 0;
	int err = -1;


	/* Configure TPS65910 for rk29 board needs */
	printk("rk29_tps65910_config: tps65910 init config.\n");
	
	err = tps65910_i2c_read_u8(TPS65910_I2C_ID0, &val, TPS65910_REG_DEVCTRL2);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910_REG_DEVCTRL2 reg\n");
		return -EIO;
	}	
	/* Set sleep state active high and allow device turn-off after PWRON long press */
	val |= (TPS65910_DEV2_SLEEPSIG_POL | TPS65910_DEV2_PWON_LP_OFF);

	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val,
			TPS65910_REG_DEVCTRL2);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_DEVCTRL2 reg\n");
		return -EIO;
	}

	/* Set the maxinum load current */
	/* VDD1 */
	err = tps65910_i2c_read_u8(TPS65910_I2C_ID0, &val, TPS65910_REG_VDD1);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910_REG_VDD1 reg\n");
		return -EIO;
	}

	val |= (1<<5);
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDD1);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_VDD1 reg\n");
		return -EIO;
	}

	/* VDD2 */
	err = tps65910_i2c_read_u8(TPS65910_I2C_ID0, &val, TPS65910_REG_VDD2);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910_REG_VDD2 reg\n");
		return -EIO;
	}

	val |= (1<<5);
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDD2);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_VDD2 reg\n");
		return -EIO;
	}

	/* VIO */
	err = tps65910_i2c_read_u8(TPS65910_I2C_ID0, &val, TPS65910_REG_VIO);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910_REG_VIO reg\n");
		return -EIO;
	}

	val |= (1<<6);
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VIO);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_VIO reg\n");
		return -EIO;
	}

	/* Mask ALL interrupts */
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, 0xFF,
			TPS65910_REG_INT_MSK);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_INT_MSK reg\n");
		return -EIO;
	}
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, 0x03,
			TPS65910_REG_INT_MSK2);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_INT_MSK2 reg\n");
		return -EIO;
	}
	
	/* Set RTC Power, disable Smart Reflex in DEVCTRL_REG */
	val = 0;
	val |= (TPS65910_SR_CTL_I2C_SEL);
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val,
			TPS65910_REG_DEVCTRL);
	if (err) {
		printk(KERN_ERR "Unable to write TPS65910_REG_DEVCTRL reg\n");
		return -EIO;
	}

	printk(KERN_INFO "TPS65910 Set default voltage.\n");
#if 1
	/* VDIG1 Set the default voltage from 1800mV to 2700 mV for camera io */
	val = 0x01;
	val |= (0x03 << 2);
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDIG1);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \
				\n", TPS65910_REG_VDIG1);
		return -EIO;
	}
#endif

#if 1
#ifdef CONFIG_MACH_A80HTN 
	/* VADC Set the default voltage 1800mV for camera DVDD */
	val = 0x01;
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDAC);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \
				\n", TPS65910_REG_VDIG1);
		return -EIO;
	}
#endif
#endif

#if 0
	/* VDD1 whitch suplies for core Set the default voltage: 1150 mV(47)*/
	val = 47;	
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDD1_OP);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \
				\n", TPS65910_REG_VDD1_OP);
		return -EIO;
	}
#endif

#if 0
	/* VDD2 whitch suplies for ddr3 Set the default voltage: 1087 * 1.25mV(41)*/
	val = 42;	
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VDD2_OP);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \
				\n", TPS65910_REG_VDD2_OP);
		return -EIO;
	}
#endif

#if 1
	/* VAUX2 whitch suplies for LCD Set the default voltage: 1290mV*/
	val = 0x09;	
	err = tps65910_i2c_write_u8(TPS65910_I2C_ID0, val, TPS65910_REG_VAUX2);
	if (err) {
		printk(KERN_ERR "Unable to read TPS65910 Reg at offset 0x%x= \
				\n", TPS65910_REG_VAUX2);
		return -EIO;
	}
#endif
	/* initilize all ISR work as NULL, specific driver will
	 * assign function(s) later.
	 */
	for (i = 0; i < TPS65910_MAX_IRQS; i++)
		pdata->handlers[i] = NULL;

	return 0;
}

struct tps65910_platform_data rk29_tps65910_data = {
	.irq_num 	= (unsigned)TPS65910_HOST_IRQ,
	.gpio  		= NULL,
	.vio   		= &rk29_regulator_vio,
	.vdd1  		= &rk29_regulator_vdd1,
	.vdd2  		= &rk29_regulator_vdd2,
	.vdd3  		= NULL,
	.vdig1		= &rk29_regulator_vdig1,
	.vdig2		= &rk29_regulator_vdig2,
	.vaux33		= &rk29_regulator_vaux33,
	.vmmc		= &rk29_regulator_vmmc,
	.vaux1		= &rk29_regulator_vaux1,
	.vaux2		= &rk29_regulator_vaux2,
	.vdac		= &rk29_regulator_vdac,
	.vpll		= &rk29_regulator_vpll,
	.board_tps65910_config = rk29_tps65910_config,
};
#endif /* CONFIG_TPS65910_CORE */


/*****************************************************************************************
 * i2c devices
 * author: kfx@rock-chips.com
*****************************************************************************************/
static int rk29_i2c0_io_init(void)
{
#ifdef CONFIG_RK29_I2C0_CONTROLLER
	rk29_mux_api_set(GPIO2B7_I2C0SCL_NAME, GPIO2L_I2C0_SCL);
	rk29_mux_api_set(GPIO2B6_I2C0SDA_NAME, GPIO2L_I2C0_SDA);
#else
	rk29_mux_api_set(GPIO2B7_I2C0SCL_NAME, GPIO2L_GPIO2B7);
	rk29_mux_api_set(GPIO2B6_I2C0SDA_NAME, GPIO2L_GPIO2B6);
#endif
	return 0;
}

static int rk29_i2c1_io_init(void)
{
#ifdef CONFIG_RK29_I2C1_CONTROLLER
	rk29_mux_api_set(GPIO1A7_I2C1SCL_NAME, GPIO1L_I2C1_SCL);
	rk29_mux_api_set(GPIO1A6_I2C1SDA_NAME, GPIO1L_I2C1_SDA);
#else
	rk29_mux_api_set(GPIO1A7_I2C1SCL_NAME, GPIO1L_GPIO1A7);
	rk29_mux_api_set(GPIO1A6_I2C1SDA_NAME, GPIO1L_GPIO1A6);
#endif
	return 0;
}
static int rk29_i2c2_io_init(void)
{
#ifdef CONFIG_RK29_I2C2_CONTROLLER
	rk29_mux_api_set(GPIO5D4_I2C2SCL_NAME, GPIO5H_I2C2_SCL);
	rk29_mux_api_set(GPIO5D3_I2C2SDA_NAME, GPIO5H_I2C2_SDA);
#else
	rk29_mux_api_set(GPIO5D4_I2C2SCL_NAME, GPIO5H_GPIO5D4);
	rk29_mux_api_set(GPIO5D3_I2C2SDA_NAME, GPIO5H_GPIO5D3);
#endif
	return 0;
}

static int rk29_i2c3_io_init(void)
{
#ifdef CONFIG_RK29_I2C3_CONTROLLER
	rk29_mux_api_set(GPIO2B5_UART3RTSN_I2C3SCL_NAME, GPIO2L_I2C3_SCL);
	rk29_mux_api_set(GPIO2B4_UART3CTSN_I2C3SDA_NAME, GPIO2L_I2C3_SDA);
#else
	rk29_mux_api_set(GPIO2B5_UART3RTSN_I2C3SCL_NAME, GPIO2L_GPIO2B5);
	rk29_mux_api_set(GPIO2B4_UART3CTSN_I2C3SDA_NAME, GPIO2L_GPIO2B4);
#endif
	return 0;
}
#ifdef CONFIG_RK29_I2C0_CONTROLLER
struct rk29_i2c_platform_data default_i2c0_data = {
	.bus_num    = 0,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c0_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c0_data = {
       .sda_pin = RK29_PIN2_PB6,
       .scl_pin = RK29_PIN2_PB7,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 0,
       .io_init = rk29_i2c0_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C1_CONTROLLER
struct rk29_i2c_platform_data default_i2c1_data = {
	.bus_num    = 1,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c1_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c1_data = {
       .sda_pin = RK29_PIN1_PA6,
       .scl_pin = RK29_PIN1_PA7,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 1,
       .io_init = rk29_i2c1_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C2_CONTROLLER
struct rk29_i2c_platform_data default_i2c2_data = {
	.bus_num    = 2,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c2_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c2_data = {
       .sda_pin = RK29_PIN5_PD3,
       .scl_pin = RK29_PIN5_PD4,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 2,
       .io_init = rk29_i2c2_io_init,
};
#endif
#ifdef CONFIG_RK29_I2C3_CONTROLLER
struct rk29_i2c_platform_data default_i2c3_data = {
	.bus_num    = 3,
	.flags      = 0,
	.slave_addr = 0xff,
	.scl_rate  = 400*1000,
	.mode 		= I2C_MODE_IRQ,
	.io_init = rk29_i2c3_io_init,
};
#else
struct i2c_gpio_platform_data default_i2c3_data = {
       .sda_pin = RK29_PIN5_PB5,
       .scl_pin = RK29_PIN5_PB4,
       .udelay = 5, // clk = 500/udelay = 100Khz
       .timeout = 100,//msecs_to_jiffies(200),
       .bus_num    = 3,
       .io_init = rk29_i2c3_io_init,
};
#endif

static struct ts_hw_data ts_hw_info = {
	.reset_gpio = TOUCH_RESET_PIN,
	.touch_en_gpio = TOUCH_POWER_PIN,
};

#ifdef CONFIG_I2C0_RK29
static struct i2c_board_info __initdata board_i2c0_devices[] = {
    {
        .type    		= "i2c0_prober",
        .addr           = 0x5a,
        .flags			= 0,
    },

#if defined ( CONFIG_SO381010_TOUCHKEY) 
	{    
		.type           = "so381010",
		.addr           = 0x2c,
		.flags          = 0, 
		.irq            = GPIO_TOUCHKEY_INT,
	},   
#endif

#if defined (CONFIG_RK1000_CONTROL)
	{
		.type    		= "rk1000_control",
		.addr           = 0x40,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_SND_SOC_RT5621)
        {
                .type                   = "rt5621",
                .addr                   = 0x1a,
                .flags                  = 0,
        },
#endif
#if defined (CONFIG_SND_SOC_RT5631)
        {
                .type                   = "rt5631",
                .addr                   = 0x1a,
                .flags                  = 0,
        },
#endif
#if defined (CONFIG_SND_SOC_RK1000)
	{
		.type    		= "rk1000_i2c_codec",
		.addr           = 0x60,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_SND_SOC_WM8988)
	{
		.type                   = "wm8988",
		.addr           = 0x1A,
		.flags                  = 0,
	},
#endif
#if defined (CONFIG_SND_SOC_ES8388)
	{
		.type                   = "es8388",
		.addr           = 0x10,
		.flags                  = 0,
	},
#endif
#if defined (CONFIG_SND_SOC_WM8900)
	{
		.type    		= "wm8900",
		.addr           = 0x1A,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_BATTERY_STC3100)
	{
		.type    		= "stc3100",
		.addr           = 0x70,
		.flags			= 0,
	},
#endif
#if defined (CONFIG_RTC_HYM8563)
	{
		.type    		= "rtc_hym8563",
		.addr           = 0x51,
		.flags			= 0,
		.irq            = GPIO_RTC_INT,
	},
#endif
#if defined(CONFIG_GS_MMA7660)
	{
		.type    		= "mma7660",
		.addr           = 0x4c,
		.flags			= 0,
		.irq            = MMA8452_INT_PIN,
	},
#endif
#if defined (CONFIG_GS_MMA8452)
    {
      .type           = "gs_mma8452",
      .addr           = 0x1c,
      .flags          = 0,
      .irq            = MMA8452_INT_PIN,
      .platform_data  = &mma8452_info,
    },
#endif
#if defined (CONFIG_COMPASS_AK8973)
	{
		.type    		= "ak8973",
		.addr           = 0x1d,
		.flags			= 0,
		.irq			= GPIO_COMP_INT,
	},
#endif
#if defined (CONFIG_COMPASS_AK8975)
	{
		.type    		= "ak8975",
		.addr           = 0x0d,
		.flags			= 0,
		.irq			= GPIO_COMP_INT,
	},
#endif
#if defined (CONFIG_MPU_SENSORS_MPU3050) 
	{
		.type			= "mpu3050",
		.addr			= 0x68,
		.flags			= 0,
		.irq			= GPIO_MPU_INT,
		.platform_data	= &mpu3050_data,
	},
#endif
#if defined (CONFIG_MPU_SENSORS_KXTF9)
	{
		.type    		= "kxtf9",
		.addr           = 0x0f,
		.flags			= 0,	
		.platform_data = &inv_mpu_kxtf9_data,
	},
#endif
#if defined (CONFIG_MPU_SENSORS_AK8975)
	{
		.type			= "ak8975",
		.addr			= 0x0d,
		.flags			= 0,	
		.platform_data = &inv_mpu_ak8975_data,
	},
#endif

#if defined (CONFIG_SND_SOC_CS42L52)
	{
		.type    		= "cs42l52",
		.addr           = 0x4A,
		.flags			= 0,
		.platform_data	= &cs42l52_info,
	},
#endif
#if defined (CONFIG_RTC_M41T66)
	{
		.type           = "rtc-M41T66",
		.addr           = 0x68,
		.flags          = 0,
		.irq            = GPIO_RTC_INT,
	},
#endif
};
#endif
#if defined (CONFIG_ANX7150)
static int anx7150_io_init(void)
{
	int ret = 0;
#ifdef GPIO_ANX7150_RST
	ret = gpio_request(GPIO_ANX7150_RST, "anx7150 reset");
	if(ret)
	{
		gpio_free(GPIO_ANX7150_RST);
		ret = gpio_request(GPIO_ANX7150_RST, "anx7150 reset");
	}
#ifdef ANX7150_RST_MUX_NAME
	rk29_mux_api_set(ANX7150_RST_MUX_NAME, ANX7150_RST_MUX_MODE);
#endif
	gpio_direction_output(GPIO_ANX7150_RST, GPIO_HIGH);
	gpio_set_value(GPIO_ANX7150_RST, GPIO_HIGH);
	mdelay(1);
	gpio_set_value(GPIO_ANX7150_RST, GPIO_LOW);
	mdelay(20);
	gpio_set_value(GPIO_ANX7150_RST, GPIO_HIGH);
#endif
}

struct hdmi_platform_data anx7150_data = {
       .io_init = anx7150_io_init,
};
#endif
#ifdef CONFIG_I2C1_RK29
static struct i2c_board_info __initdata board_i2c1_devices[] = {
#if defined (CONFIG_TOUCHSCREEN_FT5406) || defined (CONFIG_TOUCHSCREEN_FT5606)
#if defined (TOUCH_USE_I2C1)
    {
      .type           = "ft5x0x",
      .addr           = 0x38,
      .flags          = 0,
      .irq            = TOUCH_INT_PIN,
      .platform_data  = &ts_hw_info,
    },
#endif
#endif

#if defined (CONFIG_RK1000_CONTROL1)
	{
		.type			= "rk1000_control",
		.addr			= 0x40,
		.flags			= 0,
	},
#endif
#if defined (ANX7150_ATTACHED_BUS1)
#if defined (CONFIG_ANX7150)
    {
		.type           = "anx7150",
        .addr           = 0x39,             //0x39, 0x3d
        .flags          = 0,
        .irq            = GPIO_HDMI_DET,
		.platform_data  = &anx7150_data,
    },
#endif
#endif
#ifdef CONFIG_BU92747GUW_CIR
    {
    	.type	="bu92747_cir",
    	.addr 	= 0x77,    
    	.flags      =0,
    	.irq		= BU92747_CIR_IRQ_PIN,
    	.platform_data = &bu92747guw_pdata,
    },
#endif

};
#endif

#ifdef CONFIG_I2C2_RK29
static struct i2c_board_info __initdata board_i2c2_devices[] = {
#if defined (CONFIG_HANNSTAR_P1003)
    {
      .type           = "p1003_touch",
      .addr           = 0x04,
      .flags          = 0, //I2C_M_NEED_DELAY
      .irq            = TOUCH_INT_PIN,
      .platform_data  = &p1003_info,
      //.udelay		  = 100
    },
#endif
#if defined (CONFIG_TOUCHSCREEN_GT819)
    {
		.type	= "Goodix-TS",
		.addr 	= 0x55,
		.flags      =0,
		.irq		= TOUCH_INT_PIN,
		.platform_data = &goodix_info,
    },
#endif
#if defined (TOUCH_USE_I2C2)
#if defined (CONFIG_TOUCHSCREEN_FT5406) || defined (CONFIG_TOUCHSCREEN_FT5606)
    {
      .type           = "ft5x0x",
      .addr           = 0x38,
      .flags          = 0,
      .irq            = TOUCH_INT_PIN,
      .platform_data  = &ts_hw_info,
    },
#endif
#if defined(CONFIG_TOUCHSCREEN_ST1564)
	{
		.type           = "st1564_ts",
		.addr           = 0x55,
		.flags          = 0,
		.irq            = TOUCH_INT_PIN,
		.platform_data  = &ts_hw_info,
	},
#endif
#if defined (CONFIG_TOUCHSCREEN_A080SN03)
    {
      .type           = "raydium310",
      .addr           = 0x5c,
      .flags          = 0,
      .irq            = TOUCH_INT_PIN,
      .platform_data  = &ts_hw_info,
    },
#endif
#if defined (CONFIG_TPS65910_CORE)
	{
		.type           = "tps659102",
		.addr           = TPS65910_I2C_ID0,
		.flags          = 0,
		.irq            = TPS65910_HOST_IRQ,
		.platform_data  = &rk29_tps65910_data,
	},
#endif
#if defined (CONFIG_TOUCHSCREEN_BU21020)
    {
    	.type           = "bu21020",
    	.addr           = 0x5C,
    	.flags          = 0,
     	.irq            = TOUCH_INT_PIN,
    	.platform_data  = &ts_hw_info,
    },
#endif
#if defined(CONFIG_TOUCHSCREEN_TOUCHPLUS)
	{
		.type           = "touchplus_ts",
		.addr           = 0x5C,
		.flags          = 0,
		.irq            = TOUCH_INT_PIN,
		.platform_data  = &ts_hw_info,
	},
#endif
#endif
};
#endif

#ifdef CONFIG_I2C3_RK29
static struct i2c_board_info __initdata board_i2c3_devices[] = {
#if defined (ANX7150_ATTACHED_BUS3)
#if defined (CONFIG_ANX7150)
    {
		.type           = "anx7150",
        .addr           = 0x39,             //0x39, 0x3d
        .flags          = 0,
        .irq            = GPIO_HDMI_DET,
		.platform_data  = &anx7150_data,
    },
#endif
#endif
#if defined (CONFIG_BATTERY_BQ27541)
	{
		.type    		= "bq27541",
		.addr           = 0x55,
		.flags			= 0,
		.platform_data  = &bq27541_info,
	},
#endif
#if defined (CONFIG_REGULATOR_ACT8891)
	{
		.type    		= "act8891",
		.addr           = 0x5b, 
		.flags			= 0,
		.platform_data=&act8891_data,
	},
#endif

};
#endif

/*****************************************************************************************
 * camera  devices
 * author: ddl@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_VIDEO_RK29
#define CONFIG_SENSOR_POWER_IOCTL_USR      0
#define CONFIG_SENSOR_RESET_IOCTL_USR      0
#define CONFIG_SENSOR_POWERDOWN_IOCTL_USR      0
#define CONFIG_SENSOR_FLASH_IOCTL_USR      0

#if CONFIG_SENSOR_POWER_IOCTL_USR
static int sensor_power_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_POWER_IOCTL_USR is 1, sensor_power_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_RESET_IOCTL_USR
static int sensor_reset_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_RESET_IOCTL_USR is 1, sensor_reset_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
static int sensor_powerdown_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_POWERDOWN_IOCTL_USR is 1, sensor_powerdown_usr_cb function must be writed!!";
}
#endif

#if CONFIG_SENSOR_FLASH_IOCTL_USR
static int sensor_flash_usr_cb (struct rk29camera_gpio_res *res,int on)
{
    #error "CONFIG_SENSOR_FLASH_IOCTL_USR is 1, sensor_flash_usr_cb function must be writed!!";
}
#endif

static struct rk29camera_platform_ioctl_cb  sensor_ioctl_cb = {
    #if CONFIG_SENSOR_POWER_IOCTL_USR
    .sensor_power_cb = sensor_power_usr_cb,
    #else
    .sensor_power_cb = NULL,
    #endif

    #if CONFIG_SENSOR_RESET_IOCTL_USR
    .sensor_reset_cb = sensor_reset_usr_cb,
    #else
    .sensor_reset_cb = NULL,
    #endif

    #if CONFIG_SENSOR_POWERDOWN_IOCTL_USR
    .sensor_powerdown_cb = sensor_powerdown_usr_cb,
    #else
    .sensor_powerdown_cb = NULL,
    #endif

    #if CONFIG_SENSOR_FLASH_IOCTL_USR
    .sensor_flash_cb = sensor_flash_usr_cb,
    #else
    .sensor_flash_cb = NULL,
    #endif
};
#include "../../../drivers/media/video/rk29_camera.c"
#endif

#ifdef CONFIG_BATTERY_RK2918
static struct rk2918_battery_platform_data rk2918_battery_data = {
		.dc_det_pin = GPIO_DC_DET,
		.charge_ok_pin = GPIO_CHG_OK,
		.dc_det_level	= DC_DET_EFFECTIVE,
		.charge_ok_level= CHG_OK_EFFECTIVE,
		.charge_set_pin	= INVALID_GPIO,
#ifdef DC_CURRENT_IN_TWO_MODE          
		.charge_cur_ctl = GPIO_CURRENT_CONTROL,
		.charge_cur_ctl_level = GPIO_CURRENT_CONTROL_LEVEL,
#else
		.charge_cur_ctl = INVALID_GPIO,
		.charge_cur_ctl_level = GPIO_LOW,
#endif
};

struct platform_device rk2918_device_battery = {
		.name   = "rk2918-battery",
		.id     = -1,
		.dev = {
				.platform_data = &rk2918_battery_data,
		}
};
#endif

/*****************************************************************************************
 * backlight  devices
 * author: nzy@rock-chips.com
 *****************************************************************************************/
#ifdef CONFIG_BACKLIGHT_RK29_BL
 /*
 GPIO1B5_PWM0_NAME,       GPIO1L_PWM0
 GPIO5D2_PWM1_UART1SIRIN_NAME,  GPIO5H_PWM1
 GPIO2A3_SDMMC0WRITEPRT_PWM2_NAME,   GPIO2L_PWM2
 GPIO1A5_EMMCPWREN_PWM3_NAME,     GPIO1L_PWM3
 */


static int rk29_backlight_io_init(void)
{
    int ret = 0;

    rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);
#ifdef BL_EN_PIN 
#ifdef BLEN_MUX_NAME
    rk29_mux_api_set(BL_EN_MUX_NAME, BL_EN_MUX_MODE);
#endif

    ret = gpio_request(BL_EN_PIN, NULL);
    if(ret != 0)
    {
        gpio_free(BL_EN_PIN);
    }

    gpio_direction_output(BL_EN_PIN, 0);
    gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
#endif
    return ret;
}

static int rk29_backlight_io_deinit(void)
{
    int ret = 0;
    #ifdef  LCD_DISP_ON_PIN
    gpio_free(BL_EN_PIN);
    #endif
    rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
    
    return ret;
}

static int rk29_backlight_pwm_suspend(void)
{
	int ret = 0;
	rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	if (gpio_request(PWM_GPIO, NULL)) {
		printk("func %s, line %d: request gpio fail\n", __FUNCTION__, __LINE__);
		return -1;
	}
	gpio_direction_output(PWM_GPIO, GPIO_HIGH);
   #ifdef  LCD_DISP_ON_PIN
    gpio_direction_output(BL_EN_PIN, 0);
    gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
   #endif
	return ret;
}

static int rk29_backlight_pwm_resume(void)
{
	msleep(200);

	gpio_free(PWM_GPIO);
	rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE);

    #ifdef  LCD_DISP_ON_PIN
    msleep(30);
    gpio_direction_output(BL_EN_PIN, 1);
    gpio_set_value(BL_EN_PIN, BL_EN_VALUE);
    #endif
	return 0;
}

struct rk29_bl_info rk29_bl_info = {
    .pwm_id   = PWM_ID,
    .bl_ref   = PWM_EFFECT_VALUE,
    .io_init   = rk29_backlight_io_init,
    .io_deinit = rk29_backlight_io_deinit,
    .pwm_suspend = rk29_backlight_pwm_suspend,
    .pwm_resume = rk29_backlight_pwm_resume,
};
#endif
/*****************************************************************************************
* pwm voltage regulator devices
******************************************************************************************/
#if defined (CONFIG_RK29_PWM_REGULATOR)

static struct regulator_consumer_supply pwm_consumers[] = {
	{
		.supply = "vcore",
	}
};

static struct regulator_init_data rk29_pwm_regulator_data = {
	.constraints = {
		.name = "PWM2",
		.min_uV =  950000,
		.max_uV = 1400000,
		.apply_uV = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
	},
	.num_consumer_supplies = ARRAY_SIZE(pwm_consumers),
	.consumer_supplies = pwm_consumers,
};

static struct pwm_platform_data rk29_regulator_pwm_platform_data = {
	.pwm_id = REGULATOR_PWM_ID,
	.pwm_gpio = REGULATOR_PWM_GPIO,
	//.pwm_iomux_name[] = REGULATOR_PWM_MUX_NAME;
	.pwm_iomux_name = REGULATOR_PWM_MUX_NAME,
	.pwm_iomux_pwm = REGULATOR_PWM_MUX_MODE,
	.pwm_iomux_gpio = REGULATOR_PWM_MUX_MODE_GPIO,
	.init_data  = &rk29_pwm_regulator_data,
};

static struct platform_device rk29_device_pwm_regulator = {
	.name = "pwm-voltage-regulator",
	.id   = -1,
	.dev  = {
		.platform_data = &rk29_regulator_pwm_platform_data,
	},
};

#endif


/*****************************************************************************************
 * SDMMC devices
*****************************************************************************************/
#if !defined(CONFIG_SDMMC_RK29_OLD)	
static void rk29_sdmmc_gpio_open(int device_id, int on)
{
    switch(device_id)
    {
        case 0://mmc0
        {
            #ifdef CONFIG_SDMMC0_RK29
            if(on)
            {
                gpio_direction_output(RK29_PIN1_PD0,GPIO_HIGH);//set mmc0-clk to high
                gpio_direction_output(RK29_PIN1_PD1,GPIO_HIGH);//set mmc0-cmd to high.
                gpio_direction_output(RK29_PIN1_PD2,GPIO_HIGH);//set mmc0-data0 to high.
                gpio_direction_output(RK29_PIN1_PD3,GPIO_HIGH);//set mmc0-data1 to high.
                gpio_direction_output(RK29_PIN1_PD4,GPIO_HIGH);//set mmc0-data2 to high.
                gpio_direction_output(RK29_PIN1_PD5,GPIO_HIGH);//set mmc0-data3 to high.

                mdelay(30);
            }
            else
            {
                rk29_mux_api_set(GPIO1D0_SDMMC0CLKOUT_NAME, GPIO1H_GPIO1_D0);
                gpio_request(RK29_PIN1_PD0, "mmc0-clk");
                gpio_direction_output(RK29_PIN1_PD0,GPIO_LOW);//set mmc0-clk to low.

                rk29_mux_api_set(GPIO1D1_SDMMC0CMD_NAME, GPIO1H_GPIO1_D1);
                gpio_request(RK29_PIN1_PD1, "mmc0-cmd");
                gpio_direction_output(RK29_PIN1_PD1,GPIO_LOW);//set mmc0-cmd to low.

                rk29_mux_api_set(GPIO1D2_SDMMC0DATA0_NAME, GPIO1H_GPIO1D2);
                gpio_request(RK29_PIN1_PD2, "mmc0-data0");
                gpio_direction_output(RK29_PIN1_PD2,GPIO_LOW);//set mmc0-data0 to low.

                rk29_mux_api_set(GPIO1D3_SDMMC0DATA1_NAME, GPIO1H_GPIO1D3);
                gpio_request(RK29_PIN1_PD3, "mmc0-data1");
                gpio_direction_output(RK29_PIN1_PD3,GPIO_LOW);//set mmc0-data1 to low.

                rk29_mux_api_set(GPIO1D4_SDMMC0DATA2_NAME, GPIO1H_GPIO1D4);
                gpio_request(RK29_PIN1_PD4, "mmc0-data2");
                gpio_direction_output(RK29_PIN1_PD4,GPIO_LOW);//set mmc0-data2 to low.

                rk29_mux_api_set(GPIO1D5_SDMMC0DATA3_NAME, GPIO1H_GPIO1D5);
                gpio_request(RK29_PIN1_PD5, "mmc0-data3");
                gpio_direction_output(RK29_PIN1_PD5,GPIO_LOW);//set mmc0-data3 to low.

                mdelay(30);
            }
            #endif
        }
        break;
        
        case 1://mmc1
        {
            #ifdef CONFIG_SDMMC1_RK29
            if(on)
            {
                gpio_direction_output(RK29_PIN1_PC7,GPIO_HIGH);//set mmc1-clk to high
                gpio_direction_output(RK29_PIN1_PC2,GPIO_HIGH);//set mmc1-cmd to high.
                gpio_direction_output(RK29_PIN1_PC3,GPIO_HIGH);//set mmc1-data0 to high.
                gpio_direction_output(RK29_PIN1_PC4,GPIO_HIGH);//set mmc1-data1 to high.
                gpio_direction_output(RK29_PIN1_PC5,GPIO_HIGH);//set mmc1-data2 to high.
                gpio_direction_output(RK29_PIN1_PC6,GPIO_HIGH);//set mmc1-data3 to high.
                mdelay(100);
            }
            else
            {
                rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_GPIO1C7);
                gpio_request(RK29_PIN1_PC7, "mmc1-clk");
                gpio_direction_output(RK29_PIN1_PC7,GPIO_LOW);//set mmc1-clk to low.

                rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_GPIO1C2);
                gpio_request(RK29_PIN1_PC2, "mmc1-cmd");
                gpio_direction_output(RK29_PIN1_PC2,GPIO_LOW);//set mmc1-cmd to low.

                rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_GPIO1C3);
                gpio_request(RK29_PIN1_PC3, "mmc1-data0");
                gpio_direction_output(RK29_PIN1_PC3,GPIO_LOW);//set mmc1-data0 to low.

                mdelay(100);
            }
            #endif
        }
        break; 
        
        case 2: //mmc2
        break;
        
        default:
        break;
    }
}

static void rk29_sdmmc_set_iomux_mmc0(unsigned int bus_width)
{
    switch (bus_width)
    {
        
    	case 1://SDMMC_CTYPE_4BIT:
    	{
        	rk29_mux_api_set(GPIO1D3_SDMMC0DATA1_NAME, GPIO1H_SDMMC0_DATA1);
        	rk29_mux_api_set(GPIO1D4_SDMMC0DATA2_NAME, GPIO1H_SDMMC0_DATA2);
        	rk29_mux_api_set(GPIO1D5_SDMMC0DATA3_NAME, GPIO1H_SDMMC0_DATA3);
    	}
    	break;

    	case 0x10000://SDMMC_CTYPE_8BIT:
    	    break;
    	case 0xFFFF: //gpio_reset
    	{
            rk29_mux_api_set(GPIO5D5_SDMMC0PWREN_NAME, GPIO5H_GPIO5D5);   
            gpio_request(RK29_PIN5_PD5,"sdmmc-power");
            gpio_direction_output(RK29_PIN5_PD5,GPIO_HIGH); //power-off

            rk29_sdmmc_gpio_open(0, 0);

            gpio_direction_output(RK29_PIN5_PD5,GPIO_LOW); //power-on

            rk29_sdmmc_gpio_open(0, 1);
    	}
    	break;

    	default: //case 0://SDMMC_CTYPE_1BIT:
        {
        	rk29_mux_api_set(GPIO1D1_SDMMC0CMD_NAME, GPIO1H_SDMMC0_CMD);
        	rk29_mux_api_set(GPIO1D0_SDMMC0CLKOUT_NAME, GPIO1H_SDMMC0_CLKOUT);
        	rk29_mux_api_set(GPIO1D2_SDMMC0DATA0_NAME, GPIO1H_SDMMC0_DATA0);

        	rk29_mux_api_set(GPIO1D3_SDMMC0DATA1_NAME, GPIO1H_GPIO1D3);
        	gpio_request(RK29_PIN1_PD3, "mmc0-data1");
        	gpio_direction_output(RK29_PIN1_PD3,GPIO_HIGH);

        	rk29_mux_api_set(GPIO1D4_SDMMC0DATA2_NAME, GPIO1H_GPIO1D4);
        	gpio_request(RK29_PIN1_PD4, "mmc0-data2");
        	gpio_direction_output(RK29_PIN1_PD4,GPIO_HIGH);
        	
            rk29_mux_api_set(GPIO1D5_SDMMC0DATA3_NAME, GPIO1H_GPIO1D5);
            gpio_request(RK29_PIN1_PD5, "mmc0-data3");
        	gpio_direction_output(RK29_PIN1_PD5,GPIO_HIGH);
    	}
    	break;
	}
}

static void rk29_sdmmc_set_iomux_mmc1(unsigned int bus_width)
{
#if 0
    switch (bus_width)
    {
        
    	case 1://SDMMC_CTYPE_4BIT:
    	{
            rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_SDMMC1_CMD);
            rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_SDMMC1_CLKOUT);
            rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_SDMMC1_DATA0);
            rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_SDMMC1_DATA1);
            rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_SDMMC1_DATA2);
            rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_SDMMC1_DATA3);
    	}
    	break;

    	case 0x10000://SDMMC_CTYPE_8BIT:
    	    break;
    	case 0xFFFF:
    	{
    	   rk29_sdmmc_gpio_open(1, 0); 
    	   rk29_sdmmc_gpio_open(1, 1);
    	}
    	break;

    	default: //case 0://SDMMC_CTYPE_1BIT:
        {
            rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_SDMMC1_CMD);
        	rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_SDMMC1_CLKOUT);
        	rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_SDMMC1_DATA0);

            rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_GPIO1C4);
            gpio_request(RK29_PIN1_PC4, "mmc1-data1");
        	gpio_direction_output(RK29_PIN1_PC4,GPIO_HIGH);
        	
            rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_GPIO1C5);
            gpio_request(RK29_PIN1_PC5, "mmc1-data2");
        	gpio_direction_output(RK29_PIN1_PC5,GPIO_HIGH);

            rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_GPIO1C6);
            gpio_request(RK29_PIN1_PC6, "mmc1-data3");
        	gpio_direction_output(RK29_PIN1_PC6,GPIO_HIGH);

    	}
    	break;
	}
#else
    rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_SDMMC1_CMD);
    rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_SDMMC1_CLKOUT);
    rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_SDMMC1_DATA0);
    rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_SDMMC1_DATA1);
    rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_SDMMC1_DATA2);
    rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_SDMMC1_DATA3);

#endif
}

static void rk29_sdmmc_set_iomux_mmc2(unsigned int bus_width)
{
    ;//
}


static void rk29_sdmmc_set_iomux(int device_id, unsigned int bus_width)
{
    switch(device_id)
    {
        case 0:
            #ifdef CONFIG_SDMMC0_RK29
            rk29_sdmmc_set_iomux_mmc0(bus_width);
            #endif
            break;
        case 1:
            #ifdef CONFIG_SDMMC1_RK29
            rk29_sdmmc_set_iomux_mmc1(bus_width);
            #endif
            break;
        case 2:
            rk29_sdmmc_set_iomux_mmc2(bus_width);
            break;
        default:
            break;
    }    
}

#endif

#ifdef CONFIG_WIFI_CONTROL_FUNC
static int rk29sdk_wifi_status(struct device *dev);
static int rk29sdk_wifi_status_register(void (*callback)(int card_presend, void *dev_id), void *dev_id);
#endif

#ifdef CONFIG_SDMMC0_RK29
static int rk29_sdmmc0_cfg_gpio(void)
{
#ifdef CONFIG_SDMMC_RK29_OLD	
    rk29_mux_api_set(GPIO1D1_SDMMC0CMD_NAME, GPIO1H_SDMMC0_CMD);
	rk29_mux_api_set(GPIO1D0_SDMMC0CLKOUT_NAME, GPIO1H_SDMMC0_CLKOUT);
	rk29_mux_api_set(GPIO1D2_SDMMC0DATA0_NAME, GPIO1H_SDMMC0_DATA0);
	rk29_mux_api_set(GPIO1D3_SDMMC0DATA1_NAME, GPIO1H_SDMMC0_DATA1);
	rk29_mux_api_set(GPIO1D4_SDMMC0DATA2_NAME, GPIO1H_SDMMC0_DATA2);
	rk29_mux_api_set(GPIO1D5_SDMMC0DATA3_NAME, GPIO1H_SDMMC0_DATA3);
	
	rk29_mux_api_set(GPIO2A2_SDMMC0DETECTN_NAME, GPIO2L_GPIO2A2);

    rk29_mux_api_set(GPIO5D5_SDMMC0PWREN_NAME, GPIO5H_GPIO5D5);   ///GPIO5H_SDMMC0_PWR_EN);  ///GPIO5H_GPIO5D5);
	gpio_request(RK29_PIN5_PD5,"sdmmc");
#if 0
	gpio_set_value(RK29_PIN5_PD5,GPIO_HIGH);
	mdelay(100);
	gpio_set_value(RK29_PIN5_PD5,GPIO_LOW);
#else
	gpio_direction_output(RK29_PIN5_PD5,GPIO_LOW);
#endif

#else
    rk29_sdmmc_set_iomux(0, 0xFFFF);
    
	rk29_mux_api_set(GPIO2A2_SDMMC0DETECTN_NAME, GPIO2L_SDMMC0_DETECT_N);//Modifyed by xbw.

	#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
    gpio_request(SDMMC0_WRITE_PROTECT_PIN,"sdmmc-wp");
    gpio_direction_input(SDMMC0_WRITE_PROTECT_PIN);	    
    #endif

#endif

	return 0;
}

#define CONFIG_SDMMC0_USE_DMA
struct rk29_sdmmc_platform_data default_sdmmc0_data = {
	.host_ocr_avail = (MMC_VDD_25_26|MMC_VDD_26_27|MMC_VDD_27_28|MMC_VDD_28_29|MMC_VDD_29_30|
					   MMC_VDD_30_31|MMC_VDD_31_32|MMC_VDD_32_33|
					   MMC_VDD_33_34|MMC_VDD_34_35| MMC_VDD_35_36),
	.host_caps 	= (MMC_CAP_4_BIT_DATA|MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED),
	.io_init = rk29_sdmmc0_cfg_gpio,
	
#if !defined(CONFIG_SDMMC_RK29_OLD)		
	.set_iomux = rk29_sdmmc_set_iomux,
#endif

	.dma_name = "sd_mmc",
#ifdef CONFIG_SDMMC0_USE_DMA
	.use_dma  = 1,
#else
	.use_dma = 0,
#endif
	.detect_irq = RK29_PIN2_PA2, // INVALID_GPIO
	.enable_sd_wakeup = 0,

#if defined(CONFIG_SDMMC0_RK29_WRITE_PROTECT)
    .write_prt = SDMMC0_WRITE_PROTECT_PIN,
#else
    .write_prt = INVALID_GPIO,
#endif
};
#endif
#ifdef CONFIG_SDMMC1_RK29
#define CONFIG_SDMMC1_USE_DMA
static int rk29_sdmmc1_cfg_gpio(void)
{
#if defined(CONFIG_SDMMC_RK29_OLD)
	rk29_mux_api_set(GPIO1C2_SDMMC1CMD_NAME, GPIO1H_SDMMC1_CMD);
	rk29_mux_api_set(GPIO1C7_SDMMC1CLKOUT_NAME, GPIO1H_SDMMC1_CLKOUT);
	rk29_mux_api_set(GPIO1C3_SDMMC1DATA0_NAME, GPIO1H_SDMMC1_DATA0);
	rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_SDMMC1_DATA1);
	rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_SDMMC1_DATA2);
	rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_SDMMC1_DATA3);
	//rk29_mux_api_set(GPIO1C0_UART0CTSN_SDMMC1DETECTN_NAME, GPIO1H_SDMMC1_DETECT_N);

#else

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
    gpio_request(SDMMC1_WRITE_PROTECT_PIN,"sdio-wp");
    gpio_direction_input(SDMMC1_WRITE_PROTECT_PIN);	    
#endif

#endif

	return 0;
}




struct rk29_sdmmc_platform_data default_sdmmc1_data = {
	.host_ocr_avail = (MMC_VDD_25_26|MMC_VDD_26_27|MMC_VDD_27_28|MMC_VDD_28_29|
					   MMC_VDD_29_30|MMC_VDD_30_31|MMC_VDD_31_32|
					   MMC_VDD_32_33|MMC_VDD_33_34),

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)					   
	.host_caps 	= (MMC_CAP_4_BIT_DATA|MMC_CAP_SDIO_IRQ|
				   MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED),
#else
    .host_caps 	= (MMC_CAP_4_BIT_DATA|MMC_CAP_MMC_HIGHSPEED|MMC_CAP_SD_HIGHSPEED),
#endif

	.io_init = rk29_sdmmc1_cfg_gpio,
	
#if !defined(CONFIG_SDMMC_RK29_OLD)		
	.set_iomux = rk29_sdmmc_set_iomux,
#endif	

	.dma_name = "sdio",
#ifdef CONFIG_SDMMC1_USE_DMA
	.use_dma  = 1,
#else
	.use_dma = 0,
#endif

#if !defined(CONFIG_USE_SDMMC1_FOR_WIFI_DEVELOP_BOARD)
#ifdef CONFIG_WIFI_CONTROL_FUNC
        .status = rk29sdk_wifi_status,
        .register_status_notify = rk29sdk_wifi_status_register,
#endif

#if defined(CONFIG_SDMMC1_RK29_WRITE_PROTECT)
    .write_prt = SDMMC1_WRITE_PROTECT_PIN,
#else
    .write_prt = INVALID_GPIO, 
#endif  

#else
//for wifi develop board
    .detect_irq = INVALID_GPIO,
    .enable_sd_wakeup = 0,
#endif

};
#endif ////endif--#ifdef CONFIG_SDMMC1_RK29


int rk29sdk_wifi_power_state = 0;
int rk29sdk_bt_power_state = 0;

#ifdef CONFIG_WIFI_CONTROL_FUNC

static int rk29sdk_wifi_cd = 0;   /* wifi virtual 'card detect' status */
static void (*wifi_status_cb)(int card_present, void *dev_id);
static void *wifi_status_cb_devid;

static int rk29sdk_wifi_status(struct device *dev)
{
        return rk29sdk_wifi_cd;
}

static int rk29sdk_wifi_status_register(void (*callback)(int card_present, void *dev_id), void *dev_id)
{
        if(wifi_status_cb)
                return -EAGAIN;
        wifi_status_cb = callback;
        wifi_status_cb_devid = dev_id;
        return 0;
}

static int rk29sdk_wifi_bt_gpio_control_init(void)
{
    if (gpio_request(GPIO_WIFI_POWER, "wifi_bt_power")) {
           pr_info("%s: request wifi_bt power gpio failed\n", __func__);
           return -1;
    }

    if (gpio_request(GPIO_WIFI_RESET, "wifi reset")) {
           pr_info("%s: request wifi reset gpio failed\n", __func__);
           gpio_free(GPIO_WIFI_POWER);
           return -1;
    }

#if defined(GPIO_BT_RESET)
    if (gpio_request(GPIO_BT_RESET, "bt reset")) {
          pr_info("%s: request bt reset gpio failed\n", __func__);
          gpio_free(GPIO_WIFI_RESET);
          return -1;
    }
    gpio_direction_output(GPIO_BT_RESET,      GPIO_LOW);
#endif

    gpio_direction_output(GPIO_WIFI_POWER, GPIO_LOW);
    gpio_direction_output(GPIO_WIFI_RESET,    GPIO_LOW);

    #if defined(CONFIG_SDMMC1_RK29) && !defined(CONFIG_SDMMC_RK29_OLD)
    
    rk29_mux_api_set(GPIO1C4_SDMMC1DATA1_NAME, GPIO1H_GPIO1C4);
    gpio_request(RK29_PIN1_PC4, "mmc1-data1");
    gpio_direction_output(RK29_PIN1_PC4,GPIO_LOW);//set mmc1-data1 to low.

    rk29_mux_api_set(GPIO1C5_SDMMC1DATA2_NAME, GPIO1H_GPIO1C5);
    gpio_request(RK29_PIN1_PC5, "mmc1-data2");
    gpio_direction_output(RK29_PIN1_PC5,GPIO_LOW);//set mmc1-data2 to low.

    rk29_mux_api_set(GPIO1C6_SDMMC1DATA3_NAME, GPIO1H_GPIO1C6);
    gpio_request(RK29_PIN1_PC6, "mmc1-data3");
    gpio_direction_output(RK29_PIN1_PC6,GPIO_LOW);//set mmc1-data3 to low.
    
    rk29_sdmmc_gpio_open(1, 0); //added by xbw at 2011-10-13
    #endif    
    pr_info("%s: init finished\n",__func__);

    return 0;
}

static int rk29sdk_wifi_power(int on)
{
	printk("%s: %d\n", __func__, on);
	if (on){
		gpio_set_value(GPIO_WIFI_POWER, GPIO_HIGH);

#if defined(CONFIG_SDMMC1_RK29) && !defined(CONFIG_SDMMC_RK29_OLD)	
		rk29_sdmmc_gpio_open(1, 1); //added by xbw at 2011-10-13
#endif

		gpio_set_value(GPIO_WIFI_RESET, GPIO_HIGH);
		mdelay(100);
		printk("wifi turn on power\n");
	}else{
		if (!rk29sdk_bt_power_state){
			gpio_set_value(GPIO_WIFI_POWER, GPIO_LOW);

#if defined(CONFIG_SDMMC1_RK29) && !defined(CONFIG_SDMMC_RK29_OLD)	
			rk29_sdmmc_gpio_open(1, 0); //added by xbw at 2011-10-13
#endif

			mdelay(100);
			pr_info("wifi shut off power\n");
		}else
		{
			printk("wifi shouldn't shut off power, bt is using it!\n");
		}
		gpio_set_value(GPIO_WIFI_RESET, GPIO_LOW);

	}

	rk29sdk_wifi_power_state = on;
	return 0;
}

static int rk29sdk_wifi_reset_state;
static int rk29sdk_wifi_reset(int on)
{
        printk("%s: %d\n", __func__, on);
        gpio_set_value(GPIO_WIFI_RESET, on);
        mdelay(100);
        rk29sdk_wifi_reset_state = on;
        return 0;
}

int rk29sdk_wifi_set_carddetect(int val)
{
        printk("%s:%d\n", __func__, val);
        rk29sdk_wifi_cd = val;
        if (wifi_status_cb){
                wifi_status_cb(val, wifi_status_cb_devid);
        }else {
                printk("%s, nobody to notify\n", __func__);
        }
        return 0;
}
EXPORT_SYMBOL(rk29sdk_wifi_set_carddetect);

static struct wifi_mem_prealloc wifi_mem_array[PREALLOC_WLAN_SEC_NUM] = {
        {NULL, (WLAN_SECTION_SIZE_0 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_1 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_2 + PREALLOC_WLAN_SECTION_HEADER)},
        {NULL, (WLAN_SECTION_SIZE_3 + PREALLOC_WLAN_SECTION_HEADER)}
};

static void *rk29sdk_mem_prealloc(int section, unsigned long size)
{
        if (section == PREALLOC_WLAN_SEC_NUM)
                return wlan_static_skb;

        if ((section < 0) || (section > PREALLOC_WLAN_SEC_NUM))
                return NULL;

        if (wifi_mem_array[section].size < size)
                return NULL;

        return wifi_mem_array[section].mem_ptr;
}

int __init rk29sdk_init_wifi_mem(void)
{
        int i;
        int j;

        for (i = 0 ; i < WLAN_SKB_BUF_NUM ; i++) {
                wlan_static_skb[i] = dev_alloc_skb(
                                ((i < (WLAN_SKB_BUF_NUM / 2)) ? 4096 : 8192));

                if (!wlan_static_skb[i])
                        goto err_skb_alloc;
        }

        for (i = 0 ; i < PREALLOC_WLAN_SEC_NUM ; i++) {
                wifi_mem_array[i].mem_ptr =
                                kmalloc(wifi_mem_array[i].size, GFP_KERNEL);

                if (!wifi_mem_array[i].mem_ptr)
                        goto err_mem_alloc;
        }
        return 0;

err_mem_alloc:
        pr_err("Failed to mem_alloc for WLAN\n");
        for (j = 0 ; j < i ; j++)
               kfree(wifi_mem_array[j].mem_ptr);

        i = WLAN_SKB_BUF_NUM;

err_skb_alloc:
        pr_err("Failed to skb_alloc for WLAN\n");
        for (j = 0 ; j < i ; j++)
                dev_kfree_skb(wlan_static_skb[j]);

        return -ENOMEM;
}

static struct wifi_platform_data rk29sdk_wifi_control = {
        .set_power = rk29sdk_wifi_power,
        .set_reset = rk29sdk_wifi_reset,
        .set_carddetect = rk29sdk_wifi_set_carddetect,
        .mem_prealloc   = rk29sdk_mem_prealloc,
};
static struct platform_device rk29sdk_wifi_device = {
        .name = "bcm4329_wlan",
        .id = 1,
        .dev = {
                .platform_data = &rk29sdk_wifi_control,
         },
};
#endif


/* bluetooth rfkill device */
static struct platform_device rk29sdk_rfkill = {
        .name = "rk29sdk_rfkill",
        .id = -1,
};


#ifdef CONFIG_VIVANTE
#define GPU_HIGH_CLOCK        552
#define GPU_LOW_CLOCK         (periph_pll_default / 1000000) /* same as general pll clock rate below */
static struct resource resources_gpu[] = {
    [0] = {
		.name 	= "gpu_irq",
        .start 	= IRQ_GPU,
        .end    = IRQ_GPU,
        .flags  = IORESOURCE_IRQ,
    },
    [1] = {
		.name   = "gpu_base",
        .start  = RK29_GPU_PHYS,
        .end    = RK29_GPU_PHYS + RK29_GPU_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    [2] = {
		.name   = "gpu_mem",
        .start  = PMEM_GPU_BASE,
        .end    = PMEM_GPU_BASE + PMEM_GPU_SIZE - 1,
        .flags  = IORESOURCE_MEM,
    },
    [3] = {
		.name 	= "gpu_clk",
        .start 	= GPU_LOW_CLOCK,
        .end    = GPU_HIGH_CLOCK,
        .flags  = IORESOURCE_IO,
    },
};
static struct platform_device rk29_device_gpu = {
    .name             = "galcore",
    .id               = 0,
    .num_resources    = ARRAY_SIZE(resources_gpu),
    .resource         = resources_gpu,
};
#endif

#ifdef CONFIG_KEYS_RK29
extern struct rk29_keys_platform_data rk29_keys_pdata;
static struct platform_device rk29_device_keys = {
	.name		= "rk29-keypad",
	.id		= -1,
	.dev		= {
		.platform_data	= &rk29_keys_pdata,
	},
};
#endif


static void __init rk29_board_iomux_init(void)
{
	#ifdef CONFIG_RK29_PWM_REGULATOR
	rk29_mux_api_set(REGULATOR_PWM_MUX_NAME,REGULATOR_PWM_MUX_MODE);
	#endif
}

static struct platform_device *devices[] __initdata = {

#ifdef CONFIG_RK29_WATCHDOG
	&rk29_device_wdt,
#endif

#ifdef CONFIG_UART1_RK29
	&rk29_device_uart1,
#endif
#ifdef CONFIG_UART0_RK29
	&rk29_device_uart0,
#endif
#ifdef CONFIG_UART2_RK29
	&rk29_device_uart2,
#endif
#ifdef CONFIG_UART3_RK29
	&rk29_device_uart3,
#endif

#ifdef CONFIG_RK29_PWM_REGULATOR
	&rk29_device_pwm_regulator,
#endif
#ifdef CONFIG_SPIM0_RK29
    &rk29xx_device_spi0m,
#endif
#ifdef CONFIG_SPIM1_RK29
    &rk29xx_device_spi1m,
#endif
#ifdef CONFIG_ADC_RK29
	&rk29_device_adc,
#endif
#ifdef CONFIG_I2C0_RK29
	&rk29_device_i2c0,
#endif
#ifdef CONFIG_I2C1_RK29
	&rk29_device_i2c1,
#endif
#ifdef CONFIG_I2C2_RK29
	&rk29_device_i2c2,
#endif
#ifdef CONFIG_I2C3_RK29
	&rk29_device_i2c3,
#endif

#ifdef CONFIG_SND_RK29_SOC_I2S_2CH
        &rk29_device_iis_2ch,
#endif
#ifdef CONFIG_SND_RK29_SOC_I2S_8CH
        &rk29_device_iis_8ch,
#endif

#ifdef CONFIG_KEYS_RK29
	&rk29_device_keys,
#endif
#ifdef CONFIG_KEYS_RK29_NEWTON
	&rk29_device_keys,
#endif
#ifdef CONFIG_SDMMC0_RK29
	&rk29_device_sdmmc0,
#endif
#ifdef CONFIG_SDMMC1_RK29
	&rk29_device_sdmmc1,
#endif

#ifdef CONFIG_MTD_NAND_RK29XX
	&rk29xx_device_nand,
#endif

#ifdef CONFIG_WIFI_CONTROL_FUNC
        &rk29sdk_wifi_device,
#endif

#ifdef CONFIG_BT
        &rk29sdk_rfkill,
#endif

#ifdef CONFIG_MTD_NAND_RK29
	&rk29_device_nand,
#endif

#ifdef CONFIG_FB_RK29
	&rk29_device_fb,
	&rk29_device_dma_cpy,
#endif
#ifdef CONFIG_BACKLIGHT_RK29_BL
	&rk29_device_backlight,
#endif
#ifdef CONFIG_BACKLIGHT_RK29_NEWTON_BL
	&rk29_device_backlight,
#endif
#ifdef CONFIG_RK29_VMAC
	&rk29_device_vmac,
#endif
#ifdef CONFIG_VIVANTE
	&rk29_device_gpu,
#endif
#ifdef CONFIG_VIDEO_RK29
 	&rk29_device_camera,      /* ddl@rock-chips.com : camera support  */
 	#if (CONFIG_SENSOR_IIC_ADDR_0 != 0x00)
 	&rk29_soc_camera_pdrv_0,
 	#endif
 	&rk29_soc_camera_pdrv_1,
 	&android_pmem_cam_device,
#endif
#if PMEM_SKYPE_SIZE > 0
	&android_pmem_skype_device,
#endif
#ifdef CONFIG_ION
	&rk29_ion_device,
#endif
	&android_pmem_device,
	&rk29_vpu_mem_device,
#ifdef CONFIG_USB20_OTG
	&rk29_device_usb20_otg,
#endif
#ifdef CONFIG_USB20_HOST
	&rk29_device_usb20_host,
#endif
#ifdef CONFIG_USB11_HOST
	&rk29_device_usb11_host,
#endif
#ifdef CONFIG_USB_ANDROID
	&android_usb_device,
	&usb_mass_storage_device,
#endif
#ifdef CONFIG_USB_ANDROID_RNDIS
    &rk29_device_rndis,
#endif
#ifdef CONFIG_RK29_IPP
	&rk29_device_ipp,
#endif
#ifdef CONFIG_VIDEO_RK29XX_VOUT
	&rk29_v4l2_output_devce,
#endif
#ifdef CONFIG_RK29_NEWTON
	&rk29_device_newton,
#endif
#ifdef CONFIG_RK_IRDA
    &irda_device,
#endif
#ifdef CONFIG_LEDS_GPIO_PLATFORM
	&rk29_device_gpio_leds,
#endif
#ifdef CONFIG_LEDS_NEWTON_PWM
	&rk29_device_pwm_leds,
#endif
#ifdef CONFIG_SND_RK29_SOC_CS42L52
	&rk29_cs42l52_device,
#endif
#ifdef CONFIG_BATTERY_RK2918
	&rk2918_device_battery,
#endif
};

/*****************************************************************************************
 * spi devices
 * author: cmc@rock-chips.com
 *****************************************************************************************/
static int rk29_vmac_register_set(void)
{
	//config rk29 vmac as rmii, 100MHz
	u32 value= readl(RK29_GRF_BASE + 0xbc);
	value = (value & 0xfff7ff) | (0x400);
	writel(value, RK29_GRF_BASE + 0xbc);
	return 0;
}

static int rk29_rmii_io_init(void)
{
	int err;

	//phy power gpio
	err = gpio_request(RK29_PIN6_PB0, "phy_power_en");
	if (err) {
		gpio_free(RK29_PIN6_PB0);
		printk("-------request RK29_PIN6_PB0 fail--------\n");
		return -1;
	}
	//phy power down
	gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
	gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);

	return 0;
}

static int rk29_rmii_io_deinit(void)
{
	//phy power down
	gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
	gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);
	//free
	gpio_free(RK29_PIN6_PB0);
	return 0;
}

static int rk29_rmii_power_control(int enable)
{
	if (enable) {
		//enable phy power
		gpio_direction_output(RK29_PIN6_PB0, GPIO_HIGH);
		gpio_set_value(RK29_PIN6_PB0, GPIO_HIGH);
	}
	else {
		gpio_direction_output(RK29_PIN6_PB0, GPIO_LOW);
		gpio_set_value(RK29_PIN6_PB0, GPIO_LOW);
	}
	return 0;
}

struct rk29_vmac_platform_data rk29_vmac_pdata = {
	.vmac_register_set = rk29_vmac_register_set,
	.rmii_io_init = rk29_rmii_io_init,
	.rmii_io_deinit = rk29_rmii_io_deinit,
	.rmii_power_control = rk29_rmii_power_control,
};

/*****************************************************************************************
 * spi devices
 * author: cmc@rock-chips.com
 *****************************************************************************************/
#define SPI_CHIPSELECT_NUM 2
static struct spi_cs_gpio rk29xx_spi0_cs_gpios[SPI_CHIPSELECT_NUM] = {
    {
		.name = "spi0 cs0",
		.cs_gpio = RK29_PIN2_PC1,
		.cs_iomux_name = GPIO2C1_SPI0CSN0_NAME,
		.cs_iomux_mode = GPIO2H_SPI0_CSN0,
	},
	{
		.name = "spi0 cs1",
		.cs_gpio = RK29_PIN1_PA4,
		.cs_iomux_name = GPIO1A4_EMMCWRITEPRT_SPI0CS1_NAME,//if no iomux,set it NULL
		.cs_iomux_mode = GPIO1L_SPI0_CSN1,
	}
};

static struct spi_cs_gpio rk29xx_spi1_cs_gpios[SPI_CHIPSELECT_NUM] = {
    {
		.name = "spi1 cs0",
		.cs_gpio = RK29_PIN2_PC5,
		.cs_iomux_name = GPIO2C5_SPI1CSN0_NAME,
		.cs_iomux_mode = GPIO2H_SPI1_CSN0,
	},
	{
		.name = "spi1 cs1",
		.cs_gpio = RK29_PIN1_PA3,
		.cs_iomux_name = GPIO1A3_EMMCDETECTN_SPI1CS1_NAME,//if no iomux,set it NULL
		.cs_iomux_mode = GPIO1L_SPI1_CSN1,
	}
};

static int spi_io_init(struct spi_cs_gpio *cs_gpios, int cs_num)
{
#if 1
	int i;
	if (cs_gpios) {
		for (i=0; i<cs_num; i++) {
			rk29_mux_api_set(cs_gpios[i].cs_iomux_name, cs_gpios[i].cs_iomux_mode);
		}
	}
#endif
	return 0;
}

static int spi_io_deinit(struct spi_cs_gpio *cs_gpios, int cs_num)
{
	return 0;
}

static int spi_io_fix_leakage_bug(void)
{
	return 0;
}

static int spi_io_resume_leakage_bug(void)
{
	return 0;
}

struct rk29xx_spi_platform_data rk29xx_spi0_platdata = {
	.num_chipselect = SPI_CHIPSELECT_NUM,
	.chipselect_gpios = rk29xx_spi0_cs_gpios,
	.io_init = spi_io_init,
	.io_deinit = spi_io_deinit,
	.io_fix_leakage_bug = spi_io_fix_leakage_bug,
	.io_resume_leakage_bug = spi_io_resume_leakage_bug,
};

struct rk29xx_spi_platform_data rk29xx_spi1_platdata = {
	.num_chipselect = SPI_CHIPSELECT_NUM,
	.chipselect_gpios = rk29xx_spi1_cs_gpios,
	.io_init = spi_io_init,
	.io_deinit = spi_io_deinit,
	.io_fix_leakage_bug = spi_io_fix_leakage_bug,
	.io_resume_leakage_bug = spi_io_resume_leakage_bug,
};

/*****************************************************************************************
 * xpt2046 touch panel
 * author: cmc@rock-chips.com
 *****************************************************************************************/
#define DEBOUNCE_REPTIME  3

#if defined(CONFIG_TOUCHSCREEN_XPT2046_320X480_SPI)
static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 0,
	.x_min			= 0,
	.x_max			= 320,
	.y_min			= 0,
	.y_max			= 480,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= TOUCH_INT_PIN,
	.penirq_recheck_delay_usecs = 1,
};
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_320X480_CBN_SPI)
static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 0,
	.x_min			= 0,
	.x_max			= 320,
	.y_min			= 0,
	.y_max			= 480,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= TOUCH_INT_PIN,
	.penirq_recheck_delay_usecs = 1,
};
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_SPI)
static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 1,
	.x_min			= 0,
	.x_max			= 800,
	.y_min			= 0,
	.y_max			= 480,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= TOUCH_INT_PIN,

	.penirq_recheck_delay_usecs = 1,
};
#elif defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
static struct xpt2046_platform_data xpt2046_info = {
	.model			= 2046,
	.keep_vref_on 	= 1,
	.swap_xy		= 1,
	.x_min			= 0,
	.x_max			= 800,
	.y_min			= 0,
	.y_max			= 480,
	.debounce_max		= 7,
	.debounce_rep		= DEBOUNCE_REPTIME,
	.debounce_tol		= 20,
	.gpio_pendown		= TOUCH_INT_PIN,

	.penirq_recheck_delay_usecs = 1,
};
#endif

static struct spi_board_info board_spi_devices[] = {
#if defined(CONFIG_TOUCHSCREEN_XPT2046_320X480_SPI) || defined(CONFIG_TOUCHSCREEN_XPT2046_320X480_CBN_SPI)\
    ||defined(CONFIG_TOUCHSCREEN_XPT2046_SPI) || defined(CONFIG_TOUCHSCREEN_XPT2046_CBN_SPI)
	{
		.modalias	= "xpt2046_ts",
		.chip_select	= 0,
		.max_speed_hz	= 125 * 1000 * 26,/* (max sample rate @ 3V) * (cmd + data + overhead) */
		.bus_num	= 0,
		.irq = TOUCH_INT_PIN,
		.platform_data = &xpt2046_info,
	},
#endif
};


static void __init rk29_gic_init_irq(void)
{
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 38))
	gic_init(0, 32, (void __iomem *)RK29_GICPERI_BASE, (void __iomem *)RK29_GICCPU_BASE);
#else
	gic_dist_init(0, (void __iomem *)RK29_GICPERI_BASE, 32);
	gic_cpu_init(0, (void __iomem *)RK29_GICCPU_BASE);
#endif
}

static void __init machine_rk29_init_irq(void)
{
	rk29_gic_init_irq();
	rk29_gpio_init();
}

#if 0
#ifdef CONFIG_REGULATOR_TPS65910
static struct cpufreq_frequency_table freq_table[] = { 
	{ .index = 1275000, .frequency =  408000 },
	{ .index = 1275000, .frequency =  912000 },
	{ .frequency = CPUFREQ_TABLE_END },
};
#else
static struct cpufreq_frequency_table freq_table[] = {
	//{ .index = 1175000, .frequency =  408000 },
	//{ .index = 1175000, .frequency =  816000 },
	{ .index = 1275000, .frequency = 1008000 },
	{ .frequency = CPUFREQ_TABLE_END },
};
#endif
#endif
static struct cpufreq_frequency_table freq_table[] = 
#ifdef CPU_FREQ_TABLE
	CPU_FREQ_TABLE;
#else
{
	{ .index = 1200000, .frequency =  204000 },
	{ .index = 1200000, .frequency =  300000 },
	{ .index = 1200000, .frequency =  408000 },
	{ .index = 1200000, .frequency =  600000 },
	{ .index = 1200000, .frequency =  816000 },
//	{ .index = 1250000, .frequency = 1008000 },
	{ .index = 1300000, .frequency = 1008000 },
	{ .index = 1300000, .frequency = 1104000 },
	{ .index = 1400000, .frequency = 1176000 },
	{ .index = 1400000, .frequency = 1200000 },
        { .index = 1450000, .frequency = 1300000 },
	{ .frequency = CPUFREQ_TABLE_END },
};
#endif

static int rk29_bcm4329_power_on(void)
{
#if defined(WIFI_EXT_POWER)
	int err;

#if defined(WIFI_EXT_POWER_MUX_NAME)
    rk29_mux_api_set(WIFI_EXT_POWER_MUX_NAME, WIFI_EXT_POWER_GPIO_MODE);
#endif
	//phy power gpio
	err = gpio_request(WIFI_EXT_POWER, "b23_pwr_on");
	if (err) {
		gpio_free(WIFI_EXT_POWER);
		printk("-------request WIFI_EXT_POWER fail--------\n");
		return -1;
	}
	//phy power down
	gpio_direction_output(WIFI_EXT_POWER, GPIO_HIGH);
	gpio_set_value(WIFI_EXT_POWER, GPIO_HIGH);

#endif
#if defined(GPIO_WIFI_POWER_MUX_NAME)
    rk29_mux_api_set(GPIO_WIFI_POWER_MUX_NAME, GPIO_WIFI_POWER_GPIO_MODE);
#endif
	
	return 0;
}

static void __init machine_rk29_board_init(void)
{
	int ret = 0;
	rk29_board_iomux_init();
	if(FB_DISPLAY_ON_PIN != INVALID_GPIO)
	{
	    ret = gpio_request(FB_DISPLAY_ON_PIN, NULL);
	    if(ret != 0)
	    {
	        gpio_free(FB_DISPLAY_ON_PIN);
	        printk(">>>>>> FB_DISPLAY_ON_PIN gpio_request err \n ");
	    }
		gpio_direction_output(FB_DISPLAY_ON_PIN, 0);
    	gpio_set_value(FB_DISPLAY_ON_PIN, !FB_DISPLAY_ON_VALUE);  
	}   
	
#if defined(LCD_RST_PIN)
	if (LCD_RST_PIN != INVALID_GPIO) 
	{
		ret = gpio_request(LCD_RST_PIN, NULL);
		if(ret != 0)
		{
			gpio_free(LCD_RST_PIN);
			printk(">>>>>> LCD_RST_PIN gpio_request err \n ");
		}
	}
	gpio_direction_output(LCD_RST_PIN, 0);
	gpio_pull_updown(LCD_RST_PIN, 0);
	gpio_set_value(LCD_RST_PIN, 0);
#endif
	
#ifdef BL_EN_PIN 
    gpio_direction_output(BL_EN_PIN, !BL_EN_VALUE);
    gpio_set_value(BL_EN_PIN, !BL_EN_VALUE);
#endif

	rk29_mux_api_set(PWM_MUX_NAME, PWM_MUX_MODE_GPIO);
	if (gpio_request(PWM_GPIO, NULL)) {
		printk("func %s, line %d: request gpio fail\n", __FUNCTION__, __LINE__);
		gpio_free(PWM_GPIO);;
	}
	gpio_direction_output(PWM_GPIO, !PWM_EFFECT_VALUE);
	gpio_set_value(PWM_GPIO, !PWM_EFFECT_VALUE);
	
	board_power_init();
	board_update_cpufreq_table(freq_table);
	rk29_bcm4329_power_on();

		platform_add_devices(devices, ARRAY_SIZE(devices));
#ifdef CONFIG_I2C0_RK29
	i2c_register_board_info(default_i2c0_data.bus_num, board_i2c0_devices,
			ARRAY_SIZE(board_i2c0_devices));
#endif
#ifdef CONFIG_I2C1_RK29
	i2c_register_board_info(default_i2c1_data.bus_num, board_i2c1_devices,
			ARRAY_SIZE(board_i2c1_devices));
#endif
#ifdef CONFIG_I2C2_RK29
	i2c_register_board_info(default_i2c2_data.bus_num, board_i2c2_devices,
			ARRAY_SIZE(board_i2c2_devices));
#endif
#ifdef CONFIG_I2C3_RK29
	i2c_register_board_info(default_i2c3_data.bus_num, board_i2c3_devices,
			ARRAY_SIZE(board_i2c3_devices));
#endif

	spi_register_board_info(board_spi_devices, ARRAY_SIZE(board_spi_devices));
        
#ifdef CONFIG_WIFI_CONTROL_FUNC
	rk29sdk_wifi_bt_gpio_control_init();
	rk29sdk_init_wifi_mem();
#endif

	board_usb_detect_init(GPIO_USB_INT);
#if defined(CONFIG_RK_IRDA) || defined(CONFIG_BU92747GUW_CIR)
	smc0_init(NULL);
	bu92747guw_io_init();
#endif

}

static void __init machine_rk29_fixup(struct machine_desc *desc, struct tag *tags,
					char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = RK29_SDRAM_PHYS;
	mi->bank[0].size = LINUX_SIZE;
#if SDRAM_SIZE > SZ_512M
	mi->nr_banks = 2;
	mi->bank[1].start = RK29_SDRAM_PHYS + SZ_512M;
	mi->bank[1].size = SDRAM_SIZE - SZ_512M;
#endif
}

static void __init machine_rk29_mapio(void)
{
	rk29_map_common_io();
	rk29_setup_early_printk();
	rk29_sram_init();
	rk29_clock_init(periph_pll_default);
	rk29_iomux_init();
	ddr_init(DDR_TYPE, DDR_FREQ);
}

MACHINE_START(RK29, "RK29board")
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 37))
	/* UART for LL DEBUG */
	.phys_io	= RK29_UART1_PHYS & 0xfff00000,
	.io_pg_offst	= ((RK29_UART1_BASE) >> 18) & 0xfffc,
#endif
	.boot_params	= RK29_SDRAM_PHYS + 0x88000,
	.fixup		= machine_rk29_fixup,
	.map_io		= machine_rk29_mapio,
	.init_irq	= machine_rk29_init_irq,
	.init_machine	= machine_rk29_board_init,
	.timer		= &rk29_timer,
MACHINE_END
