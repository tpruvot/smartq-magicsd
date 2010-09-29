/*
 * (C) Copyright 2010
 * Author: Maurus Cuelenaere <mcuelenaere@gmail.com>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __COMMON_H_
#define __COMMON_H_

#include <qi.h>
#include <utils.h>

#define s32 signed int
#define s16 signed short
#define s8  signed char

#define readl(a) (*(volatile unsigned int *)(a))
#define writel(v,a) (*(volatile unsigned int *)(a) = (v))

#endif	/* __COMMON_H_ */
