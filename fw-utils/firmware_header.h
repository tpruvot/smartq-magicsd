/****************************************************************
 * $ID: compress.h      Mon, 16 Feb 2009 15:39:39 +0800  root $ *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  (Meihui Fan)  <mhfan@hhcn.com>            *
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	        *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 * Last modified: Wed, 11 Mar 2009 09:05:27 +0800       by root #
 ****************************************************************/
#ifndef FIRMWARE_HEADER_H
#define FIRMWARE_HEADER_H

enum sections {
    QI,
    U_BOOT,
    ZIMAGE,
    INITRAMFS,
    ROOTFS,
    HOMEFS,
    BOOTARGS,
    MAX_SECTIONS,
};

#define NANDBLKSZ            (2048)

typedef struct _firmware_fileheader {
    uint32_t magic;	    // '2009'
    uint32_t check_sum;	    // for 8 ~ .fh_size 
    uint32_t fh_size;
    uint32_t version;	 // major.minor.revision.xxx, each section in 8-bits
    uint32_t date;       // seconds since the Epoch
    char vendor[28];	    // char vendor_string[] 
    uint32_t machType;   // machine type from linux/include/asm/mach-types.h
    //uint32_t nand_off_end1=16M/512, nand_off_end2=8M/512;  // offset from INAND END(BLOCKS)
    uint32_t component_count /* = 4*/;
    struct stanza  {
       struct {
	       uint32_t offset, size;
	    } file, nand;
	    uint32_t check_sum;
    } qi, u_boot, zimage, initramfs, rootfs, homefs, bootArgs; // components[];
} firmware_fileheader;

typedef struct _firmware_fileheader FWFileHdr;

#endif
/******************* End Of File: compress.h *******************/
// vim:sts=4:ts=8: 
