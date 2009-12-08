/*
 * (C) Copyright 2007 OpenMoko, Inc.
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

#define __ASM_MODE__
#define __ASSEMBLY__

#include <s3c6410.h>

#define TEXT_BASE 0x53000000

#ifndef HHTECH_MINIPMP
#define S3C6410_POP_A 1
#endif

#define PLL_ENABLE	(1  << 31)
#define SET_PLL(mdiv, pdiv, sdiv)	((mdiv)<<16|(pdiv)<<8|(sdiv))
#define SET_PLL_ENABLED(m, p, s)	(PLL_ENABLE|SET_PLL(m, p, s))

#undef  CONFIG_SYNC_MODE /* ASYNC MODE */
/* the smartQ DOES NOT WORK at APLL = MPLL = 666MHz */
/* this appears to be a Samsung errata lightly disclosed in:  */
/* "6410X Power Design Giode Preliminary Rev 0.9A" */
/* "October 7, 2009 */
/* Pick 1 of these clocks in config.mk NOT here */
// #define CONFIG_CLK_800_166_55
// #define CONFIG_CLK_800_133_66
// #define CONFIG_CLK_666_166_66
// #define CONFIG_CLK_666_166_66_SYNC
// #define CONFIG_CLK_666_133_66
// #define CONFIG_CLK_532_133_66

#if defined(CONFIG_CLK_800_166_55) /* FIN 12MHz, Fout 800MHz */

#define APLLmdiv	400
#define MPLLmdiv	333

#define PCLKdiv		6  /* 333 / 6 = 55.5 */

#elif defined(CONFIG_CLK_800_133_66) /* FIN 12MHz, Fout 800MHz */

#define APLLmdiv       400

#elif defined(CONFIG_CLK_666_166_55) /* FIN 12MHz, Fout 666MHz */

#define APLLmdiv	333
#define MPLLmdiv	333

#define	PCLKdiv		6  /* 333 / 6 = 55.5MHz */

#elif defined(CONFIG_CLK_666_166_55_SYNC) /* FIN 12MHz, Fout 666MHz */

#define APLLmdiv       333

#define	PCLKdiv		6  /* 333 / 6 = 55.5MHz */

#define  CONFIG_SYNC_MODE 

#elif defined(CONFIG_CLK_666_133_66) /* FIN 12MHz, Fout 666MHz */

#define APLLmdiv       333

#elif defined(CONFIG_CLK_532_133_66) /* FIN 12MHz, Fout 532MHz */

/* this is the default case */

#else
#error Define SPEED in config.mk
#endif

/* input clock of PLL */
#define CONFIG_SYS_CLK_FREQ     12000000  /* the SmartQ has 12MHz input clock */

/* true divisors (ie., actual div not div+1) */
/* defaults are 532/532/266/133/66 */
#ifndef APLLmdiv
#define APLLmdiv       266
#endif

#ifndef APLLpdiv
#define APLLpdiv       3
#endif

#ifndef APLLsdiv
#define APLLsdiv       1
#endif

#ifndef MPLLmdiv
#define MPLLmdiv       266
#endif

#ifndef MPLLpdiv
#define MPLLpdiv       3
#endif

#ifndef MPLLsdiv
#define MPLLsdiv	1
#endif

/* User Manual 3-26 says PCLKdiv needs to be even divisor (2,4,6,...) */
#ifndef PCLKdiv
#define	PCLKdiv		4  /* normally 266 / 4 = 66.5MHz */
#endif

#ifndef APLLdiv
#define APLLdiv         1
#endif

#ifndef MPLLdiv
#define MPLLdiv		2
#endif

#ifndef HCLKdiv
#define HCLKdiv		2
#endif

#ifndef HCLKX2div
#define HCLKX2div      	2
#endif

/* CLKUART(=100Mhz) = CLKUART_input(800/2=400Mhz) / (UART_RATIO(3)+1) */
/* CLKUART(=66.5Mhz) = CLKUART_input(532/2=266Mhz) / (UART_RATIO(3)+1) */
/* CLKUART(=50Mhz) = CLKUART_input(400/2=200Mhz) / (UART_RATIO(3)+1) */
/* Now, When you use UART CLK SRC by EXT_UCLK1, We support 532MHz & 400MHz value */
#define UARTmask	7
#define UARTsft		16

