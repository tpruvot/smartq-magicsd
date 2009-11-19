/****************************************************************
 * $ID$                                                         *
 *                                                              *
 * Description:                                                 *
 *                                                              *
 * Maintainer:  (Meihui Fan)  <mhfan@hhcn.com>                  *
 *                                                              *
 * Copyright (C)  2009  HHTech                                  *
 *   www.hhcn.com, www.hhcn.org                                 *
 *   All rights reserved.                                       *
 *                                                              *
 * Significant restructure dave at chronolytics dot com         *
 *                                                              *
 * This file is free software;                                  *
 *   you are free to modify and/or redistribute it   	          *
 *   under the terms of the GNU General Public Licence (GPL).   *
 *                                                              *
 ****************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#include "firmware_header.h"

/* see linux/include/asm-xxx/mach-types.h */
#define MACH_TYPE_SMARTQ5              2534
#define MACH_TYPE_SMARTQ7              2479

#define SIZE_PER_READ	(4 * 1024 * 1024)   // 4MB
static char Q5[] = "SmartQ5";
static char Q7[] = "SmartQ7";
static char *fwName	= Q7;
static unsigned machType = MACH_TYPE_SMARTQ7; /* default */

static unsigned char *buffer = NULL;

#define NOT_IN_NAND (0)

#define FW_STANZA_OFFSET(stanza) (offsetof(FWFileHdr, stanza))

typedef struct section {
   const char * name;
   uint32_t     nandOffset;  /* nand offsets are in 2K blocks */
   size_t       stanzaOffset;
   uint32_t     fileSize;
   uint32_t     checkSum;
} Section;

static Section sects[MAX_SECTIONS] = {
   { "QI",        0,           FW_STANZA_OFFSET(qi),        }, 
   { "UBOOT",     2,           FW_STANZA_OFFSET(u_boot),    },    
   { "ZIMAGE",    512+2,       FW_STANZA_OFFSET(zimage),    },
   { "INITRAMFS", 4096,        FW_STANZA_OFFSET(initramfs), },
   { "ROOTFS",    NOT_IN_NAND, FW_STANZA_OFFSET(rootfs),    },
   { "HOMEFS",    NOT_IN_NAND, FW_STANZA_OFFSET(homefs),    },
   { "BOOTARGS",  4097,        FW_STANZA_OFFSET(bootArgs),  }
};


static inline int get_sum(unsigned char *buffer, int buffer_size)
{
    int ret = 0, i;
    for(i = 0; i < buffer_size; i++)
	ret += buffer[i];
    return ret;
}

static uint32_t get_check_sum(char *file_name, int file_size)
{
    /* use this to boot one-true-smartQ kernel */
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

static void fill_fw_fh(FWFileHdr *fw_fh)
{
    unsigned fileOffset = sizeof(FWFileHdr);
    struct stanza *stp;
    void* stanzaBase = fw_fh;
    int i;

    fw_fh->magic = 0x39000032;	// '2009'
    fw_fh->fh_size = sizeof(FWFileHdr); 
    fw_fh->version = 1;
    fw_fh->date	= time((time_t*)NULL);
    memset(fw_fh->vendor, 0, sizeof(fw_fh->vendor));
    strcpy(fw_fh->vendor, "hhtech");

    /* use this to boot one-true-smartQ kernel */
    fw_fh->machType = machType;  

    fw_fh->component_count = 7;

    for (i = QI ; i < MAX_SECTIONS; fileOffset += sects[i].fileSize, i++) {
        /* an array would be a lot easier :-) */
        stp = (struct stanza *) (stanzaBase + sects[i].stanzaOffset);

        stp->file.offset = fileOffset;

        stp->file.size   = sects[i].fileSize;
        stp->check_sum   = sects[i].checkSum;
        stp->nand.offset = sects[i].nandOffset;
        stp->nand.size   = sects[i].fileSize;
    }
}

static uint32_t get_fw_fh_check_sum(FWFileHdr *fw_fh)
{
    uint8_t *pchar = (uint8_t *)fw_fh;
    uint32_t i, ret = 0;

    for(i = 8; i < sizeof(FWFileHdr); i++)
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

	    exit(1);
    }

    if (strstr(argv[0], Q5))
    {
       fwName = Q5;
       machType = MACH_TYPE_SMARTQ5;
    }
    else
    if (strstr(argv[0], Q7)) {
       fwName = Q7;
       machType = MACH_TYPE_SMARTQ7;
    }
    else {
       printf("Unknown caller device model.\n");
       exit(3);
    }

    printf("Creating installable image for %s.\n", fwName);

    buffer = (unsigned char *)malloc(SIZE_PER_READ);
    if(NULL == buffer) {
	    printf("malloc buffer failed\n");
	    return -1;
    }

    printf("        Section      Size   Checksum  Name\n");
    printf("=============================================================\n");
    for (i = QI ; i < MAX_SECTIONS; i++) {
	    if (i + 1 == argc) break;  /* no more args */

	    if (strcmp(argv[i + 1], ".") == 0)  /* skip files named "." */
	    {
		    sects[i].fileSize = 0;
		    sects[i].checkSum = 0;
		    continue;
	    }
	    if(stat(argv[i + 1], &buf) < 0) {
	        printf("stat %s failed\n", argv[i + 1]); exit(4);
	    } else { 
	        sects[i].fileSize = buf.st_size;
	        sects[i].checkSum = get_check_sum(argv[i + 1], buf.st_size);
	    }
       printf("%3d %11s %9d 0x%08x  %s\n",
	       i, sects[i].name, sects[i].fileSize, sects[i].checkSum, argv[i + 1]);
    }   
    
    FWFileHdr *fw_fh = (FWFileHdr *) calloc(sizeof(FWFileHdr), 1);

    fill_fw_fh(fw_fh);

    fw_fh->check_sum = get_fw_fh_check_sum(fw_fh);

    printf("Header check sum = 0x%x\n", fw_fh->check_sum);

    unlink(fwName);
    fd = open(fwName, O_RDWR | O_CREAT, 0644);

    if (fd == -1) {
       printf("Cannot open firmware file: %s.\n", strerror(errno));
       exit(2);
    }

    write(fd, (void *)fw_fh, sizeof(FWFileHdr));
    close(fd);
    free(fw_fh);
    free(buffer);

    /* append each section */
    for (i = 1 ; i < argc ; i++)  {
	    if (strcmp(argv[i], ".") == 0)  /* skip files named "." */
	       continue;
       sprintf(system_cmd, "cat %s >> %s", argv[i], fwName);
       system(system_cmd);
    }

    return 0;
}
/******************* End Of File: compress.c *******************/
// vim:sts=4:ts=8: 
