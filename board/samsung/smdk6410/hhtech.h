/********************************************************************
* $ID: hhtech.h         ËÄ, 05  2ÔÂ 2009 11:44:03 +0800  whg        *
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
#ifndef  __HHTECH_DEF_H
#define  __HHTECH_DEF_H

#define KEY_POWER               (1<<4)          // power
#define KEY_UP                  (1<<0)
#define KEY_DOWN                (1<<1)
#define KEY_LEFT                (1<<2)
#define KEY_RIGHT               (1<<3)
#define KEY_QUIT                (1<<5)
#define KEY_MENU                (1<<6)
#define KEY_ADD                 (1<<7)
#define KEY_SUB                 (1<<8)
#define KEY_LOCK                (1<<9)


#define KEY_NONE               (0)
#define KEY_UPGRADE            (KEY_UP)
#define KEY_POWER_ON           (KEY_POWER)


typedef struct tag_key_info {
    unsigned short key;
    unsigned char  gpio;   // current set maxt is 256
    unsigned char  vtg;    //
}KeyInfo;

extern KeyInfo gkeys[];


int gpio_direction_input(int gpio);
int gpio_direction_output(int gpio, int dat);
int gpio_get_value(int gpio);


int rtc_delay(int sec);


#endif// __HHTECH_DEF_H
/******************** End Of File: hhtech.h ********************/