#ifndef UARTdiv
#define UARTdiv		4  /* this is may divider not UBRDIV */
#endif

#define Fapll	(CONFIG_SYS_CLK_FREQ/(APLLpdiv<<APLLsdiv)*APLLmdiv)
#define Fmpll	(CONFIG_SYS_CLK_FREQ/(MPLLpdiv<<MPLLsdiv)*MPLLmdiv)

#if defined(CONFIG_SYNC_MODE)
#define Fhclk	((Fapll/HCLKX2div)/HCLKdiv)
#else
#define Fhclk	((Fmpll/HCLKX2div)/HCLKdiv)
#endif

#define BAUD	115200
#define Fuart	((Fmpll/2)/UARTdiv) /* the div2 is the DIVmpll divider */
#define URBDIV	(((Fuart/BAUD)/16) - 1)

/* convert tXXX to memory clks */
#define nSecToClk(nS)  (((Fhclk / 1000 * (nS)) - 1) / 1000000 + 1)

/* selects whether HCLK/PCLK are derived from APLL (=1) or MPLL (=0) */
#define DMC_OTHERS_SYNCACK      (0xf << 8) /* read only acknowledge of sync */
#define DMC_OTHERS_SYNCMUXSEL	(1<<6) 
/* sync=1 async=0 */
#define DMC_OTHERS_SYNCMODE     (1<<7)  /* sync mode request to ARM */
#define DMC_OTHERS_SYNCVICPORT  (1<<5)  /* Nice: undocument bits */

#define CLK_DIV(p, hx2, h, mp, ap)  \
		((p) << 12 | (hx2) << 9 | (h) << 8 | (mp) << 4 | (ap) << 0)

#define CLK_DIV_VAL	CLK_DIV(PCLKdiv-1, HCLKX2div-1, HCLKdiv-1, MPLLdiv-1, APLLdiv-1)

#define APLL_VAL	SET_PLL_ENABLED(APLLmdiv, APLLpdiv, APLLsdiv)
#define MPLL_VAL	SET_PLL_ENABLED(MPLLmdiv, MPLLpdiv, MPLLsdiv)

#define EPLLmdiv	32

#define EPLL_24MHZ
#ifdef EPLL_24MHZ
/* this is 24MHz for an Fin of 12MHz */
#define EPLLpdiv	2
#define EPLLsdiv	3
#else
/* this is for 192MHz for Fin of 12MHz */
#define EPLLpdiv	1
#define EPLLsdiv	1
#endif

#define EPLL_VAL 	SET_PLL_ENABLED(EPLLmdiv, EPLLpdiv, EPLLsdiv)

/* These are the fields for the DMC CFG register */
/* device column pins */
#define CFG_COL_11		(0)
#define CFG_COL_12		(1)
#define CFG_COL_13		(2)
#define CFG_COL_14		(3)

/* device row pins */
#define CFG_ROW_8		(0)
#define CFG_ROW_9		(1)
#define CFG_ROW_10		(2)
#define CFG_ROW_11		(3)
#define CFG_ROW_12		(4)

/* CFG burst length */
#define CFG_BL_4		(0x2)
#define CFG_BL_8		(0x3)

/* These are the fields for the DMC CFG2 register */
#define CFG2_RD_DELAY_SDR	(0x0 << 11)
#define CFG2_RD_DELAY_LPDDR	(0x1 << 11)
#define CFG2_RD_DELAY_DDR	(0x2 << 11)

#define CFG2_TYPE_SDR		(0x0 << 8 )
#define CFG2_TYPE_DDR		(0x1 << 8 )
#define CFG2_TYPE_LPDDR		(0x3 << 8 )

#define CFG2_WIDTH_16		(0x0 << 6 )
#define CFG2_WIDTH_32		(0x1 << 6 )

