
/* 
 * Font 6x8 (12bytes per char)
 *
 *
 * Version 1.0
 */


/* Note: binary data consists of one byte for each row of each character top
   to bottom, character 0 to character 255, six bytes per character. Each
   byte contains the same four character bits in both nybbles.
   MSBit to LSBit = left to right.
 */

//
// 6x8 terminal font
//

#define gfx_termmedium     41

#define font41WindowWidth  768
#define font41WindowHeight 8
#define font41Size         3072

//#define FONTDATAMAX 3072
//#define FONTBITS 12

//const unsigned char fontdata_6x8[3072] = {0}
/* $Id: font_6x8.h,v 1.4 2006/08/14 19:24:22 harbaum Exp $
 *
 * Based on 6x8 LCD4Linux font_6x8.h 1.4 - Tanguy Pruvot 2010
 *
 * Copyright (C) 1999, 2000, 2004 Michael Reinelt <reinelt@eunet.at>
 * Copyright (C) 2004 The LCD4Linux Team <lcd4linux-devel@users.sourceforge.net>
 *
 * LCD4Linux is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * LCD4Linux is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#define ______ 0x00
#define _____O 0x01
#define ____O_ 0x02
#define ____OO 0x03
#define ___O__ 0x04
#define ___O_O 0x05
#define ___OO_ 0x06
#define ___OOO 0x07
#define __O___ 0x08
#define __O__O 0x09
#define __O_O_ 0x0a
#define __O_OO 0x0b
#define __OO__ 0x0c
#define __OO_O 0x0d
#define __OOO_ 0x0e
#define __OOOO 0x0f
#define _O____ 0x10
#define _O___O 0x11
#define _O__O_ 0x12
#define _O__OO 0x13
#define _O_O__ 0x14
#define _O_O_O 0x15
#define _O_OO_ 0x16
#define _O_OOO 0x17
#define _OO___ 0x18
#define _OO__O 0x19
#define _OO_O_ 0x1a
#define _OO_OO 0x1b
#define _OOO__ 0x1c
#define _OOO_O 0x1d
#define _OOOO_ 0x1e
#define _OOOOO 0x1f

const unsigned char fontdata_6x8[256][8] = {

/*0x00*/ {0,0,0,0,0,0,0,0},
/*0x01*/ {0,0,0,0,0,0,0,0},
/*0x02*/ {0,0,0,0,0,0,0,0},
/*0x03*/ {0,0,0,0,0,0,0,0},
/*0x04*/ {0,0,0,0,0,0,0,0},
/*0x05*/ {0,0,0,0,0,0,0,0},
/*0x06*/ {0,0,0,0,0,0,0,0},
/*0x07*/ {0,0,0,0,0,0,0,0},
/*0x08*/ {0,0,0,0,0,0,0,0},
/*0x09*/ {0,0,0,0,0,0,0,0},
/*0x0a*/ {0,0,0,0,0,0,0,0},
/*0x0b*/ {0,0,0,0,0,0,0,0},
/*0x0c*/ {0,0,0,0,0,0,0,0},
/*0x0d*/ {0,0,0,0,0,0,0,0},
/*0x0e*/ {0,0,0,0,0,0,0,0},
/*0x0f*/ {0,0,0,0,0,0,0,0},

/*0x10*/ {0,0,0,0,0,0,0,0},
/*0x11*/ {0,0,0,0,0,0,0,0},
/*0x12*/ {0,0,0,0,0,0,0,0},
/*0x13*/ {0,0,0,0,0,0,0,0},
/*0x14*/ {0,0,0,0,0,0,0,0},
/*0x15*/ {0,0,0,0,0,0,0,0},
/*0x16*/ {0,0,0,0,0,0,0,0},
/*0x17*/ {0,0,0,0,0,0,0,0},
/*0x18*/ {0,0,0,0,0,0,0,0},
/*0x19*/ {0,0,0,0,0,0,0,0},
/*0x1a*/ {0,0,0,0,0,0,0,0},
/*0x1b*/ {0,0,0,0,0,0,0,0},
/*0x1c*/ {0,0,0,0,0,0,0,0},
/*0x1d*/ {0,0,0,0,0,0,0,0},
/*0x1e*/ {0,0,0,0,0,0,0,0},
/*0x1f*/ {0,0,0,0,0,0,0,0},
	
