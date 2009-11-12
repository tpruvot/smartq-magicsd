/****************************************************************
 * $ID: compress.c      Mon, 16 Feb 2009 10:35:21 +0800  root $ *
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
#include "compress.h"

#define SIZE_PER_READ	(4 * 1024 * 1024)   // 4MB
#define FIRMWARE_NAME	"SmartQ5"

static int fd_array[FIRMWARE + 1];
static int file_size[FIRMWARE + 1];
static uint32_t check_sum[FIRMWARE + 1];
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
    fw_fh->component_count = 5;

    // qi: 
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
    
    if(argc < 5 || argc > 7) {
	printf("usage1: ./compress qi-s3c6410-master_316aa06163e44f90 u-boot.bin zImage rootfs.uboot rootfs-mojo.tgz home-mojo.tgz\nusage2: ./compress qi-s3c6410-master_316aa06163e44f90 u-boot.bin zImage rootfs.uboot\n");
	exit(-1);
    }

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

    sprintf(system_cmd, "rm %s", FIRMWARE_NAME);
    system(system_cmd);
    fd_array[FIRMWARE] = open(FIRMWARE_NAME, O_RDWR | O_CREAT, S_IRGRP | S_IWGRP);
    write(fd_array[FIRMWARE], (void *)fw_fh, sizeof(firmware_fileheader));
    close(fd_array[FIRMWARE]);
    free(fw_fh);
    free(buffer);
    sprintf(system_cmd, "cat %s %s %s %s >> %s", argv[1], argv[2], argv[3], argv[4], FIRMWARE_NAME);
    system(system_cmd);

    if(7 == argc) {
	sprintf(system_cmd, "cat %s %s >> %s", argv[5], argv[6], FIRMWARE_NAME);
	system(system_cmd);
    }
    
    return 0;
}
/******************* End Of File: compress.c *******************/
// vim:sts=4:ts=8: 
