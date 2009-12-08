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
 * Last modified: 一, 22  6月 2009 08:37:36 +0800       by root #
 ****************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "firmware_header.h"

typedef struct section {
   const char * name;
   size_t       stanzaOffset;
} Section;

static Section sects[MAX_SECTIONS] = {
   { "QI",        FW_STANZA_OFFSET(qi),        }, 
   { "UBOOT",     FW_STANZA_OFFSET(u_boot),    },    
   { "ZIMAGE",    FW_STANZA_OFFSET(zimage),    },
   { "INITRAMFS", FW_STANZA_OFFSET(initramfs), },
   { "ROOTFS",    FW_STANZA_OFFSET(rootfs),    },
   { "HOMEFS",    FW_STANZA_OFFSET(homefs),    },
//    { "BOOTARGS",  FW_STANZA_OFFSET(bootArgs),  }
};


static uint32_t get_fw_fh_check_sum(firmware_fileheader *fw_fh)
{
    uint8_t *pchar = (uint8_t *)fw_fh;
    uint32_t i, ret = 0;

    for(i = 8; i < fw_fh->fh_size ; i++)
	ret += pchar[i];
    return ret;
}

int extract(char *filename, firmware_fileheader *fw_fh)
{
    int fd;
    int i;
    struct stanza *stp;
    
    fd = open(filename, O_RDONLY);
    if(fd == -1) {
	fprintf(stderr, "extract: open %s failed\n", filename);
	return -1;
    }
    read(fd, (void *)fw_fh, sizeof(firmware_fileheader));

    fprintf(stderr, "fw_fh->magic = %x\n", fw_fh->magic);
    if(fw_fh->magic != 0x39000032) {
	fprintf(stderr, "ERROR: magic != 0x39000032\n");
	return -1;
    }

    if (get_fw_fh_check_sum(fw_fh) != fw_fh->check_sum) {
	fprintf(stderr, "ERROR: checksum = 0x%x calc'ed 0x%x\n",
            fw_fh->check_sum, get_fw_fh_check_sum(fw_fh));
        return -1;
    }

    if (fw_fh->machType)  {  /* old style had no machType field */
       static const char q5cmd[] = "grep SMARTQ5 /proc/cpuinfo";
       static const char q7cmd[] = "grep SMARTQ7 /proc/cpuinfo";
       int ret = -1;
          /* ret 0:  found at least once */
          /* ret 1:  not found */
          /* ret 2:  file not found */
       if (fw_fh->machType == 2534) { /* smartQ5 */
          ret = WEXITSTATUS(system(q7cmd));
          if (ret == 0) { /* wrong platform */
             fprintf(stderr,"Attempt to install Q7 firmware on platform\n");
             return -1;
          }
       }
       else
       if (fw_fh->machType == 2479) { /* smartQ5 */
          ret = WEXITSTATUS(system(q5cmd));
          if (ret == 0) { /* wrong platform */
             fprintf(stderr,"Attempt to install Q7 firmware on platform\n");
             return -1;
          }
       }
    }

    fprintf(stderr, "Header:    header size     = %d\n", fw_fh->fh_size);
    fprintf(stderr, "Header:    version         = %d\n", fw_fh->version);
    fprintf(stderr, "Header:    date            = %s\n", 
        ctime((time_t*)&fw_fh->date));
    fprintf(stderr, "Header:    vendor          = %s\n", fw_fh->vendor);
    if(strcmp(fw_fh->vendor, "hhtech")) {
	fprintf(stderr, "fw_fh->vendor != hhtech\n");
	return -1;
    }
    fprintf(stderr, "Header:    component_count = %d\n", fw_fh->component_count);

    if (fw_fh->machType == MACH_TYPE_SMARTQ5)
       fprintf(stderr,"Header:    machType        = SmartQ5\n");
    else
    if (fw_fh->machType == MACH_TYPE_SMARTQ7)
       fprintf(stderr,"Header:    machType        = SmartQ7\n");
    else
       fprintf(stderr,"Header:    machType        = %d\n", fw_fh->machType);

    for (i = QI ; i < fw_fh->component_count ; i++) {
        stp = (struct stanza *) ((void*) fw_fh + sects[i].stanzaOffset);

        fprintf(stderr, "%10s file offset = %d\n", sects[i].name, stp->file.offset);
        fprintf(stderr, "%10s file size   = %d\n", sects[i].name, stp->file.size);
        fprintf(stderr, "%10s nand offset = %d\n", sects[i].name, stp->nand.offset);
        fprintf(stderr, "%10s nand size   = %d\n", sects[i].name, stp->nand.size);
    }

    close(fd);

    return 0;
}

/****************** End Of File: extract.c ******************/
// vim:sts=4:ts=8: 