/*0x20*/ {______,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x21*/ {___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ______,
	      ______,
	      ___O__,
	      ______},
/*0x22*/ {__O_O_,
	      __O_O_,
	      __O_O_,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x23*/ {__O_O_,
	      __O_O_,
	      _OOOOO,
	      __O_O_,
	      _OOOOO,
	      __O_O_,
	      __O_O_,
	      ______},
/*0x24*/ {___O__,
	      __OOOO,
	      _O_O__,
	      __OOO_,
	      ___O_O,
	      _OOOO_,
	      ___O__,
	      ______},
/*0x25*/ {_OO___,
	      _OO__O,
	      ____O_,
	      ___O__,
	      __O___,
	      _O__OO,
	      ____OO,
	      ______},
/*0x26*/ {__OO__,
	      _O__O_,
	      _O_O__,
	      __O___,
	      _O_O_O,
	      _O__O_,
	      __OO_O,
	      ______},
/*0x27*/ {__OO__,
	      ___O__,
	      __O___,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x28*/ {____O_,
	      ___O__,
	      __O___,
	      __O___,
	      __O___,
	      ___O__,
	      ____O_,
	      ______},
/*0x29*/ {__O___,
	      ___O__,
	      ____O_,
	      ____O_,
	      ____O_,
	      ___O__,
	      __O___,
	      ______},
/*0x2a*/ {______,
	      ___O__,
	      _O_O_O,
	      __OOO_,
	      _O_O_O,
	      ___O__,
	      ______,
	      ______},
/*0x2b*/ {______,
	      ___O__,
	      ___O__,
	      _OOOOO,
	      ___O__,
	      ___O__,
	      ______,
	      ______},
/*0x2c*/ {______,
	      ______,
	      ______,
	      ______,
	      __OO__,
	      ___O__,
	      __O___,
	      ______},
/*0x2d*/ {______,
	      ______,
	      ______,
	      _OOOOO,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x2e*/ {______,
	      ______,
	      ______,
	      ______,
	      ______,
	      __OO__,
	      __OO__,
	      ______},
/*0x2f*/ {______,
	      _____O,
	      ____O_,
	      ___O__,
	      __O___,
	      _O____,
	      ______,
	      ______},
/*0x30*/ {__OOO_,
	      _O___O,
	      _O__OO,
	      _O_O_O,
	      _OO__O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x31*/ {___O__,
	      __OO__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      __OOO_,
	      ______},
/*0x32*/ {__OOO_,
	      _O___O,
	      _____O,
	      ____O_,
	      ___O__,
	      __O___,
	      _OOOOO,
	      ______},
/*0x33*/ {_OOOOO,
	      ____O_,
	      ___O__,
	      ____O_,
	      _____O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x34*/ {____O_,
	      ___OO_,
	      __O_O_,
	      _O__O_,
	      _OOOOO,
	      ____O_,
	      ____O_,
	      ______},
/*0x35*/ {_OOOOO,
	      _O____,
	      _O____,
	      _OOOO_,
	      _____O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x36*/ {___OO_,
	      __O___,
	      _O____,
	      _OOOO_,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x37*/ {_OOOOO,
	      _____O,
	      ____O_,
	      ___O__,
	      __O___,
	      __O___,
	      __O___,
	      ______},
/*0x38*/ {__OOO_,
	      _O___O,
	      _O___O,
	      __OOO_,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x39*/ {__OOO_,
	      _O___O,
	      _O___O,
	      __OOOO,
	      _____O,
	      ____O_,
	      __OO__,
	      ______},
/*0x3a*/ {______,
	      __OO__,
	      __OO__,
	      ______,
	      __OO__,
	      __OO__,
	      ______,
	      ______},
/*0x3b*/ {______,
	      __OO__,
	      __OO__,
	      ______,
	      __OO__,
	      ___O__,
	      __O___,
	      ______},
/*0x3c*/ {____O_,
	      ___O__,
	      __O___,
	      _O____,
	      __O___,
	      ___O__,
	      ____O_,
	      ______},
/*0x3d*/ {______,
	      ______,
	      _OOOOO,
	      ______,
	      _OOOOO,
	      ______,
	      ______,
	      ______},
/*0x3e*/ {_O____,
	      __O___,
	      ___O__,
	      ____O_,
	      ___O__,
	      __O___,
	      _O____,
	      ______},
/*0x3f*/ {__OOO_,
	      _O___O,
	      _____O,
	      ____O_,
	      ___O__,
	      ______,
	      ___O__,
	      ______},
/*0x40*/ {__OOO_,
	      _O___O,
	      _____O,
	      __OO_O,
	      _O_O_O,
	      _O_O_O,
	      __OOO_,
	      ______},
/*0x41*/ {__OOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      _OOOOO,
	      _O___O,
	      _O___O,
	      ______},
/*0x42*/ {_OOOO_,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      ______},
/*0x43*/ {__OOO_,
	      _O___O,
	      _O____,
	      _O____,
	      _O____,
	      _O___O,
	      __OOO_,
	      ______},
/*0x44*/ {_OOO__,
	      _O__O_,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O__O_,
	      _OOO__,
	      ______},
/*0x45*/ {_OOOOO,
	      _O____,
	      _O____,
	      _OOOO_,
	      _O____,
	      _O____,
	      _OOOOO,
	      ______},
/*0x46*/ {_OOOOO,
	      _O____,
	      _O____,
	      _OOOO_,
	      _O____,
	      _O____,
	      _O____,
	      ______},
/*0x47*/ {__OOO_,
	      _O___O,
	      _O____,
	      _O_OOO,
	      _O___O,
	      _O___O,
	      __OOOO,
	      ______},
/*0x48*/ {_O___O,
	      _O___O,
	      _O___O,
	      _OOOOO,
	      _O___O,
	      _O___O,
	      _O___O,
	      ______},
/*0x49*/ {__OOO_,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      __OOO_,
	      ______},
/*0x4a*/ {___OOO,
	      ____O_,
	      ____O_,
	      ____O_,
	      ____O_,
	      _O__O_,
	      __OO__,
	      ______},
/*0x4b*/ {_O___O,
	      _O__O_,
	      _O_O__,
	      _OO___,
	      _O_O__,
	      _O__O_,
	      _O___O,
	      ______},
/*0x4c*/ {_O____,
	      _O____,
	      _O____,
	      _O____,
	      _O____,
	      _O____,
	      _OOOOO,
	      ______},
/*0x4d*/ {_O___O,
	      _OO_OO,
	      _O_O_O,
	      _O_O_O,
	      _O___O,
	      _O___O,
	      _O___O,
	      ______},
/*0x4e*/ {_O___O,
	      _O___O,
	      _OO__O,
	      _O_O_O,
	      _O__OO,
	      _O___O,
	      _O___O,
	      ______},
/*0x4f*/ {__OOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x50*/ {_OOOO_,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      _O____,
	      _O____,
	      _O____,
	      ______},
/*0x51*/ {__OOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O_O_O,
	      _O__O_,
	      __OO_O,
	      ______},
/*0x52*/ {_OOOO_,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      _O_O__,
	      _O__O_,
	      _O___O,
	      ______},
/*0x53*/ {__OOOO,
	      _O____,
	      _O____,
	      __OOO_,
	      _____O,
	      _____O,
	      _OOOO_,
	      ______},
/*0x54*/ {_OOOOO,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ______},
/*0x55*/ {_O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x56*/ {_O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      __O_O_,
	      ___O__,
	      ______},
/*0x57*/ {_O___O,
	      _O___O,
	      _O___O,
	      _O_O_O,
	      _O_O_O,
	      _O_O_O,
	      __O_O_,
	      ______},
/*0x58*/ {_O___O,
	      _O___O,
	      __O_O_,
	      ___O__,
	      __O_O_,
	      _O___O,
	      _O___O,
	      ______},
/*0x59*/ {_O___O,
	      _O___O,
	      _O___O,
	      __O_O_,
	      ___O__,
	      ___O__,
	      ___O__,
	      ______},
/*0x5a*/ {_OOOOO,
	      _____O,
	      ____O_,
	      ___O__,
	      __O___,
	      _O____,
	      _OOOOO,
	      ______},
/*0x5b*/ {__OOO_,
	      __O___,
	      __O___,
	      __O___,
	      __O___,
	      __O___,
	      __OOO_,
	      ______},
/*0x5c*/ {_O___O,
	      __O_O_,
	      _OOOOO,
	      ___O__,
	      _OOOOO,
	      ___O__,
	      ___O__,
	      ______},
/*0x5d*/ {__OOO_,
	      ____O_,
	      ____O_,
	      ____O_,
	      ____O_,
	      ____O_,
	      __OOO_,
	      ______},
/*0x5e*/ {___O__,
	      __O_O_,
	      _O___O,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x5f*/ {______,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______,
	      _OOOOO,
	      ______},
/*0x60*/ {__O___,
	      ___O__,
	      ____O_,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
/*0x61*/ {______,
	      ______,
	      __OOO_,
	      _____O,
	      __OOOO,
	      _O___O,
	      __OOOO,
	      ______},
/*0x62*/ {_O____,
	      _O____,
	      _O_OO_,
	      _OO__O,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      ______},
/*0x63*/ {______,
	      ______,
	      __OOO_,
	      _O____,
	      _O____,
	      _O___O,
	      __OOO_,
	      ______},
/*0x64*/ {_____O,
	      _____O,
	      __OO_O,
	      _O__OO,
	      _O___O,
	      _O___O,
	      __OOOO,
	      ______},
/*0x65*/ {______,
	      ______,
	      __OOO_,
	      _O___O,
	      _OOOOO,
	      _O____,
	      __OOO_,
	      ______},
/*0x66*/ {___OO_,
	      __O__O,
	      __O___,
	      _OOO__,
	      __O___,
	      __O___,
	      __O___,
	      ______},
/*0x67*/ {______,
	      ______,
	      __OOOO,
	      _O___O,
	      _O___O,
	      __OOOO,
	      _____O,
	      __OOO_},
/*0x68*/ {_O____,
	      _O____,
	      _O_OO_,
	      _OO__O,
	      _O___O,
	      _O___O,
	      _O___O,
	      ______},
/*0x69*/ {___O__,
	      ______,
	      __OO__,
	      ___O__,
	      ___O__,
	      ___O__,
	      __OOO_,
	      ______},
/*0x6a*/ {____O_,
	      ______,
	      ___OO_,
	      ____O_,
	      ____O_,
	      _O__O_,
	      __OO__,
	      ______},
/*0x6b*/ {__O___,
	      __O___,
	      __O__O,
	      __O_O_,
	      __OO__,
	      __O_O_,
	      __O__O,
	      ______},
/*0x6c*/ {__OO__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      __OOO_,
	      ______},
/*0x6d*/ {______,
	      ______,
	      _OO_O_,
	      _O_O_O,
	      _O_O_O,
	      _O___O,
	      _O___O,
	      ______},
/*0x6e*/ {______,
	      ______,
	      _OOOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O___O,
	      ______},
/*0x6f*/ {______,
	      ______,
	      __OOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},
/*0x70*/ {______,
	      ______,
	      _OOOO_,
	      _O___O,
	      _O___O,
	      _OOOO_,
	      _O____,
	      _O____},
/*0x71*/ {______,
	      ______,
	      __OO_O,
	      _O__OO,
	      _O___O,
	      __OOOO,
	      _____O,
	      _____O},
/*0x72*/ {______,
	      ______,
	      _O_OO_,
	      _OO__O,
	      _O____,
	      _O____,
	      _O____,
	      ______},
/*0x73*/ {______,
	      ______,
	      __OOO_,
	      _O____,
	      __OOO_,
	      _____O,
	      _OOOO_,
	      ______},
/*0x74*/ {__O___,
	      __O___,
	      _OOO__,
	      __O___,
	      __O___,
	      __O__O,
	      ___OO_,
	      ______},
/*0x75*/ {______,
	      ______,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O__OO,
	      __OO_O,
	      ______},
/*0x76*/ {______,
	      ______,
	      _O___O,
	      _O___O,
	      _O___O,
	      __O_O_,
	      ___O__,
	      ______},
/*0x77*/ {______,
	      ______,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O_O_O,
	      __O_O_,
	      ______},
/*0x78*/ {______,
	      ______,
	      _O___O,
	      __O_O_,
	      ___O__,
	      __O_O_,
	      _O___O,
	      ______},
/*0x79*/ {______,
	      ______,
	      _O___O,
	      _O___O,
	      __OOOO,
	      _____O,
	      __OOO_,
	      ______},
/*0x7a*/ {______,
	      ______,
	      _OOOOO,
	      ____O_,
	      ___O__,
	      __O___,
	      _OOOOO,
	      ______},
/*0x7b*/ {____O_,
	      ___O__,
	      ___O__,
	      __O___,
	      ___O__,
	      ___O__,
	      ____O_,
	      ______},
/*0x7c*/ {___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ___O__,
	      ______},
/*0x7d*/ {__O___,
	      ___O__,
	      ___O__,
	      ____O_,
	      ___O__,
	      ___O__,
	      __O___,
	      ______},
/*0x7e*/ {______,
	      ___O__,
	      ____O_,
	      _OOOOO,
	      ____O_,
	      ___O__,
	      ______,
	      ______},

/*0x7f*/ {______,
	      ___O__,
	      __O___,
	      _OOOOO,
	      __O___,
	      ___O__,
	      ______,
	      ______},

/*0x80*/ {0,0,0,0,0,0,0,0},
/*0x81*/ {0,0,0,0,0,0,0,0},
/*0x82*/ {0,0,0,0,0,0,0,0},
/*0x83*/ {0,0,0,0,0,0,0,0},
/*0x84*/ {0,0,0,0,0,0,0,0},
/*0x85*/ {0,0,0,0,0,0,0,0},
/*0x86*/ {0,0,0,0,0,0,0,0},
/*0x87*/ {0,0,0,0,0,0,0,0},
/*0x88*/ {0,0,0,0,0,0,0,0},
/*0x89*/ {0,0,0,0,0,0,0,0},
/*0x8a*/ {0,0,0,0,0,0,0,0},
/*0x8b*/ {0,0,0,0,0,0,0,0},
/*0x8c*/ {0,0,0,0,0,0,0,0},
/*0x8d*/ {0,0,0,0,0,0,0,0},
/*0x8e*/ {0,0,0,0,0,0,0,0},
/*0x8f*/ {0,0,0,0,0,0,0,0},

/*0x90*/ {0,0,0,0,0,0,0,0},
/*0x91*/ {0,0,0,0,0,0,0,0},
/*0x92*/ {0,0,0,0,0,0,0,0},
/*0x93*/ {0,0,0,0,0,0,0,0},
/*0x94*/ {0,0,0,0,0,0,0,0},
/*0x95*/ {0,0,0,0,0,0,0,0},
/*0x96*/ {0,0,0,0,0,0,0,0},
/*0x97*/ {0,0,0,0,0,0,0,0},
/*0x98*/ {0,0,0,0,0,0,0,0},
/*0x99*/ {0,0,0,0,0,0,0,0},
/*0x9a*/ {0,0,0,0,0,0,0,0},
/*0x9b*/ {0,0,0,0,0,0,0,0},
/*0x9c*/ {0,0,0,0,0,0,0,0},
/*0x9d*/ {0,0,0,0,0,0,0,0},
/*0x9e*/ {0,0,0,0,0,0,0,0},
/*0x9f*/ {0,0,0,0,0,0,0,0},

/*0xa0*/ {0,0,0,0,0,0,0,0},
/*0xa1*/ {0,0,0,0,0,0,0,0},
/*0xa2*/ {0,0,0,0,0,0,0,0},
/*0xa3*/ {0,0,0,0,0,0,0,0},
/*0xa4*/ {0,0,0,0,0,0,0,0},
/*0xa5*/ {0,0,0,0,0,0,0,0},
/*0xa6*/ {0,0,0,0,0,0,0,0},
/*0xa7*/ {0,0,0,0,0,0,0,0},
/*0xa8*/ {0,0,0,0,0,0,0,0},
/*0xa9*/ {0,0,0,0,0,0,0,0},
/*0xaa*/ {0,0,0,0,0,0,0,0},
/*0xab*/ {0,0,0,0,0,0,0,0},
/*0xac*/ {0,0,0,0,0,0,0,0},
/*0xad*/ {0,0,0,0,0,0,0,0},
/*0xae*/ {0,0,0,0,0,0,0,0},
/*0xaf*/ {0,0,0,0,0,0,0,0},

/*0xb0*/ {__OOO_,
	      __O_O_,
	      __OOO_,
	      ______,
	      ______,
	      ______,
	      ______,
	      ______},
	      
/*0xb1*/ {0,0,0,0,0,0,0,0},
/*0xb2*/ {0,0,0,0,0,0,0,0},
/*0xb3*/ {0,0,0,0,0,0,0,0},
/*0xb4*/ {0,0,0,0,0,0,0,0},
/*0xb5*/ {0,0,0,0,0,0,0,0},
/*0xb6*/ {0,0,0,0,0,0,0,0},
/*0xb7*/ {0,0,0,0,0,0,0,0},
/*0xb8*/ {0,0,0,0,0,0,0,0},
/*0xb9*/ {0,0,0,0,0,0,0,0},
/*0xba*/ {0,0,0,0,0,0,0,0},
/*0xbb*/ {0,0,0,0,0,0,0,0},
/*0xbc*/ {0,0,0,0,0,0,0,0},
/*0xbd*/ {0,0,0,0,0,0,0,0},
/*0xbe*/ {0,0,0,0,0,0,0,0},
/*0xbf*/ {0,0,0,0,0,0,0,0}, 

/*0xc0*/ {0,0,0,0,0,0,0,0},
/*0xc1*/ {0,0,0,0,0,0,0,0},
/*0xc2*/ {0,0,0,0,0,0,0,0},
/*0xc3*/ {0,0,0,0,0,0,0,0},
/*0xc4*/ {0,0,0,0,0,0,0,0},
/*0xc5*/ {0,0,0,0,0,0,0,0},
/*0xc6*/ {0,0,0,0,0,0,0,0},
/*0xc7*/ {0,0,0,0,0,0,0,0},
/*0xc8*/ {0,0,0,0,0,0,0,0},
/*0xc9*/ {0,0,0,0,0,0,0,0},
/*0xca*/ {0,0,0,0,0,0,0,0},
/*0xcb*/ {0,0,0,0,0,0,0,0},
/*0xcc*/ {0,0,0,0,0,0,0,0},
/*0xcd*/ {0,0,0,0,0,0,0,0},
/*0xce*/ {0,0,0,0,0,0,0,0},
/*0xcf*/ {0,0,0,0,0,0,0,0}, 

/*0xd0*/ {0,0,0,0,0,0,0,0},
/*0xd1*/ {0,0,0,0,0,0,0,0},
/*0xd2*/ {0,0,0,0,0,0,0,0},
/*0xd3*/ {0,0,0,0,0,0,0,0},
/*0xd4*/ {0,0,0,0,0,0,0,0},
/*0xd5*/ {0,0,0,0,0,0,0,0},
/*0xd6*/ {0,0,0,0,0,0,0,0},
/*0xd7*/ {0,0,0,0,0,0,0,0},
/*0xd8*/ {0,0,0,0,0,0,0,0},
/*0xd9*/ {0,0,0,0,0,0,0,0},
/*0xda*/ {0,0,0,0,0,0,0,0},
/*0xdb*/ {0,0,0,0,0,0,0,0},
/*0xdc*/ {0,0,0,0,0,0,0,0},
/*0xdd*/ {0,0,0,0,0,0,0,0},
/*0xde*/ {0,0,0,0,0,0,0,0},
/*0xdf*/ {0,0,0,0,0,0,0,0},

/*0xd0*/ {0,0,0,0,0,0,0,0},

/*0xe1*/ {__O_O_,
	      ______,
	      __OOO_,
	      _____O,
	      __OOOO,
	      _O___O,
	      __OOOO,
	      ______},
/*0xe2*/ {__OOO_,
	      _O___O,
	      _OOOO_,
	      _O___O,
	      _O___O,
	      _O_OO_,
	      _O____,
	      ______},

/*0xe3*/ {0,0,0,0,0,0,0,0},
/*0xe4*/ {0,0,0,0,0,0,0,0},
/*0xe5*/ {0,0,0,0,0,0,0,0},
/*0xe6*/ {0,0,0,0,0,0,0,0},
/*0xe7*/ {0,0,0,0,0,0,0,0},
/*0xe8*/ {0,0,0,0,0,0,0,0},
/*0xe9*/ {0,0,0,0,0,0,0,0},
/*0xea*/ {0,0,0,0,0,0,0,0},
/*0xeb*/ {0,0,0,0,0,0,0,0},
/*0xec*/ {0,0,0,0,0,0,0,0},
/*0xed*/ {0,0,0,0,0,0,0,0},
/*0xee*/ {0,0,0,0,0,0,0,0},

/*0xef*/ {__O_O_,
	      ______,
	      __OOO_,
	      _O___O,
	      _O___O,
	      _O___O,
	      __OOO_,
	      ______},

/*0xf0*/ {0,0,0,0,0,0,0,0},
/*0xf1*/ {0,0,0,0,0,0,0,0},
/*0xf2*/ {0,0,0,0,0,0,0,0},
/*0xf3*/ {0,0,0,0,0,0,0,0},
/*0xf4*/ {0,0,0,0,0,0,0,0},
      
/*0xf5*/ {__O_O_,
	      ______,
	      _O___O,
	      _O___O,
	      _O___O,
	      _O__OO,
	      __OO_O,
	      ______},

/*0xf6*/ {0,0,0,0,0,0,0,0},
/*0xf7*/ {0,0,0,0,0,0,0,0},
/*0xf8*/ {0,0,0,0,0,0,0,0},
/*0xf9*/ {0,0,0,0,0,0,0,0},
/*0xfa*/ {0,0,0,0,0,0,0,0},
/*0xfb*/ {0,0,0,0,0,0,0,0},
/*0xfc*/ {0,0,0,0,0,0,0,0},
/*0xfd*/ {0,0,0,0,0,0,0,0},
/*0xfe*/ {0,0,0,0,0,0,0,0},
/*0xff*/ {0,0,0,0,0,0,0,0},

};