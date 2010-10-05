/**********************************************************************
* This file is constructed by Roberto Gordo Saez
* <roberto.gordo@gmail.com>, based on U-Boot and Qi source for SmartQ.
*
* board/samsung/smdk6410/cmd_hhtech.c
* board/samsung/smdk6410/misc.c
*
*   王刚(WangGang)  <wanggang@hhcn.com>
*   CopyRight (c)  2009  HHTech
*     www.hhcn.com, www.hhcn.org
*     All rights reserved.
*
*   This file is free software;
*     you are free to modify and/or redistribute it
*     under the terms of the GNU General Public Licence (GPL).
*
*   No warranty, no liability, use this at your own risk!
*
* Qi source is (C) Copyright 2008 Openmoko, Inc.
* Author: Andy Green <andy@openmoko.org>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License as
* published by the Free Software Foundation; either version 2 of
* the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
**********************************************************************/

#include <qi.h>
#include <serial-s3c64xx.h>
#include <usbd-otg-hs.h>
#include <common.h>

#include "fbwrite.h"

#define SMDK6410_DEBUG_UART 0

#define CHANNEL ((*(volatile u32*)0x0C003FEC == 0x7c300000) ? 1 : 0)

#define CopyMMCtoMem(a,b,c,e) (((u8(*)(int, unsigned int, unsigned short, unsigned int*, u8)) \
	(*((unsigned int*)0x0C004008)))(CHANNEL,a,b,c,e))

/* LEDs are on GPN8 and GPN9 */
#if SMARTQ == 5
#define GPIO_LED_0 (('N'-'A')*16 + 8)
#define GPIO_LED_1 (('N'-'A')*16 + 9)
#define MACH_ID    2534
#define MACH_NAME  "SmartQ 5"
#elif SMARTQ == 7
#define GPIO_LED_0 (('N'-'A')*16 + 9)
#define GPIO_LED_1 (('N'-'A')*16 + 8)
#define MACH_ID    2479
#define MACH_NAME  "SmartQ 7"
#endif

/* high poweroff, low start */
#define GPIO_POWEROFF (('K'-'A')*16 + 15)

/* LCD backlight on or off */
#define GPIO_LCD_BACKLIGHT (('F'-'A')*16 + 15)

/* DAT offset for groups */
static char gdat[]= {4, 4, 4, 4, 4, 4, 4, 8, 4, 4, 8, 8, 4, 4, 4, 4, 4};
/* CONF MASK BITS for groups */
static char gcsz[]= {4, 4, 4, 4, 4, 2, 4, 4, 2, 2, 4, 4, 4, 2, 2, 2, 2};
static unsigned short gbase[] =
	{0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x120, /* A ... J */
	 0x800, 0x810, 0x820, 0x830, /* K ... N */
	 0x140, 0x160, 0x180, }; /* O P Q */

static unsigned long gpio_base = 0x7f008000;

static struct fbinfo *fbi = 0;

#define readl(a) (*(volatile unsigned int *)(a))
#define writel(v,a) (*(volatile unsigned int *)(a) = (v))

static int gpio_direction_set(int gpio, int out)
{
	int group = gpio >> 4, sub = gpio % 16;
	unsigned char mask = 0x03;
	unsigned int regconf = 0, val, regpud = 0;

	regconf = gpio_base + gbase[group];
	if(gcsz[group] == 4) {
		mask = 0x0F;
		if(sub >= 8) regconf += 4;
	}

	val = readl(regconf);
	if(4 == gcsz[group] && sub >= 8)
		val &= ~((mask) << (gcsz[group] * (sub-8)));
	else
		val &= ~((mask) << (gcsz[group] * sub));

	if(out) {
		if(4 == gcsz[group] && sub >= 8)
			val |= ((0x01) << (gcsz[group] * (sub-8)));
		else
			val |= ((0x01) << (gcsz[group] * sub));
	}
	writel(val, regconf);

	regpud = gpio_base + gbase[group] + gdat[sub] + 4;
	if(4 == gcsz[group]) regpud += 4;
	val = readl(regpud);
	val &= ~(0x3<<(sub*2));
	writel(val, regpud);

	return 0;
}