/* common JEDEC commands */
#define CMD_PRE_CHARGE		(0x0 << 18)
#define CMD_AUTO		(0x1 << 18)
#define CMD_MODE		(0x2 << 18)  /* aka MR  */
#define CMD_EXTMODE		(0x3 << 18)  /* aka EMR */

/* common (?) RAM command states */
#define MEMC_CMD_GO		(0)
#define MEMC_CMD_SLEEP		(1)
#define MEMC_CMD_WAKEUP		(2)
#define MEMC_CMD_PAUSE		(3)
#define MEMC_CMD_CONFIG		(4)


#if S3C6410_POP_A
#error NOT SMARTQ!

#define DMC1_MEM_CFG		0x00210011	/* Supports one CKE control, Chip1, Burst4, Row/Column bit */
#define DMC1_MEM_CFG2		0xB41
#define DMC1_CHIP0_CFG		0x150FC
#define DMC1_CHIP1_CFG		0x154FC

/* Memory Parameters */
/* DDR Parameters */
#define RAM_tREFRESH		5865		/* ns */
#define RAM_tRAS		50		/* ns (min: 45ns)*/
#define RAM_tRC 		68		/* ns (min: 67.5ns)*/
#define RAM_tRCD		23		/* ns (min: 22.5ns)*/
#define RAM_tRFC		133		/* ns (min: 80ns)*/
#define RAM_tRP 		23		/* ns (min: 22.5ns)*/
#define RAM_tRRD		20		/* ns (min: 15ns)*/
#define RAM_tWR 		20		/* ns (min: 15ns)*/
#define RAM_tXSR		125		/* ns (min: 120ns)*/

#define RAM_clk_tCAS		3		/* CAS Latency 3 */

#else

/* These numbers are for the Qimonda HYB25DC512160 */

#define CFG_COLS             	(CFG_COL_13 <<  0)      /* for 512Mbit */

#define CFG_ROWS		(CFG_ROW_10 <<  3)      /* for 512Mbit */

#define BURST			(CFG_BL_8)

#define CFG_AUTOPOWER		(0x1  << 13)
#define CFG_PD_CYCLES		(32 <<  7)     /* clocks before PD */
#define CFG_BURST_LEN		(BURST << 15)      

// was: #define DMC1_MEM_CFG		0x00010012
#define DMC1_MEM_CFG		\
       (CFG_BURST_LEN|CFG_AUTOPOWER|CFG_PD_CYCLES|CFG_ROWS|CFG_COLS) 

#define CFG2_RD_DELAY		(CFG2_RD_DELAY_DDR)

#define CFG2_TYPE		(CFG2_TYPE_DDR)

#define CFG2_WIDTH		(CFG2_WIDTH_32)

#define CFG2_DQM_INIT		(0x1 << 2 )
#define CFG2_SYNC		(0x1 << 0 )

// was: #define DMC1_MEM_CFG2		0xB45
#define DMC1_MEM_CFG2		\
		(CFG2_RD_DELAY|CFG2_TYPE|CFG2_WIDTH|CFG2_DQM_INIT|CFG2_SYNC)

#define BA_ROW_COL		(0x1  << 16)
#define ADDR_MATCH		(0x50 <<  8)
#define ADDR_MASK               (0xf8 <<  0)

// #define DMC1_CHIP0_CFG		0x150F8
#define DMC1_CHIP0_CFG          (BA_ROW_COL|ADDR_MATCH|ADDR_MASK)
#define DMC_DDR_32_CFG		0x0 		/* 32bit, DDR */

/* Memory Parameters */
/* DDR Parameters */

/* FYI: For HCLK=133MHz, clk=7.5188nS */

/* These numbers are for the Qimonda HYB25DC512160 */

#define RAM_tREFRESH		7800			/* ns */
#define RAM_tRAS		42			/* ns */
#define RAM_tRC 		60			/* ns */
#define RAM_tRCD		18			/* ns */
#define RAM_tRFC		72			/* ns */
#define RAM_tRP 		18			/* ns */
#define RAM_tRRD		12			/* ns */
#define RAM_tWR 		15			/* ns */
#define RAM_tXSR		200			/* ns */

