/*
 * rk29_m732hc.h
 *
 * Overview:  
 *
 * Copyright (c) 2011, YiFang Digital
 *
 * Version:  1.0
 * Created:  12/28/2011 11:20:11 AM
 * Author:  zqqu <zqqu@yifangdigital.com>
 * Company:  YiFang Digital
 * History:
 *
 * 
 */


#ifndef __RK29_M732HC_H
#define __RK29_M732HC_H
#define NO_IOMUX_PINNAME  NULL
#define NO_IO_MUX_MODE		NULL

/***************************************************
 *
 *                      CPU FREQ
 *
 **************************************************/
#define CPU_FREQ_TABLE {\
	{ .index = 1150000, .frequency =  408000 },\
	{ .index = 1200000, .frequency =  816000 },\
	{ .index = 1250000, .frequency = 1008000 },\
	{ .frequency = CPUFREQ_TABLE_END },\
}

/***************************************************
 *
 *                      KEY
 *
 **************************************************/
#define GPIO_VOLUMEUP_KEY			RK29_PIN6_PA1
#define GPIO_VOLUMEDOWN_KEY			RK29_PIN6_PA2
#define GPIO_POWER_KEY				RK29_PIN6_PA7

#define PRESS_LEV_LOW			1
#define PRESS_LEV_HIGH			0

#define KEYS_MAP	{\
	{	\
		.desc	= "play",	\
		.code	= KEY_POWER,	\
		.gpio	= GPIO_POWER_KEY,	\
		.active_low = PRESS_LEV_LOW,	\
		.wakeup	= 1,	\
	},	\
	{ \
		.desc	= "vol+",	\
		.code	= KEY_VOLUMEUP,	\
		.gpio	= GPIO_VOLUMEUP_KEY,	\
		.active_low = PRESS_LEV_LOW,	\
	},	\
	{	\
		.desc	= "vol-",	\
		.code	= KEY_VOLUMEDOWN,	\
		.gpio	= GPIO_VOLUMEDOWN_KEY, \
		.active_low = PRESS_LEV_LOW,	\
	},	\
}

/**********************************************************************************************
 *
 *										LCD
 *
 *********************************************************************************************/
#define LCD_TXD_PIN          INVALID_GPIO
#define LCD_CLK_PIN          INVALID_GPIO
#define LCD_CS_PIN           INVALID_GPIO

#define FB_ID                       0
#define FB_DISPLAY_ON_PIN           RK29_PIN6_PD1
#define FB_LCD_STANDBY_PIN          RK29_PIN6_PD0
#define FB_LCD_CABC_EN_PIN          INVALID_GPIO
#define FB_MCU_FMK_PIN              INVALID_GPIO
//#define LCD_DISP_ON_PIN

#define FB_DISPLAY_ON_VALUE         GPIO_HIGH
#define FB_LCD_STANDBY_VALUE        GPIO_HIGH


#define OUT_CLK			 32000000
#define LCDC_ACLK       400000000   

/* Base */
#define OUT_TYPE		SCREEN_RGB
#define OUT_FACE		OUT_P888

/* Timing */
#define H_PW			48
#define H_BP			40
#define H_VD			800
#define H_FP			62

#define V_PW			3
#define V_BP			29
#define V_VD			480
#define V_FP			48

/* Other */
#define DCLK_POL		0
#define SWAP_RB			0

#define LCD_WIDTH       154
#define LCD_HEIGHT      87

/**********************************************************************************************
 *
 *										BACKLIGHT
 *
 *********************************************************************************************/
#define PWM_ID				0
#define PWM_MUX_NAME		GPIO1B5_PWM0_NAME
#define PWM_MUX_MODE		GPIO1L_PWM0
#define PWM_MUX_MODE_GPIO	GPIO1L_GPIO1B5
#define PWM_GPIO			RK29_PIN1_PB5
#define PWM_EFFECT_VALUE	0

#ifdef LCD_DISP_ON_PIN
#define BL_EN_PIN			RK29_PIN6_PD1
#define BL_EN_VALUE			GPIO_HIGH
#endif

/**the value of MIN_BACKLIGHT_SCALE must be between 0~10*/
#define MIN_BACKLIGHT_SCALE	8

/**********************************************************************************************
 *
 *							PWM VOLTAGE REGULATOR
 *
 *********************************************************************************************/
#define REGULATOR_PWM_ID					2
#define REGULATOR_PWM_MUX_NAME      		GPIO2A3_SDMMC0WRITEPRT_PWM2_NAME
#define REGULATOR_PWM_MUX_MODE				GPIO2L_PWM2
#define REGULATOR_PWM_MUX_MODE_GPIO			GPIO2L_GPIO2A3
#define REGULATOR_PWM_GPIO					RK29_PIN2_PA3