/* ret : high is 1, low is 0 */
int gpio_set_value(int gpio, int dat)
{
	int group = gpio >> 4, sub = gpio % 16;
	unsigned int regdat = 0, val;

	regdat = gpio_base + gbase[group] + gdat[sub];
	if(4 == gcsz[group]) regdat += 4;
	val = readl(regdat);

	if(dat) val |= (1<<sub);
	else    val &= ~(1<<sub);

	writel(val, regdat);

	return 0;
}

static void gpio_direction_output(int gpio, int dat)
{
	gpio_direction_set(gpio, 1);
	gpio_set_value(gpio, dat);
}

void led_set(int flag)
{
	flag = flag % 4;

	if(flag & 0x01) gpio_direction_output(GPIO_LED_0, 0);
	else            gpio_direction_output(GPIO_LED_0, 1);

	if(flag & 0x02) gpio_direction_output(GPIO_LED_1, 0);
	else            gpio_direction_output(GPIO_LED_1, 1);
}

void led_blink(int l1, int l2)
{
	int i = 5;
	while(i--) {
		led_set(l1);
		udelay(1000000);
		led_set(l2);
		udelay(1000000);
	}
}

void poweroff(void)
{
	led_set(0);
	gpio_direction_output(GPIO_POWEROFF, 1);
}

void set_lcd_backlight(int flag)
{
	gpio_direction_output(GPIO_LCD_BACKLIGHT, flag ? 1 : 0);
}

int battery_probe(void)
{
#if 0
	S3C2410_ADC * const adc = S3C2410_GetBase_ADC();
	S3C64XX_GPIO * const gpio = S3C64XX_GetBase_GPIO();
	unsigned int i, val, data0, powerflag;

	if(get_dc_status()) {
		/* dc on, don't need to check battery! */
		return 0;
	}

	adc->ADCDLY = 50000 & 0xffff;
	val = S3C6410_ADCCON_RESSEL_12BIT | S3C2410_ADCCON_PRSCEN | S3C2410_ADCCON_PRSCVL(49);
	adc->ADCCON = val | S3C2410_ADCCON_SELMUX(0);   // using AN0
	udelay(10);

	for(i=0; i<5; i++) {
		adc->ADCCON |= S3C2410_ADCCON_ENABLE_START; // start ADC conversion
		do {
			val = adc->ADCCON;
		} while(!(val & S3C2410_ADCCON_ECFLG));	// wait for the end of ADC conversion

		// read data
		// ref voltage:2.4V, battery max:4.2V, battery low:3.45V
		data0 = adc->ADCDAT0 & S3C_ADCDAT0_XPDATA_MASK_12BIT;
		if(data0 < 2192)	// (1525 * 345) / 240 = 2192
			powerflag = 1;
		else
			powerflag = 0;
	}
	if(powerflag) {
		puts("The battery is too low.\n\n");
		led_blink(2, 0);
		led_blink(2, 0);
		poweroff();
	}
#endif
	return 0;
}

static int is_this_board_smartq(void)
{
	/* FIXME: find something SmartQ specific */
	set_lcd_backlight(1);
	led_set(3);
	return 1;
}

static void putc_smdk6410(char c)
{
	//if (fbi!=0)
	//	fb_putc(fbi, c);
	serial_putc_s3c64xx(SMDK6410_DEBUG_UART, c);
}

int sd_card_block_read_smartq(unsigned char *buf, unsigned long start512, int blocks512)
{
	int retval;

	if(blocks512 > 1)
		retval = (int)CopyMMCtoMem(start512, blocks512, (void*)buf, 0);
	else if(blocks512 == 1) {
		/* CopyMMCtoMem fails to read 1 block. This is a workaround that reads 2
		blocks to a temporary buffer and copy only the first one. */
		u8 tmpbuf[512 * 2];
		retval = (int)CopyMMCtoMem(start512, 2, (void*)tmpbuf, 0);
		if(retval)
			memcpy(buf, tmpbuf, 512);
	} else
		return 0;
	if(!retval)
		return -1;
	return blocks512;
}