/* CAS latency in clks */
#define RAM_clk_tCAS		3			/* clk */

#define MODE_BL			(BURST << 0)
#define MODE_CAS		(RAM_clk_tCAS << 4)

#define DRAM_MODE		(CMD_MODE | MODE_BL | MODE_CAS)  /* ModeReg */

#endif

#define RAM_clk_tREFRESH	nSecToClk(RAM_tREFRESH)	/* clk */
#define RAM_clk_tRAS		nSecToClk(RAM_tRAS)	/* clk */
#define RAM_clk_tRC		nSecToClk(RAM_tRC)	/* clk */
#define RAM_clk_tRCD		nSecToClk(RAM_tRCD)	/* clk */
#define RAM_clk_tRFC		nSecToClk(RAM_tRFC)	/* clk */
#define RAM_clk_tRP 		nSecToClk(RAM_tRP)	/* clk */
#define RAM_clk_tRRD		nSecToClk(RAM_tRRD)	/* clk */
#define RAM_clk_tWR 		nSecToClk(RAM_tWR)	/* clk */
#define RAM_clk_tXSR		nSecToClk(RAM_tXSR)	/* clk */


/*
 * mDDR memory configuration
 */
#define DMC_DDR_BA_EMRS 	2
#define DMC_DDR_CAS_LATENCY	(RAM_clk_tCAS << 1)	//  Set Cas Latency
#define RAM_clk_tDQSS		1		// Min 0.75 ~ 1.25
#define RAM_clk_tMRD		2		//Min 2 tck

#define DMC_DDR_schedule_RCD	((RAM_clk_tRCD - 3) << 3)
#define DMC_DDR_RCD		(DMC_DDR_schedule_RCD | RAM_clk_tRCD)

#define DMC_DDR_schedule_RFC	((RAM_clk_tRFC - 3) << 5)
#define DMC_DDR_RFC		(DMC_DDR_schedule_RFC | RAM_clk_tRFC)

#define DMC_DDR_schedule_RP	((RAM_clk_tRP - 3) << 3)
#define DMC_DDR_RP		(DMC_DDR_schedule_RP | RAM_clk_tRP)
#define RAM_clk_tWTR		2
#define RAM_clk_tXP		2	//1tck + tIS(1.5ns)
#define RAM_clk_tESR		RAM_clk_tXSR


.globl _start, processor_id, is_jtag

_start:	b       start_code
/* if we are injected by JTAG, the script sets _istag content to nonzero */
is_jtag:
	.word	0

/* it's at a fixed address (+0x8) so we can breakpoint it in the JTAG script
 * we need to go through this hassle because before this moment, SDRAM is not
 * working so we can't prep it from JTAG
 */

_steppingstone_done:
	ldr	pc, _start_armboot

_start_armboot:
	.word 	start_qi

_TEXT_BASE:
	.word	TEXT_BASE

/*
 * These are defined in the board-specific linker script.
 */
.globl _bss_start
_bss_start:
	.word __bss_start

.globl _bss_end
_bss_end:
	.word _end

/*
 * we have a stack in steppingstone because we can want to run full memory
 * memory tests
 */

	.fill   128
.globl _ss_stack
_ss_stack:

