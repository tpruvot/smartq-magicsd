/*
 * (C) Copyright 2007 OpenMoko, Inc.
 * Author: xiangfu liu <xiangfu@openmoko.org>
 *         Andy Green <andy@openmoko.com>
 *
 * Configuation settings for the OPENMOKO Neo GTA02 Linux GSM phone
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

/* NOTE this stuff runs in steppingstone context! */


#include <qi.h>
#include <neo_gta03.h>
#include <neo_smdk6410.h>
#ifdef HHTECH_MINIPMP
#include "../../../../fw-utils/firmware_header.h"
#endif

#define stringify2(s) stringify1(s)
#define stringify1(s) #s

extern void bootloader_second_phase(void);

const struct board_api *boards[] = {
#ifndef HHTECH_MINIPMP
			&board_api_gta03,
#endif
			&board_api_smdk6410,
			NULL /* always last */
};

struct board_api const * this_board;
extern int is_jtag;

#ifdef HHTECH_MINIPMP
#define RD_MEM_ADDR          (0x53000000) // u-boot run here
#define UBOOT_BLKS           (256*2)      // max size is 256K

#define globalBlockSizeHide (*((volatile unsigned int*)(0x0C004000-0x4)))
#define CHANNEL             ((*(volatile u32*)0x0C003FEC == 0x7c300000) ? 1 : 0)
#define SDHC                (*(volatile u32*)0x0C003FF8 & 0x01)
/**
 * * This Function copies SD/MMC Card Data to memory.
 * * Always use EPLL source clock.
 * * @param channel : HSMMC Controller channel number ( Not support. Depend on GPN15, GPN14 and GPN13 )
 * * @param StartBlkAddress : Source card(SD/MMC) Address.(It must block address.)
 * * @param blockSize : Number of blocks to copy.
 * * @param memoryPtr : Buffer to copy from.
 * * @param with_init : reinitialize or not
 * * @return bool(unsigend char) - Success or failure.
 * */
typedef int (*CopyFunc)(int, u32, u16, u32*, int);

static inline int CopyMMCtoMem(int chan, u32 startBlkAddr,
                               u16 nBlks, u32* memPtr, int reinit)
{
   CopyFunc rom =  *(CopyFunc *)0x0c004008;

   return rom(chan, startBlkAddr, nBlks, memPtr, reinit);
}


#include <s3c6410.h>
void led_set(int on);
int do_load_uboot(void);
#endif

#include <serial-s3c64xx.h>