static int usb_inited=0;
static int usb_init(void)
{
	fbi = fb_init();

	usb_inited = s3c_usbctl_init();

	//fb_clear(fbi);
	fb_printf(fbi,"FB: %x ",(uint32)fbi->fb);
	led_set(0);

	return usb_inited;
}

static int usb_read(unsigned char * buf, unsigned long start512, int blocks512)
{
	(void)buf;
	(void)start512;
	(void)blocks512;
	void (*usb_boot)(void);

	if(usb_inited != 0) //0:ok
		return usb_inited;

	s3c_receive_done = 0;
	s3c_usbd_dn_cnt = 0;

	fb_printf(fbi, "Waiting for DN to transmit data\n");

	while (1) {
		if (S3C_USBD_DETECT_IRQ()) {
			s3c_udc_int_hndlr();
			S3C_USBD_CLEAR_IRQ();
		}

		if (s3c_receive_done)
			break;

#if 0
		if (serial_tstc()) {
			serial_getc();
			break;
		}
#endif
	}

	/* when operation is done, usbd must be stopped */
	s3c_usb_stop();

	if(s3c_usbd_dn_cnt) {
		usb_boot = (void*)s3c_usbd_dn_addr;
		fb_printf(fbi, "Booting...\n");
		usb_boot();
	}

	return -1;
}

static const struct board_variant board_variants[] = {
	[0] = {
		.name = MACH_NAME,
		.machine_revision = 0,
	},
};

static const struct board_variant const * get_board_variant_smartq(void)
{
	return &board_variants[0];
}

const struct board_api board_api_smartq = {
	.name = "SmartQ",
	.linux_machine_id = MACH_ID,
	.linux_mem_start = 0x50000000,
	.linux_mem_size = (128 * 1024 * 1024),
	.linux_tag_placement = 0x50000000 + 0x100,
	.get_board_variant = get_board_variant_smartq,
	.is_this_board = is_this_board_smartq,
	.putc = putc_smdk6410,
	.commandline_board = "loglevel=6 rootwait s3cfb.backlight=80 ", //backlight setting for future kernel
	.commandline_board_debug = "console=ttySAC0,115200n8 ignore_loglevel ",
	.noboot = "boot/noboot-SMDK6410",
	.append = "boot/append-SMDK6410",
	.kernel_source = {
		[0] = {
			.name = "USB boot",
			.block_init = usb_init,
			.block_read = usb_read,
			.filesystem = FS_RAW,
		},
		[1] = {
			.name = "SD Card rootfs P1",
			.block_read = sd_card_block_read_smartq,
			.filesystem = FS_EXT2,
			.partition_index = 1,
			.filepath = "boot/linux-SMDK6410.bin",
			.commandline_append = "root=/dev/mmcblk1p1 "
		},
		[2] = {
			.name = "SD Card rootfs P2",
			.block_read = sd_card_block_read_smartq,
			.filesystem = FS_EXT2,
			.partition_index = 2,
			.filepath = "boot/linux-SMDK6410.bin",
			.commandline_append = "root=/dev/mmcblk1p2 "
		},
		[3] = {
			.name = "SD Card rootfs P3",
			.block_read = sd_card_block_read_smartq,
			.filesystem = FS_EXT2,
			.partition_index = 3,
			.filepath = "boot/linux-SMDK6410.bin",
			.commandline_append = "root=/dev/mmcblk1p3 "
		},
		[4] = {
			.name = "SD Card rootfs P4",
			.block_read = sd_card_block_read_smartq,
			.filesystem = FS_EXT2,
			.partition_index = 4,
			.filepath = "boot/linux-SMDK6410.bin",
			.commandline_append = "root=/dev/mmcblk1p4 "
		},
	},
};