/**********************************************************************************************
 *
 *										TOUCH PANEL
 *
 *********************************************************************************************/
#define TOUCH_POWER_PIN			RK29_PIN6_PB0
#define TOUCH_RESET_PIN			RK29_PIN6_PC3
#define TOUCH_INT_PIN			RK29_PIN0_PA2
#define TOUCH_USE_I2C2			1
#define TOUCH_KEY_LED			RK29_PIN6_PA6
//#define TOUCHKEY_ON_SCREEN

/**********************************************************************************************
 *
 *										RTC
 *
 *********************************************************************************************/
#define GPIO_RTC_INT			 RK29_PIN0_PA1

/***************************************************
 *
 *                      AUDIO
 *
 **************************************************/
#define GPIO_SPK_CON			 RK29_PIN6_PB6
#define RT5631_DEF_VOL					0xc2
#define RT5631_DEF_VOL_SPK				0xc4
#define DEF_HP_EQ				NORMAL

/**********************************************************************************************
 *
 *										GSENSOR
 *
 *********************************************************************************************/
#define MMA8452_INT_PIN   RK29_PIN0_PA3


/**********************************************************************************************
 *
 *										USB
 *
 *********************************************************************************************/
#define GPIO_USB_INT			 RK29_PIN0_PA0
#define MASS_STORAGE_NAME "M712HC"
#define MASS_STORAGE_PRODUCT ""
#define USB_PRODUCT_ID			0x2910
#define ADB_PRODUCT_ID			0x0c02
#define VENDOR_ID				0x0bb4
#define ADB_PRODUCT_NAME		"rk2918"
#define ADB_MANUFACTURE_NAME	"RockChip"

/**********************************************************************************************
 *
 *										HDMI
 *
 *********************************************************************************************/
#define ANX7150_ATTACHED_BUS1
#define GPIO_HDMI_DET			 RK29_PIN1_PD7
#define	GPIO_ANX7150_RST		RK29_PIN2_PC7
#define ANX7150_RST_MUX_NAME	GPIO2C7_SPI1RXD_NAME
#define ANX7150_RST_MUX_MODE	GPIO2H_GPIO2C7


/**********************************************************************************************
 *
 *										SDMMC
 *
 *********************************************************************************************/
#define SDMMC_POWER_PIN		RK29_PIN5_PD5
#define SDMMC_DET_PIN		RK29_PIN2_PA2


/**********************************************************************************************
 *
 *									WIFI/BT	
 *
 *********************************************************************************************/
#define GPIO_WIFI_POWER       RK29_PIN6_PB3

/**********************************************************************************************
 *
 *									BATTERY
 *
 *********************************************************************************************/
#define DC_DET_EFFECTIVE		1
#define CHG_OK_EFFECTIVE		1
#define GPIO_DC_DET			RK29_PIN4_PA1
#define GPIO_CHG_OK			RK29_PIN4_PA3
#define ADC_CLI_VALUE		20
#define ADC_ADD_VALUE		1
#define CHARGE_FULL_GATE 		4150


//This parameter is for new battery driver//
#define	TIMER_MS_COUNTS		            50	//timers length(ms)

#define	SLOPE_SECOND_COUNTS	            15	//time interval(s) for computing voltage slope
#define	DISCHARGE_MIN_SECOND	        60	//minimum time interval for discharging 1% battery
#define	CHARGE_MIN_SECOND	            100	//minimum time interval for charging 1% battery
#define	CHARGE_MID_SECOND	            160	//time interval for charging 1% battery when battery capacity over 80%
#define	CHARGE_MAX_SECOND	            180	//max time interval for charging 1% battery
#define CHARGE_FULL_DELAY_TIMES         10  //delay time when charging FULL
#define USBCHARGE_IDENTIFY_TIMES        5   //time for identifying USB and Charge
#define STABLE_SECOND					8  
#define SHUTDOWN_SECOND					20
#define SPEEDLOSE_SECOND                120 //play game rapid down

