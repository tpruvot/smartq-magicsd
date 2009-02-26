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
* Last modified:                                                    *
*                                                                   *
* No warranty, no liability, use this at your own risk!             *
********************************************************************/
#include <common.h>

#ifdef CONFIG_HHTECH_MINIPMP

#include <regs.h>
#include <asm/io.h>
#include "hhtech.h"


KeyInfo gkeys[] = {
//    {KEY_POWER,   ('N'-'A')<<4 + 8, 1},

    {0xFFFF, 0, 0},
};

//                             A  B  C  D  E  F  G H  I  J  K   L  M  N  O  P  Q
//static char  gpio_goups[] = {8, 7, 8, 5, 5,16, 7,10,16,12,16,15, 6,16,16,15, 9};
static char  gpio_dat_offs[]= {4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 8, 8, 4, 4, 4, 4, 4};
static unsigned short gpio_group_offs[] = 
    {0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0, 0x100, 0x120,  // A...J
     0x880, 0x810, 0x820, 0x830, // K ... N
     0x140, 0x160, 0x180, };     // O P Q
static unsigned long  gpio_base = ELFIN_GPIO_BASE;

#define  REG_BASE()      (gpio_base + gpio_group_offs[group] + ((sub > 8) ? 4: 0))
#define  CLEAR_CON(v, n) ((v) & ~(0xF<<((((n)>=8) ? ((n)-8): (n))*4)))

#define  REGIN           CLEAR_CON
#define  REGOUT(v,n)     ((CLEAR_CON((v),(n))) | (0x01<<((((n)>=8) ? ((n)-8): (n))*4)))

int gpio_direction_input(int gpio)
{
    int group = gpio >> 4, sub = gpio % 16;
    unsigned int reg, val;

    reg = REG_BASE(); val = readl(reg);
    val = REGIN(val, sub); writel(val, reg);

    return 0;
}

int gpio_direction_output(int gpio, int dat)
{
    int group = gpio >> 4, sub = gpio % 16;
    unsigned int reg, val;

    reg = REG_BASE(); val = readl(reg);
    val = REGOUT(val, sub); writel(val, reg);

    reg += gpio_dat_offs[sub];
    val = readl(reg); val |= (1<<sub);
    writel(val, reg);

    return 0;
}

// ret : high is 1, low is 0
int gpio_get_value(int gpio)
{
    int group = gpio >> 4, sub = gpio % 16;
    unsigned int reg, val;

    reg = REG_BASE() + gpio_dat_offs[sub];
    val = readl(reg);

    if(val & (1<<sub)) return 1;

    return 0;
}


int rtc_delay(int sec)
{

    return 0;
}


#endif
/********************* End Of File: gpio.c *********************/
