/****************************************************************
 * $ID: uncompress.c    Mon, 16 Feb 2009 15:38:50 +0800  root $ *
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
 * Last modified: 一, 22  6月 2009 08:37:36 +0800       by root #
 ****************************************************************/
#include "uncompress.h"
extern uint32_t total_size_sum;
extern int keep_userzone;

static uint32_t get_fw_fh_check_sum(firmware_fileheader *fw_fh)
{
    uint8_t *pchar = (uint8_t *)fw_fh;
    uint32_t i, ret = 0;

    for(i = 8; i < sizeof(firmware_fileheader); i++)
	ret += pchar[i];
    return ret;
}

int uncompress(char *filename, firmware_fileheader *fw_fh)
{
    char buf[100];
    FILE *fp;
    int fd, c, ret, i;
    int32_t cur_check_sum[7];
    
    fd = open(filename, O_RDONLY);
    if(fd == -1) {
	fprintf(stderr, "uncompress: open %s failed\n", filename);
	return -1;
    }
    read(fd, (void *)fw_fh, sizeof(firmware_fileheader));

    fprintf(stderr, "fw_fh->magic = %x\n", fw_fh->magic);
    if(fw_fh->magic != 0x39000032) {
	fprintf(stderr, "fw_fh->magic != 0x39000032\n");
	return -1;
    }
    fprintf(stderr, "fw_fh->fh_size = %d\n", fw_fh->fh_size);
    fprintf(stderr, "fw_fh->version = %d\n", fw_fh->version);
    fprintf(stderr, "fw_fh->date	= %d\n", fw_fh->date);
    fprintf(stderr, "fw_fh->vendor = %s\n", fw_fh->vendor);
    if(strcmp(fw_fh->vendor, "hhtech")) {
	fprintf(stderr, "fw_fh->vendor != hhtech\n");
	return -1;
    }
    fprintf(stderr, "fw_fh->component_count = %d\n", fw_fh->component_count);

    // qi:
    fprintf(stderr, "(fw_fh->qi).file.offset = %d\n", (fw_fh->qi).file.offset);
    fprintf(stderr, "(fw_fh->qi).file.size = %d\n", (fw_fh->qi).file.size);
    fprintf(stderr, "(fw_fh->qi).nand.offset = %d\n", (fw_fh->qi).nand.offset);
    fprintf(stderr, "(fw_fh->qi).nand.size = %d\n", (fw_fh->qi).nand.size);

    // u-boot:
    fprintf(stderr, "(fw_fh->u_boot).file.offset = %d\n", (fw_fh->u_boot).file.offset);
    fprintf(stderr, "(fw_fh->u_boot).file.size = %d\n", (fw_fh->u_boot).file.size);
    fprintf(stderr, "(fw_fh->u_boot).nand.offset = %d\n", (fw_fh->u_boot).nand.offset);
    fprintf(stderr, "(fw_fh->u_boot).nand.size = %d\n", (fw_fh->u_boot).nand.size);
    
    // zimage:
    fprintf(stderr, "(fw_fh->zimage).file.offset = %d\n", (fw_fh->zimage).file.offset); 
    fprintf(stderr, "(fw_fh->zimage).file.size = %d\n", (fw_fh->zimage).file.size); 
    fprintf(stderr, "(fw_fh->zimage).nand.offset = %d\n", (fw_fh->zimage).nand.offset); 
    fprintf(stderr, "(fw_fh->zimage).nand.size = %d\n", (fw_fh->zimage).nand.size);
    
    // initramfs:
    fprintf(stderr, "(fw_fh->initramfs).file.offset = %d\n", (fw_fh->initramfs).file.offset);
    fprintf(stderr, "(fw_fh->initramfs).file.size = %d\n", (fw_fh->initramfs).file.size);
    fprintf(stderr, "(fw_fh->initramfs).nand.offset = %d\n", (fw_fh->initramfs).nand.offset);
    fprintf(stderr, "(fw_fh->initramfs).nand.size = %d\n", (fw_fh->initramfs).nand.size);

    // rootfs: 
    fprintf(stderr, "(fw_fh->rootfs).file.offset = %d\n", (fw_fh->rootfs).file.offset);
    fprintf(stderr, "(fw_fh->rootfs).file.size = %d\n", (fw_fh->rootfs).file.size);
    fprintf(stderr, "(fw_fh->rootfs).nand.offset = %d\n", (fw_fh->rootfs).nand.offset);
    fprintf(stderr, "(fw_fh->rootfs).nand.size = %d\n", (fw_fh->rootfs).nand.size);
    
    // homefs: 
    fprintf(stderr, "(fw_fh->homefs).file.offset = %d\n", (fw_fh->homefs).file.offset);
    fprintf(stderr, "(fw_fh->homefs).file.size = %d\n", (fw_fh->homefs).file.size);
    fprintf(stderr, "(fw_fh->homefs).nand.offset = %d\n", (fw_fh->homefs).nand.offset);
    fprintf(stderr, "(fw_fh->homefs).nand.size = %d\n", (fw_fh->homefs).nand.size);
    if(!keep_userzone)
	total_size_sum = (fw_fh->homefs).file.offset + (fw_fh->homefs).file.size;
    else
	total_size_sum = (fw_fh->homefs).file.offset;
    total_size_sum += (fw_fh->fh_size + (fw_fh->u_boot).file.size + (fw_fh->zimage).file.size + (fw_fh->initramfs).file.size);
    total_size_sum -= (fw_fh->qi).file.size;
    fprintf(stderr, "total_size_sum = %d\n", total_size_sum);

    close(fd);

    return 0;
}

/****************** End Of File: uncompress.c ******************/
// vim:sts=4:ts=8: 