#define	NUM_VOLTAGE_SAMPLE	            ((SLOPE_SECOND_COUNTS * 1000) / TIMER_MS_COUNTS)	//samling numbers
#define	NUM_DISCHARGE_MIN_SAMPLE	    ((DISCHARGE_MIN_SECOND * 1000) / TIMER_MS_COUNTS)	
#define	NUM_CHARGE_MIN_SAMPLE	        ((CHARGE_MIN_SECOND * 1000) / TIMER_MS_COUNTS)	    
#define	NUM_CHARGE_MID_SAMPLE	        ((CHARGE_MID_SECOND * 1000) / TIMER_MS_COUNTS)	    
#define	NUM_CHARGE_MAX_SAMPLE	        ((CHARGE_MAX_SECOND * 1000) / TIMER_MS_COUNTS)	    
#define NUM_CHARGE_FULL_DELAY_TIMES     ((CHARGE_FULL_DELAY_TIMES * 1000) / TIMER_MS_COUNTS)	
#define NUM_USBCHARGE_IDENTIFY_TIMES    ((USBCHARGE_IDENTIFY_TIMES * 1000) / TIMER_MS_COUNTS)
#define NUM_STABLE_SAMPLE				((STABLE_SECOND * 1000) / TIMER_MS_COUNTS)
#define NUM_SHUTD0WN_SAMPLE             ((SHUTDOWN_SECOND * 1000) / TIMER_MS_COUNTS)
#define NUM_SPEEDLOSE_SAMPLE  			((SPEEDLOSE_SECOND * 1000) / TIMER_MS_COUNTS)

#define BAT_2V5_VALUE	        2500
#define BATT_MAX_VOL_VALUE	    4190	//voltage of FULL battery
#define	BATT_ZERO_VOL_VALUE     3500	//voltage when poweroff
#define BATT_NOMAL_VOL_VALUE    3800
#define SHUTDOWNVOLTAGE			3400

//define  divider resistors for ADC sampling, units as K
#define BAT_PULL_UP_R           549
#define BAT_PULL_DOWN_R         200


/***************************************************
 *
 *                  CAMERA SENSOR
 *
 **************************************************/
#define CONFIG_SENSOR_0 RK29_CAM_SENSOR_OV5642			/* back camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_0 	    0x78
#define CONFIG_SENSOR_IIC_ADAPTER_ID_0    1
#define CONFIG_SENSOR_ORIENTATION_0       90
#define CONFIG_SENSOR_POWER_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_0       RK29_PIN6_PB7
#define CONFIG_SENSOR_FALSH_PIN_0         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_0 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_0 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_0 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_0 RK29_CAM_FLASHACTIVE_L
#define OV5642_BP_REGULATOR	0x0b	

#define CONFIG_SENSOR_QCIF_FPS_FIXED_0      15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_0      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_0       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_0       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_0      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_0      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_0      30000


//---------------------------------------------------------------------------
#if defined CONFIG_SOC_CAMERA_GT2005
#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_GT2005 /* front camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_1				0x78
#define GT2005_FLIP 1
#elif defined CONFIG_SOC_CAMERA_NT99250
#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_NT99250 /* front camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_1				0x6c
#if defined CONFIG_OLD_NT99250
#define NT99250_X_OFFSET	2
#define NT99250_X_OFFSET	2
#define VAL_306D	0x00
#endif 	
#define NT99250_MIRROR	0
#define NT99250_FLIP		0
#elif defined CONFIG_SOC_CAMERA_FCAM
#define CONFIG_SENSOR_1 RK29_CAM_SENSOR_FCAM /* front camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_1				0xff
#elif defined CONFIG_SOC_CAMERA_GC0307
#define CONFIG_SENSOR_1						RK29_CAM_SENSOR_GC0307	/* front camera sensor */
#define CONFIG_SENSOR_IIC_ADDR_1			0x42
#endif

#define GC0308_MIRROR	1
#define GC0308_FLIP		1

#define CONFIG_SENSOR_IIC_ADAPTER_ID_1    1
#define CONFIG_SENSOR_ORIENTATION_1       270
#define CONFIG_SENSOR_POWER_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_RESET_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERDN_PIN_1       RK29_PIN5_PD7
#define CONFIG_SENSOR_FALSH_PIN_1         INVALID_GPIO
#define CONFIG_SENSOR_POWERACTIVE_LEVEL_1 RK29_CAM_POWERACTIVE_L
#define CONFIG_SENSOR_RESETACTIVE_LEVEL_1 RK29_CAM_RESETACTIVE_L
#define CONFIG_SENSOR_POWERDNACTIVE_LEVEL_1 RK29_CAM_POWERDNACTIVE_H
#define CONFIG_SENSOR_FLASHACTIVE_LEVEL_1 RK29_CAM_FLASHACTIVE_L
#define CONFIG_SENSOR_QCIF_FPS_FIXED_1      15000
#define CONFIG_SENSOR_QVGA_FPS_FIXED_1      15000
#define CONFIG_SENSOR_CIF_FPS_FIXED_1       15000
#define CONFIG_SENSOR_VGA_FPS_FIXED_1       15000
#define CONFIG_SENSOR_480P_FPS_FIXED_1      15000
#define CONFIG_SENSOR_SVGA_FPS_FIXED_1      15000
#define CONFIG_SENSOR_720P_FPS_FIXED_1      30000

#endif

