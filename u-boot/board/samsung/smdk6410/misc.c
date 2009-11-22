/********************************************************************
* $ID: gpio.c           ËÄ, 05  2ÔÂ 2009 09:41:25 +0800  whg        *
*                                                                   *
* Description:                                                      *
*                                                                   *
* Maintainer:  Íõ¸Õ(WangGang)  <wanggang@hhcn.com>                  *
*                                                                   *
* CopyRight (c)  2009  HHTech                                       *
*   www.hhcn.com, www.hhcn.org                                      *
*   All rights reserved.                                            *
*                                                                   *
* This file is free software;                                       *
*   you are free to modify and/or redistribute it                   *
*   under the terms of the GNU General Public Licence (GPL).        *
*                                                                   *
* Last modified: Fri, 06 Mar 2009 16:27:00 +0800       by root #
*                                                                   *
* No warranty, no liability, use this at your own risk!             *
********************************************************************/
#include <common.h>

#ifdef CONFIG_HHTECH_MINIPMP

#include <regs.h>
#include <asm/io.h>
#include "hhtech.h"


KeyInfo gkeys[] = {
    {KEY_POWER,   ('L'-'A')*16 + 14, 0},
    {KEY_UP,      ('N'-'A')*16 + 15, 0},
    {KEY_DOWN,      ('N'-'A')*16 + 12, 0},
    {KEY_LEFT,      ('N'-'A')*16 + 2, 0},

    {0xFFFF, 0, 0},
};

//                    A  B  C  D  E  F  G H  I  J  K   L  M  N  O  P  Q
//r   gpio_goups[] = {8, 7, 8, 5, 5,16, 7,10,16,12,16,15, 6,16,16,15, 9};
// DAT offset for groups
static char  gdat[]= {4, 4, 4, 4, 4, 4, 4, 8, 4, 4, 8, 8, 4, 4, 4, 4, 4};
// CONF MASK BITS for groups
static char  gcsz[]= {4, 4, 4, 4, 4, 2, 4, 4, 2, 2, 4, 4, 4, 2, 2, 2, 2};
static unsigned short gbase[] = 
    {0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x120,  // A...J
     0x800, 0x810, 0x820, 0x830, // K ... N
     0x140, 0x160, 0x180, };     // O P Q
static unsigned long  gpio_base = ELFIN_GPIO_BASE;

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

    //printf("SET: gpio=%d,regconf=0x%x\n", gpio, regconf);
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

// ret : high is 1, low is 0
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

// ret : high is 1, low is 0
int gpio_get_value(int gpio)
{
    int group = gpio >> 4, sub = gpio % 16;
    unsigned int regdat = 0, val;

    regdat = gpio_base + gbase[group] + gdat[sub];
    if(4 == gcsz[group]) regdat += 4;
    val = readl(regdat);
    
//    printf("READ:gpio=%d,regdat=0x%x\n", gpio, regdat);
    if(val & (1<<sub)) return 1;

    return 0;
}

int gpio_direction_input(int gpio)
{
    gpio_direction_set(gpio, 0);
    return 0;
}

int gpio_direction_output(int gpio, int dat)
{
    gpio_direction_set(gpio, 1);
    gpio_set_value(gpio, dat);
    return 0;
}


int rtc_delay(int sec)
{

    return 0;
}


#endif
/********************* End Of File: gpio.c *********************/