start_code:
	/*
	 * set the cpu to SVC32 mode
	 */
	mrs	r0,cpsr
	bic	r0,r0,#0x1f
	orr	r0,r0,#0xd3
	msr	cpsr,r0

	/*
	 * flush v4 I/D caches
	 */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c7, 0	/* flush v3/v4 cache */
	mcr	p15, 0, r0, c8, c7, 0	/* flush v4 TLB */

	/*
	 * disable MMU stuff and caches
	 */
	mrc	p15, 0, r0, c1, c0, 0
	bic	r0, r0, #0x00002300	@ clear bits 13, 9:8 (--V- --RS)
	bic	r0, r0, #0x00000087	@ clear bits 7, 2:0 (B--- -CAM)
	orr	r0, r0, #0x00000002	@ set bit 2 (A) Align
	orr	r0, r0, #0x00005000	@ Enable I and D-Cache
	mcr	p15, 0, r0, c1, c0, 0

	/* Peri port setup */
	ldr	r0, =0x70000000
	orr	r0, r0, #0x13
	mcr	p15,0,r0,c15,c2,4       @ 256M(0x70000000-0x7fffffff)

	/* SDRAM */

	ldr	r0, =ELFIN_MEM_SYS_CFG			@Memory sussystem address 0x7e00f120
	mov	r1, #0xd				@ Xm0CSn2 = NFCON CS0, Xm0CSn3 = NFCON CS1
	str	r1, [r0]

	ldr	r0, =ELFIN_DMC1_BASE			@DMC1 base address 0x7e001000

	ldr	r1, =MEMC_CMD_CONFIG
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

	ldr	r1, =RAM_clk_tREFRESH
	str	r1, [r0, #INDEX_DMC_REFRESH_PRD]

	ldr	r1, =DMC_DDR_CAS_LATENCY
	str	r1, [r0, #INDEX_DMC_CAS_LATENCY]

	ldr	r1, =RAM_clk_tDQSS
	str	r1, [r0, #INDEX_DMC_T_DQSS]

	ldr	r1, =RAM_clk_tMRD
	str	r1, [r0, #INDEX_DMC_T_MRD]

	ldr	r1, =RAM_clk_tRAS
	str	r1, [r0, #INDEX_DMC_T_RAS]

	ldr	r1, =RAM_clk_tRC
	str	r1, [r0, #INDEX_DMC_T_RC]

	ldr	r1, =DMC_DDR_RCD
	str	r1, [r0, #INDEX_DMC_T_RCD]

	ldr	r1, =DMC_DDR_RFC
	str	r1, [r0, #INDEX_DMC_T_RFC]

	ldr	r1, =DMC_DDR_RP
	str	r1, [r0, #INDEX_DMC_T_RP]

	ldr	r1, =RAM_clk_tRRD
	str	r1, [r0, #INDEX_DMC_T_RRD]

	ldr	r1, =RAM_clk_tWR
	str	r1, [r0, #INDEX_DMC_T_WR]

	ldr	r1, =RAM_clk_tWTR
	str	r1, [r0, #INDEX_DMC_T_WTR]

	ldr	r1, =RAM_clk_tXP
	str	r1, [r0, #INDEX_DMC_T_XP]

	ldr	r1, =RAM_clk_tXSR
	str	r1, [r0, #INDEX_DMC_T_XSR]

	ldr	r1, =RAM_clk_tESR
	str	r1, [r0, #INDEX_DMC_T_ESR]

	ldr	r1, =DMC1_MEM_CFG
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG]

	ldr	r1, =DMC1_MEM_CFG2
	str	r1, [r0, #INDEX_DMC_MEMORY_CFG2]

	ldr	r1, =DMC1_CHIP0_CFG
	str	r1, [r0, #INDEX_DMC_CHIP_0_CFG]

	ldr	r1, =DMC_DDR_32_CFG
	str	r1, [r0, #INDEX_DMC_USER_CONFIG]

	@DMC0 DDR Chip 0 configuration direct command reg
	ldr	r1, =DMC_NOP0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Precharge All
	ldr	r1, =DMC_PA0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Auto Refresh	2 times
	ldr	r1, =DMC_AR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

#ifdef DMC_mDDR_EMR0
	@MRS
	ldr	r1, =DMC_mDDR_EMR0
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
#endif

	@Mode Reg
	ldr	r1, =DRAM_MODE
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

#if S3C6410_POP_A
	ldr	r1, =DMC1_CHIP1_CFG
	str	r1, [r0, #INDEX_DMC_CHIP_1_CFG]

	@DMC0 DDR Chip 0 configuration direct command reg
	ldr	r1, =DMC_NOP1
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Precharge All
	ldr	r1, =DMC_PA1
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Auto Refresh	2 time
	ldr	r1, =DMC_AR1
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@MRS
	ldr	r1, =DMC_mDDR_EMR1
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]

	@Mode Reg
	ldr	r1, =DMC_mDDR_MR1
	str	r1, [r0, #INDEX_DMC_DIRECT_CMD]
#endif

	@Enable DMC1
	ldr	r1, =MEMC_CMD_GO
	str	r1, [r0, #INDEX_DMC_MEMC_CMD]

1:
	ldr	r1, [r0, #INDEX_DMC_MEMC_STATUS]
	mov	r2, #0x3
	and	r1, r1, r2
	cmp	r1, #0x1
	bne	1b
	nop

	ldr	r0, =ELFIN_CLOCK_POWER_BASE	@0x7e00f000

#ifdef  CONFIG_SYNC_MODE
	ldr	r1, [r0, #OTHERS_OFFSET]
	mov	r2, #DMC_OTHERS_SYNCMUXSEL
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

	nop
	nop
	nop
	nop
	nop

	mov	r2, #DMC_OTHERS_SYNCMODE
	orr	r1, r1, r2
	str	r1, [r0, #OTHERS_OFFSET]

check_sync_ack:
	ldr	r1, [r0, #OTHERS_OFFSET]
	mov	r2, #DMC_OTHERS_SYNCACK
	and	r1, r1, r2
	cmp	r1, #DMC_OTHERS_SYNCACK
	bne	check_sync_ack

#else /* ASYNC mode */

        nop
        nop
        nop
        nop
        nop

        ldr     r1, [r0, #OTHERS_OFFSET]
        bic     r1, r1, #(DMC_OTHERS_SYNCMODE|DMC_OTHERS_SYNCMUXSEL)
        orr     r1, r1, #DMC_OTHERS_SYNCMUXSEL
        str     r1, [r0, #OTHERS_OFFSET]

wait_for_async:
        ldr     r1, [r0, #OTHERS_OFFSET]
        and     r1, r1, #DMC_OTHERS_SYNCACK
        cmp     r1, #0x0
        bne     wait_for_async

        ldr     r1, [r0, #OTHERS_OFFSET]
        bic     r1, r1, #DMC_OTHERS_SYNCMUXSEL
        str     r1, [r0, #OTHERS_OFFSET]
#endif

	mov   r1, #0xff00
	orr   r1, r1, #0xff
	str   r1, [r0, #APLL_LOCK_OFFSET]
	str   r1, [r0, #MPLL_LOCK_OFFSET]
	str   r1, [r0, #EPLL_LOCK_OFFSET]


	/* put primary uart divisor in place */
	ldr   	r1, [r0, #CLK_DIV2_OFFSET]
	bic	r1, r1, #UARTmask<<UARTsft
	orr	r1, r1, #(UARTdiv - 1)<<UARTsft
	str	r1, [r0, #CLK_DIV2_OFFSET]


	ldr   	r1, [r0, #CLK_DIV0_OFFSET]	/*Set Clock Divider*/
	bic	r1, r1, #0x30000
	bic	r1, r1, #0xff00
	bic	r1, r1, #0xff
	ldr	r2, =CLK_DIV_VAL
	orr	r1, r1, r2
	str	r1, [r0, #CLK_DIV0_OFFSET]

	ldr   r1, =APLL_VAL
	str   r1, [r0, #APLL_CON_OFFSET]
	ldr   r1, =MPLL_VAL
	str   r1, [r0, #MPLL_CON_OFFSET]
	ldr   r1, =EPLL_VAL
	str   r1, [r0, #EPLL_CON0_OFFSET]
	mov   r1, #0
	str   r1, [r0, #EPLL_CON1_OFFSET]

	ldr	r1, [r0, #CLK_SRC_OFFSET]	/* APLL, MPLL, EPLL select to Fout */

	ldr	r2, =0x2007
	orr	r1, r1, r2

	str	r1, [r0, #CLK_SRC_OFFSET]

	/* wait at least 200us to stablize all clock */
	mov	r1, #0x10000
ppl_wait:	
	subs	r1, r1, #1
	bne	ppl_wait

#ifdef CONFIG_SYNC_MODE                    /* Synchronization for VIC port */

	ldr	r1, [r0, #OTHERS_OFFSET]
	orr	r1, r1, #DMC_OTHERS_SYNCVICPORT
	str	r1, [r0, #OTHERS_OFFSET]
#else
        ldr     r1, [r0, #OTHERS_OFFSET]
        bic     r1, r1, #DMC_OTHERS_SYNCVICPORT
        str     r1, [r0, #OTHERS_OFFSET]

#endif


	/* set GPIO to enable UART */
	@ GPIO setting for UART
	ldr	r0, =ELFIN_GPIO_BASE
	ldr	r1, =0x2222
#ifdef HHTECH_MINIPMP
	str	r1, [r0, #GPACON_OFFSET]
	ldr	r0, =ELFIN_UART_BASE + ELFIN_UART0_OFFSET	@0x7F005c00 uart 0
#else
	str	r1, [r0, #GPBCON_OFFSET]
	ldr	r0, =ELFIN_UART_BASE + ELFIN_UART3_OFFSET	@0x7F005c00 uart 3
#endif
	mov	r1, #0x0
	str	r1, [r0, #UFCON_OFFSET]
	str	r1, [r0, #UMCON_OFFSET]

	mov	r1, #0x3                	@was 0.
	str	r1, [r0, #ULCON_OFFSET]

	ldr	r1, =0xe45		/* UARTCLK SRC = 11 => EXT_UCLK1*/

	str	r1, [r0, #UCON_OFFSET]

	ldr	r1, =URBDIV
	str	r1, [r0, #UBRDIV_OFFSET]

	ldr	r1, =0x1FFF
	str	r1, [r0, #UDIVSLOT_OFFSET]

	/* resuming? */

	ldr     r0, =(ELFIN_CLOCK_POWER_BASE+RST_STAT_OFFSET)
	ldr     r1, [r0]
	bic     r1, r1, #0xfffffff7
	cmp     r1, #0x8
	beq     wakeup_reset

	/* no, cold boot */

	ldr	r0, =ELFIN_UART_BASE + ELFIN_UART3_OFFSET
	ldr	r1, =0x55
	str	r1, [r0, #UTXH_OFFSET]		@'U'
								/* >> CFG_VIDEO_LOGO_MAX_SIZE */
#define CFG_GBL_DATA_SIZE		128			/* size in bytes reserved for initial data */


	ldr	r0, _TEXT_BASE		/* upper 128 KiB: relocated uboot   */
	sub	r0, r0, #CFG_GBL_DATA_SIZE 	/* bdinfo                        */
	sub	sp, r0, #12		/* leave 3 words for abort-stack    */
clear_bss:
	ldr	r0, _bss_start		/* find start of bss segment        */
	ldr	r1, _bss_end		/* stop here                        */
	mov 	r2, #0x00000000	/* clear                            */

clbss_l:
	str	r2, [r0]			/* clear loop...                    */
	add	r0, r0, #4
	cmp	r0, r1
	ble	clbss_l

	b	_steppingstone_done

	/* resume */

wakeup_reset:

	ldr	r0, =ELFIN_UART_BASE + ELFIN_UART3_OFFSET
	ldr	r1, =0x4b4b4b4b
	str	r1, [r0, #UTXH_OFFSET]

	/*Clear wakeup status register*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+WAKEUP_STAT_OFFSET)
	ldr	r1, [r0]
	str	r1, [r0]

#if 0
		/*LED test*/
		ldr     r0, =ELFIN_GPIO_BASE
		ldr     r1, =0x3000
		str     r1, [r0, #GPNDAT_OFFSET]
#endif

	/*Load return address and jump to kernel*/
	ldr	r0, =(ELFIN_CLOCK_POWER_BASE+INF_REG0_OFFSET)
	ldr	r1, [r0]	/* r1 = physical address of s3c6400_cpu_resume function*/
	mov	pc, r1		/*Jump to kernel (sleep-s3c6400.S)*/
	nop
	nop

4:
	b 4b
