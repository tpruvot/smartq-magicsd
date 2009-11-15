/****************************************************************
 * $ID$                                                         *
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
 * Last modified: 一, 22  6月 2009 09:24:32 +0800       by root #
 ****************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "firmware_header.h"

#define SIZE_PER_READ	(4 * 1024 * 1024)   // 4MB
static const char Q5[] = "SmartQ5";
static const char Q7[] = "SmartQ7";
static char *fwName	= Q7;

static int file_size[MAX_SECTIONS];
static uint32_t check_sum[MAX_SECTIONS];
static unsigned char *buffer = NULL;

static inline int get_sum(unsigned char *buffer, int buffer_size)
{
    int ret = 0, i;
    for(i = 0; i < buffer_size; i++)
	ret += buffer[i];
    return ret;
}

static uint32_t get_check_sum(char *file_name, int file_size)
{
    int fd;
    int remnant_size, size, ret = 0;
    
    fd = open(file_name, O_RDONLY);
    if(-1 == fd) {
	printf("open %s failed\n", file_name);
	return -1;
    }
    
    remnant_size = file_size;
    size = 0;
    while(remnant_size / SIZE_PER_READ > 0) {
	size += read(fd, buffer, SIZE_PER_READ);
	ret += get_sum(buffer, SIZE_PER_READ);
	remnant_size = file_size - size;
    }
    size += read(fd, buffer, remnant_size);
    ret += get_sum(buffer, remnant_size);

    close(fd);

    return ret;
}

static void fill_fw_fh(firmware_fileheader *fw_fh)
{
    fw_fh->magic = 0x39000032;	// '2009'
    fw_fh->fh_size = sizeof(firmware_fileheader); 
    fw_fh->version = 1;
    fw_fh->date	= time((time_t*)NULL);
    memset(fw_fh->vendor, 0, sizeof(fw_fh->vendor));
    strcpy(fw_fh->vendor, "hhtech");
    memset(fw_fh->model, 0, sizeof(fw_fh->model));
    strcpy(fw_fh->model, fwName);
    fw_fh->component_count = 7;


    // qi:  4K
    (fw_fh->qi).file.offset = sizeof(firmware_fileheader);
    (fw_fh->qi).file.size = file_size[QI];
    (fw_fh->qi).check_sum = check_sum[QI];
    (fw_fh->qi).nand.offset = 0;
    (fw_fh->qi).nand.size = file_size[QI];
    
    // u-boot: 256KB
    (fw_fh->u_boot).file.offset = (fw_fh->qi).file.offset + (fw_fh->qi).file.size;
    (fw_fh->u_boot).file.size = file_size[U_BOOT];
    (fw_fh->u_boot).check_sum = check_sum[U_BOOT];
    (fw_fh->u_boot).nand.offset = 2;
    (fw_fh->u_boot).nand.size = file_size[U_BOOT];
    
    // zimage: 2MB - 257KB
    (fw_fh->zimage).file.offset = (fw_fh->u_boot).file.offset + (fw_fh->u_boot).file.size;
    (fw_fh->zimage).file.size = file_size[ZIMAGE];
    (fw_fh->zimage).check_sum = check_sum[ZIMAGE];
    (fw_fh->zimage).nand.offset = 512 + 2;
    (fw_fh->zimage).nand.size = file_size[ZIMAGE];
    
    // initramfs: 6MB
    (fw_fh->initramfs).file.offset = (fw_fh->zimage).file.offset + (fw_fh->zimage).file.size;
    (fw_fh->initramfs).file.size = file_size[INITRAMFS];
    (fw_fh->initramfs).check_sum = check_sum[INITRAMFS];
    (fw_fh->initramfs).nand.offset = 4096;
    (fw_fh->initramfs).nand.size = file_size[INITRAMFS];
    
    // rootfs: 
    (fw_fh->rootfs).file.offset = (fw_fh->initramfs).file.offset + (fw_fh->initramfs).file.size;
    (fw_fh->rootfs).file.size = file_size[ROOTFS];
    (fw_fh->rootfs).check_sum = check_sum[ROOTFS];
    (fw_fh->rootfs).nand.offset = 0;	// not used
    (fw_fh->rootfs).nand.size = file_size[ROOTFS];
    
    // homefs: 
    (fw_fh->homefs).file.offset = (fw_fh->rootfs).file.offset + (fw_fh->rootfs).file.size;
    (fw_fh->homefs).file.size = file_size[HOMEFS];
    (fw_fh->homefs).check_sum = check_sum[HOMEFS];
    (fw_fh->homefs).nand.offset = 0;	// not used
    (fw_fh->homefs).nand.size = file_size[HOMEFS];

   // boot args: 2K
    (fw_fh->bootArgs).file.offset = (fw_fh->rootfs).file.offset + (fw_fh->rootfs).file.size;
    (fw_fh->bootArgs).file.size = file_size[BOOTARGS];
    (fw_fh->bootArgs).check_sum = check_sum[BOOTARGS];
    (fw_fh->bootArgs).nand.offset = 4097;
    (fw_fh->bootArgs).nand.size = file_size[BOOTARGS];
}

static uint32_t get_fw_fh_check_sum(firmware_fileheader *fw_fh)
{
    uint8_t *pchar = (uint8_t *)fw_fh;
    uint32_t i, ret = 0;

    for(i = 8; i < sizeof(firmware_fileheader); i++)
	ret += pchar[i];
    return ret;
}

int main(int argc, char *argv[])
{
    int i;
    struct stat buf;
    char system_cmd[512];
    int fd; /* firmware out */
    
    if(argc < 5 || argc > 7) {
	printf("Usage: mkSmartQ5/7 qi.bin u-boot.bin zImage initramfs.igz [ rootfs homefs] [bootargs]\n"
         "or\n"
         "Usage mkSmartQ5/7 qi.bin u-boot.bin zImage initramfs.igz\n"
         "where the filesystems may be either tar.gz or tar.xz\n"
         "NOTE: The name of this binary (mkSmartQ5 or mkSmartQ7) determines\n"
         "      the target device.\n"
         );

	exit(-1);
    }

    if (strstr(argv[0], Q5))
    {
       fwName = Q5;
    }
    else
    if (strstr(argv[0], Q7)) {
       fwName = Q7;
    }
    else
       printf("Unknown caller device model.\n");

    buffer = (unsigned char *)malloc(SIZE_PER_READ);
    if(NULL == buffer) {
	printf("malloc buffer failed\n");
	return -1;
    }

    for (i = 1 ; i < argc; i++) {
	if(stat(argv[i], &buf) < 0) {
	    printf("stat %s failed\n", argv[i]); exit(-1);
	} else { 
	    file_size[i-1] = buf.st_size;
	    check_sum[i-1] = get_check_sum(argv[i], file_size[i-1]);
	}
	printf("arg[%d] = %s, file_size = %d, check_sum = %d\n", i, argv[i], file_size[i-1], check_sum[i-1]);
    }   
    
    firmware_fileheader *fw_fh = (firmware_fileheader *)malloc(sizeof(firmware_fileheader));
    memset(fw_fh, 0, sizeof(firmware_fileheader));
    fill_fw_fh(fw_fh);
    fw_fh->check_sum = get_fw_fh_check_sum(fw_fh);
    printf("fw_fh->check_sum = %d\n", fw_fh->check_sum);

    unlink(fwName);
    fd = open(fwName, O_RDWR | O_CREAT, S_IRGRP | S_IWGRP);
    write(fd, (void *)fw_fh, sizeof(firmware_fileheader));
    close(fd);
    free(fw_fh);
    free(buffer);

    /* append each section */
    for (i = 1 ; i < argc ; i++)  {
    
        sprintf(system_cmd, "cat %s >> %s", argv[i], fwName);
        system(system_cmd);
    }

    return 0;
}
/******************* End Of File: compress.c *******************/
// vim:sts=4:ts=8: 