void start_qi(void)
{
	int flag = 0;
	int board = 0;
	unsigned int sd_sectors = 0;

#ifdef HHTECH_MINIPMP
	led_set(0);
#endif
	/*
	 * well, we can be running on this CPU two different ways.
	 *
	 * 1) We were copied into steppingstone and TEXT_BASE already
	 *    by JTAG.  We don't have to do anything else.  JTAG script
	 *    then sets data at address 0x4 to 0xffffffff as a signal we
	 *    are running by JTAG.
	 *
	 * 2) We only got our first 4K into steppingstone, we need to copy
	 *    the rest of ourselves into TEXT_BASE.
	 *
	 * So we do the copy out of NAND only if we see we did not come up
	 * under control of JTAG.
	 */


	/* ask all the boards we support in turn if they recognize this
	 * hardware we are running on, accept the first positive answer
	 */

	this_board = boards[board];
	while (!flag && this_board)
		/* check if it is the right board... */
		if (this_board->is_this_board())
			flag = 1;
		else
			this_board = boards[board++];

	/* okay, do the critical port and serial init for our board */

	if (this_board->early_port_init)
		this_board->early_port_init();

	set_putc_func(this_board->putc);

	/* stick some hello messages on debug console */

	puts("\n\n\nQi Bootloader "stringify2(QI_CPU)"  "
				   stringify2(BUILD_HOST)" "
				   stringify2(BUILD_VERSION)" "
				   "\n");

	puts(stringify2(BUILD_DATE) "  Copyright (C) 2008 Openmoko, Inc.\n\n");

#ifdef HHTECH_MINIPMP
	if( (flag = do_load_uboot()) != 1) {
	    unsigned int channel = CHANNEL;

	    // uboot max size is 256K
	    // SDHC globalBlockSizeHide = sact sd blk num - 1024
	    if(SDHC) sd_sectors = globalBlockSizeHide - UBOOT_BLKS - 16 - 1 - 1;
	    else     sd_sectors = globalBlockSizeHide - UBOOT_BLKS - 16 - 1 - 1;

	    flag = CopyMMCtoMem(channel, sd_sectors, UBOOT_BLKS,
		(u32*)RD_MEM_ADDR, 0);
	}

////    DEBUG
//	puts("BAS:"); print32(*(u32*)0x0C003FEC); puts("\n");
//	puts("RCA:"); print32(*(u32*)0x0C003FF8); puts("\n");
//	puts("BLS:"); print32(*(u32*)0x0C003FFC); puts("\n");
//	puts("READ print\n");
//	print32(*(u32*)(RD_MEM_ADDR+0x0));
//	print32(*(u32*)(RD_MEM_ADDR+0x4));
//	print32(*(u32*)(RD_MEM_ADDR+0x8));
//	print32(*(u32*)(RD_MEM_ADDR+0xc));
//	print32(*(u32*)(RD_MEM_ADDR+0x10));
//	puts("--END\n");

	puts("Read SDMMC");print8(CHANNEL);
	if(SDHC) puts(" sdhc [");
	else     puts(" sd [");
	print32(sd_sectors);
	if(flag) puts("] uboot success\n");
	else     puts("] uboot fail\n");
	led_set(1);

	// jump to bootloader running
	board = ((ulong (*)(int))RD_MEM_ADDR)(0);

	// no run here
	while(1) {
		led_set(0); udelay(0x100000);
		led_set(1);udelay(0x100000);
	}

#else
	if (!is_jtag) {
		/*
		* We got the first 8KBytes of the bootloader pulled into the
		* steppingstone SRAM for free.  Now we pull the whole bootloader
		* image into SDRAM.
		*
		* This code and the .S files are arranged by the linker script
		* to expect to run from 0x0.  But the linker script has told
		* everything else to expect to run from 0x53000000+.  That's
		* why we are going to be able to copy this code and not have it
		* crash when we run it from there.
		*/

		/* We randomly pull 32KBytes of bootloader */

		extern unsigned int s3c6410_mmc_init(int verbose);
		unsigned long s3c6410_mmc_bread(int dev_num,
				unsigned long start_blk, unsigned long blknum,
								     void *dst);
		sd_sectors = s3c6410_mmc_init(1);
		s3c6410_mmc_bread(0, sd_sectors - 1026 - 16 - (256 * 2),
						     32 * 2, (u8 *)0x53000000);
	}

	/* all of Qi is in memory now, stuff outside steppingstone too */


	if (this_board->port_init)
		this_board->port_init();

	puts("\n     Detected: ");
	puts(this_board->name);
	puts(", ");
	puts((this_board->get_board_variant)()->name);
	puts("\n");

	/*
	 * jump to bootloader_second_phase() running from DRAM copy
	 */
	bootloader_second_phase();
#endif
}

#ifdef HHTECH_MINIPMP

#define INAND_KERNEL0_BEND    ((8<<20)/NANDBLKSIZE)   // 8M
#define INAND_KERNEL1_BEND    ((16<<20)/NANDBLKSIZE)  // 16M
static int do_read_inand(int ch, unsigned long offset_end_blk)
{
    unsigned long start = 0, cnt = 0;
    int ret = 0;

    FWFileHdr *fh = (FWFileHdr*)RD_MEM_ADDR;
    start = globalBlockSizeHide - offset_end_blk;

    ret = CopyMMCtoMem(ch, start, 2, (u32*)fh, 0);
    if(fh->magic != HEAD_MAGIC) {
	puts("Error magic "); print32(fh->magic);puts("\n");
	return -1;
    }

    start += fh->u_boot.nand.offset;
    cnt   = (fh->u_boot.nand.size / NANDBLKSIZE) + 1;


    return CopyMMCtoMem(ch, start, cnt, (u32*)fh, 0);
}

int do_load_uboot(void)
{
    int ret, channel = CHANNEL;

    ret = do_read_inand(channel, INAND_KERNEL1_BEND);

    if(ret != 1) ret = do_read_inand(channel, INAND_KERNEL0_BEND);

    return ret;
}

// led is contrl by GPN12, low is ON
void led_set(int on)
{
    unsigned long val;

    val = GPNCON_REG;
    if(!on) val |= (0x01<<(12*2));
    else   val &= ~(0x03<<(12*2));
    GPNCON_REG = val;

    val = GPNDAT_REG;
    if(!on) val |= (0x01<<12);
    else    val &= ~(0x01<<12);
    GPNDAT_REG = val;
}
#endif
