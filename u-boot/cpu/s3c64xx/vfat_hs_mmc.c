#include <common.h>

#if (defined(CONFIG_SMDK6400) || defined(CONFIG_SMDK6410)) && defined(CONFIG_SUPPORT_VFAT)

#define CONFIG_SUPPORT_MMC_PLUS 1
#define HCLK_OPERATION

#undef DEBUG_HSMMC
#ifdef DEBUG_HSMMC
#define dbg(x...)       printf(x)
#else
#define dbg(x...)       do { } while (0)
#endif

#if 0
  #define HSMMC_DPRINT(x...) printf(x)
#else
  #define HSMMC_DPRINT(x...) do { } while(0)
#endif

#include <regs.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/protocol.h>
#include <asm/io.h>
#include "hs_mmc.h"

#define SD_BLOCK_SIZE           (512)

inline u32 UNSTUFF_BITS(u32 *resp, int start, int size)
{
    const int __size = size; 
    const u32 __mask = (__size < 32 ? 1 << __size : 0) - 1; 
    const int __off = ((start) / 32); 
    const int __shft = (start) % 32; 
    u32 __res; 
    __res = resp[__off] >> __shft; 
    if (__size + __shft > 32)
	__res |= resp[__off+1] << (32 - __shft); 

    __res &= __mask; 

    return __res;
}

static const unsigned int tran_exp[] = {
    10000,      100000,     1000000,    10000000,
    0,      0,      0,      0
};

static const unsigned char tran_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

static const unsigned int tacc_exp[] = {
    1,  10, 100,    1000,   10000,  100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

typedef struct mmc_cid {
    unsigned int        manfid;
    char                prod_name[8];
    unsigned int        serial;
    unsigned short      oemid;
    unsigned short      year;
    unsigned char       hwrev;
    unsigned char       fwrev;
    unsigned char       month;
}mmc_cid_t;

typedef struct mmc_csd {
    unsigned char       mmca_vsn;
    unsigned short      cmdclass;
    unsigned short      tacc_clks;
    unsigned int        tacc_ns;
    unsigned int        r2w_factor;
    unsigned int        max_dtr;
    unsigned int        read_blkbits;
    unsigned int        write_blkbits;
    unsigned int        capacity;
    unsigned int        read_partial:1,
			read_misalign:1,
			write_partial:1,
			write_misalign:1;
}mmc_csd_t;
#define MOVLEFT_CID_CSD(v)                                                 \
    do {                                                                   \
	(v)[3] = ((((v)[3] << 8) & 0xFFFFFF00) | (((v)[2] >> 24) & 0xFF)); \
	(v)[2] = ((((v)[2] << 8) & 0xFFFFFF00) | (((v)[1] >> 24) & 0xFF)); \
	(v)[1] = ((((v)[1] << 8) & 0xFFFFFF00) | (((v)[0] >> 24) & 0xFF)); \
	(v)[0] = ((((v)[0] << 8) & 0xFFFFFF00));                           \
    } while(0)

#include <part.h>
static block_dev_desc_t mmc_dev;
static int block_addr = 0;

static int sd_if_cond(ulong ocr);
static int mmc_decode_csd(u32 *raw_csd, mmc_csd_t *csd);

#if defined(CONFIG_S3C6400)
extern ulong virt_to_phy_smdk6400(ulong addr);
#elif defined(CONFIG_S3C6410)
extern ulong virt_to_phy_smdk6410(ulong addr);
#endif

#define SDI_Tx_buffer_HSMMC		(0x51000000)
#define SDI_Rx_buffer_HSMMC		(0x51000000+(0x300000))
#define SDI_Compare_buffer_HSMMC	(0x51000000+(0x600000))

/*  Global variables */
static uint ocr_check = 0;
static uint mmc_card = 0;
static uint rca = 0;
static uint mmc_spec, sd_spec;

static ulong HCLK;
static uint card_mid = 0;

static int dev_select = -1;

/* extern functions */
extern ulong get_HCLK(void);

#define s3c_hsmmc_readl(x)	readl((ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))
#define s3c_hsmmc_readw(x)	readw((ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))
#define s3c_hsmmc_readb(x)	readb((ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))

#define s3c_hsmmc_writel(v,x)	writel((v),(ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))
#define s3c_hsmmc_writew(v,x)	writew((v),(ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))
#define s3c_hsmmc_writeb(v,x)	writeb((v),(ELFIN_HSMMC_BASE + (dev_select * 0x100000)) + (x))

static int wait_for_r_buf_ready (void)
{
	uint uLoop = 0;

	while (!(s3c_hsmmc_readw(HM_NORINTSTS) & 0x20)) {
		if (uLoop % 500000 == 0 && uLoop > 0) {
			return 0;
		}
		uLoop++;
	}
	return 1;
}

ushort  _last_cmd = 0;
static int wait_for_cmd_done (void)
{
	uint i;
	ushort n_int, e_int;

	dbg("wait_for_cmd_done\n");
	for (i=0; i<0x20000000; i++) {
		n_int = s3c_hsmmc_readw(HM_NORINTSTS);
		dbg("  HM_NORINTSTS: %04x\n", n_int);
		if (n_int & 0x8000) break;
		if (n_int & 0x0001) return 0;
	}

	e_int = s3c_hsmmc_readw(HM_ERRINTSTS);
	s3c_hsmmc_writew(e_int, HM_ERRINTSTS);
	s3c_hsmmc_writew(n_int, HM_NORINTSTS);
	dbg("cmd error1: 0x%04x, HM_NORINTSTS: 0x%04x\n", e_int, n_int);
	HSMMC_DPRINT("cmd(%d) error1: 0x%04x, HM_NORINTSTS: 0x%04x\n", _last_cmd, e_int, n_int);
	return -1;
}

/* XXX: must modify algorithm. it has bugs. by scsuh */
static int wait_for_data_done (void)
{
	while (!(s3c_hsmmc_readw(HM_NORINTSTS) & 0x2))
		return 1;
	return 1;
}

static void ClearCommandCompleteStatus(void)
{
	s3c_hsmmc_writew(1 << 0, HM_NORINTSTS);
	while (s3c_hsmmc_readw(HM_NORINTSTS) & 0x1) {
		s3c_hsmmc_writew(1 << 0, HM_NORINTSTS);
	}
}

static void ClearTransferCompleteStatus(void)
{
	s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 1), HM_NORINTSTS);
	while (s3c_hsmmc_readw(HM_NORINTSTS) & 0x2) {
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 1), HM_NORINTSTS);
	}
}

#if 0
static void ClearBufferWriteReadyStatus(void)
{
	rHM_NORINTSTS |= (1 << 4);
	while (rHM_NORINTSTS & 0x10)
		rHM_NORINTSTS |= (1 << 4);
}
#endif

static void ClearBufferReadReadyStatus(void)
{
	s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 5), HM_NORINTSTS);
	while (s3c_hsmmc_readw(HM_NORINTSTS) & 0x20)
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 5), HM_NORINTSTS);
}

static void card_irq_enable(ushort temp)
{
	s3c_hsmmc_writew((s3c_hsmmc_readw(HM_NORINTSTSEN) & 0xFEFF) | (temp << 8), HM_NORINTSTSEN);
}

void hsmmc_reset (void)
{
	s3c_hsmmc_writeb(0x3, HM_SWRST);
}

static void hsmmc_set_gpio (int dev)
{
	u32 reg;

    if(0 == dev) {
	reg = readl(GPGCON) & 0xf0000000;
	writel(reg | 0x02222222, GPGCON);

	reg = readl(GPGPUD) & 0xfffff000;
	writel(reg, GPGPUD);
    }
    else if(1 == dev) {
	writel(0x00222222, GPHCON0);
	writel(0x00000000, GPHCON1);

	reg = readl(GPHPUD) & 0xfffff000;
	writel(reg, GPHPUD);
    }
}

static void set_transfer_mode_register (uint MultiBlk, uint DataDirection, uint AutoCmd12En, uint BlockCntEn, uint DmaEn)
{
	s3c_hsmmc_writew((s3c_hsmmc_readw(HM_TRNMOD) & ~(0xffff)) | (MultiBlk << 5) 
		| (DataDirection << 4) | (AutoCmd12En << 2) 
		| (BlockCntEn << 1) | (DmaEn << 0), HM_TRNMOD);
	dbg("\nHM_TRNMOD = 0x%04x\n", HM_TRNMOD);
}

static void set_arg_register (uint arg)
{
	s3c_hsmmc_writel(arg, HM_ARGUMENT);
}

static void set_blkcnt_register(ushort uBlkCnt)
{
	s3c_hsmmc_writew(uBlkCnt, HM_BLKCNT);
}

static void SetSystemAddressReg(uint SysAddr)
{
	s3c_hsmmc_writel(SysAddr, HM_SYSAD);
}

static void set_blksize_register(ushort uDmaBufBoundary, ushort uBlkSize)
{
	s3c_hsmmc_writew((uDmaBufBoundary << 12) | (uBlkSize), HM_BLKSIZE);
}

static void ClearErrInterruptStatus(void)
{
	while (s3c_hsmmc_readw(HM_NORINTSTS) & (0x1 << 15)) {
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS), HM_NORINTSTS);
		s3c_hsmmc_writew(s3c_hsmmc_readw(HM_ERRINTSTS), HM_ERRINTSTS);
	}
}

static void InterruptEnable(ushort NormalIntEn, ushort ErrorIntEn)
{
	ClearErrInterruptStatus();
	s3c_hsmmc_writew(NormalIntEn, HM_NORINTSTSEN);
	s3c_hsmmc_writew(ErrorIntEn, HM_ERRINTSTSEN);
}

static void hsmmc_clock_onoff (int on)
{
	u16 reg16;

	if (on == 0) {
		reg16 = s3c_hsmmc_readw(HM_CLKCON) & ~(0x1<<2);
		s3c_hsmmc_writew(reg16, HM_CLKCON);
	} else {
		reg16 = s3c_hsmmc_readw(HM_CLKCON);
		s3c_hsmmc_writew(reg16 | (0x1<<2), HM_CLKCON);

		while (1) {
			reg16 = s3c_hsmmc_readw(HM_CLKCON);
			if (reg16 & (0x1<<3))	/*  SD_CLKSRC is Stable */
				break;
		}
	}
}

static void set_clock (uint clksrc, uint div)
{
	u16 reg16;
	uint i;

#if defined(CONFIG_S3C6400)
	s3c_hsmmc_writel(0xC0000100 | (clksrc << 4), HM_CONTROL2);	// feedback control off
	s3c_hsmmc_writel(0x00000000, HM_CONTROL3);
#else
	s3c_hsmmc_writel(0xC0004100 | (clksrc << 4), HM_CONTROL2);	// rx feedback control
	s3c_hsmmc_writel(0x00008080, HM_CONTROL3); 			// Low clock: 00008080
	s3c_hsmmc_writel(0x3 << 16, HM_CONTROL4);
#endif

	s3c_hsmmc_writew(s3c_hsmmc_readw(HM_CLKCON) & ~(0xff << 8), HM_CLKCON);

	/* SDCLK Value Setting + Internal Clock Enable */
	s3c_hsmmc_writew(((div<<8) | 0x1), HM_CLKCON);

	/* CheckInternalClockStable */
	for (i=0; i<0x10000; i++) {
		reg16 = s3c_hsmmc_readw(HM_CLKCON);
		if (reg16 & 0x2)
			break;
	}
	if (i == 0x10000)
		printf("internal clock stabilization failed\n");

	dbg("HM_CONTROL2(0x80) = 0x%08x\n", s3c_hsmmc_readl(HM_CONTROL2));
	dbg("HM_CONTROL3(0x84) = 0x%08x\n", s3c_hsmmc_readl(HM_CONTROL3));
	dbg("HM_CLKCON  (0x2c) = 0x%04x\n", s3c_hsmmc_readw(HM_CLKCON));

	hsmmc_clock_onoff(1);
}

static void set_cmd_register (ushort cmd, uint data, uint flags)
{
	ushort val = (cmd << 8);
	_last_cmd = cmd;

	if (cmd == 12)
		val |= (3 << 6);

	if (flags & MMC_RSP_136)	/* Long RSP */
		val |= 0x01;
	else if (flags & MMC_RSP_BUSY)	/* R1B */
		val |= 0x03;
	else if (flags & MMC_RSP_PRESENT)	/* Normal RSP */
		val |= 0x02;

	if (flags & MMC_RSP_OPCODE)
		val |= (1<<4);

	if (flags & MMC_RSP_CRC)
		val |= (1<<3);

	if (data)
		val |= (1<<5);

	dbg("cmdreg =  0x%04x\n", val);
	s3c_hsmmc_writew(val, HM_CMDREG);
}

static int issue_command (ushort cmd, uint arg, uint data, uint flags)
{
	int i;

	dbg("### issue_command: %d, %08x, %d, %08x\n", cmd, arg, data, flags);
	/* Check CommandInhibit_CMD */
	for (i=0; i<0x1000000; i++) {
		if (!(s3c_hsmmc_readl(HM_PRNSTS) & 0x1))
			break;
	}
	if (i == 0x1000000) {
		printf("@@@@@@1 rHM_PRNSTS: %08lx\n", s3c_hsmmc_readl(HM_PRNSTS));
	}

	/* Check CommandInhibit_DAT */
	if (flags & MMC_RSP_BUSY) {
		for (i=0; i<0x1000000; i++) {
			if (!(s3c_hsmmc_readl(HM_PRNSTS) & 0x2))
				break;
		}
		if (i == 0x1000000) {
			printf("@@@@@@2 rHM_PRNSTS: %08lx\n", s3c_hsmmc_readl(HM_PRNSTS));
		}
	}

	s3c_hsmmc_writel(arg, HM_ARGUMENT);

	set_cmd_register(cmd, data, flags);

	if (wait_for_cmd_done())
		return 0;
	
	ClearCommandCompleteStatus();

	if (!(s3c_hsmmc_readw(HM_NORINTSTS) & 0x8000)) {
		return 1;
	} else {
		if (ocr_check == 1)
			return 0;
		else {
			printf("Command = %d, Error Stat = 0x%04x\n", (s3c_hsmmc_readw(HM_CMDREG) >> 8), s3c_hsmmc_readw(HM_ERRINTSTS));
			return 0;
		}
	}

}

static void set_mmc_speed (uint eSDSpeedMode)
{
	uint ucSpeedMode, arg;

	ucSpeedMode = (eSDSpeedMode == HIGH) ? 1 : 0;
	arg = (3 << 24) | (185 << 16) | (ucSpeedMode << 8);	/*  Change to the high-speed mode */
	while (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B));
}

static void set_sd_speed (uint eSDSpeedMode)
{
	uint temp;
	uint arg = 0;
	uchar ucSpeedMode;
	int i;

	ucSpeedMode = (eSDSpeedMode == HIGH) ? 1 : 0;

	if (!issue_command(MMC_SET_BLOCKLEN, 64, 0, MMC_RSP_R1)) {
		printf("CMD16 fail\n");
	} else {
		set_blksize_register(7, 64);
		set_blkcnt_register(1);
		set_arg_register(0 * 64);

		set_transfer_mode_register(0, 1, 0, 0, 0);

		arg = (0x1 << 31) | (0xffff << 8) | (ucSpeedMode << 0);

		if (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B))
			printf("CMD6 fail\n");
		else {
			wait_for_r_buf_ready();
			ClearBufferReadReadyStatus();

			for (i = 0; i < 16; i++) {
				temp = s3c_hsmmc_readl(HM_BDATA);
			}

			if (!wait_for_data_done())
				printf(("Transfer NOT Complete\n"));
			
			ClearTransferCompleteStatus();
		}
	}
}

static int get_sd_scr (void)
{
	uint uSCR1, uSCR2;

	if (!issue_command(MMC_SET_BLOCKLEN, 8, 0, MMC_RSP_R1))
		return 0;
	else {
		set_blksize_register(7, 8);
		set_blkcnt_register(1);
		set_arg_register(0 * 8);

		set_transfer_mode_register(0, 1, 0, 0, 0);
		if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1))
			return 0;
		else {
			if (!issue_command(SD_APP_SEND_SCR, 0, 1, MMC_RSP_R1))
				return 0;
			else {
				wait_for_r_buf_ready();
				ClearBufferReadReadyStatus();

				uSCR1 = s3c_hsmmc_readl(HM_BDATA);
				uSCR2 = s3c_hsmmc_readl(HM_BDATA);

				if (!wait_for_data_done())
					printf(("Transfer NOT Complete\n"));
				
				ClearTransferCompleteStatus();

				if (uSCR1 & 0x1)
					sd_spec = 1;	/*  Version 1.10, support cmd6 */
				else
					sd_spec = 0;	/*  Version 1.0 ~ 1.01 */

				dbg("sd_spec = %d(0x%08x)\n", sd_spec, uSCR1);
				return 1;
			}
		}
	}
}

static int check_card_status(void)
{
	if (!issue_command(MMC_SEND_STATUS, rca<<16, 0, MMC_RSP_R1)) {
		return 0;
	} else {
		if (((s3c_hsmmc_readl(HM_RSPREG0) >> 9) & 0xf) == 4) {
			dbg("Card is transfer status\n");
			return 1;
		}
	}

	return 1;
}

static void set_hostctl_speed (uchar mode)
{
	u8 reg8;

	reg8 = s3c_hsmmc_readb(HM_HOSTCTL) & ~(0x1<<2);
	s3c_hsmmc_writeb(reg8 | (mode<<2), HM_HOSTCTL);
}

/* return 0: OK
 * return -1: error
 */
static int set_bus_width (uint width)
{
    uint arg = 0;
    uchar reg = s3c_hsmmc_readb(HM_HOSTCTL);
    uchar bitmode = 0;

    dbg("bus width: %d\n", width);

    switch (width) {
	case 8:
	    width = mmc_card ? 8 : 4;
	    break;
	case 4:
	case 1:
	    break;
	default:
	    return -1;
    }

    card_irq_enable(0);	// Disable sd card interrupt

    if (mmc_card) { /* MMC Card */
	/* MMC Spec 4.x or above */
	if (mmc_spec == 4) {
	    if (width == 1) bitmode = 0;
	    else if (width == 4) bitmode = 1;
	    else if (width == 8) bitmode = 2;
	    else {
		printf("#### unknown mode\n");
		return -1;
	    }

	    arg = ((3 << 24) | (183 << 16) | (bitmode << 8));
	    while (!issue_command(MMC_SWITCH, arg, 0, MMC_RSP_R1B));
	} else
	    bitmode = 0;
    }
    else { /* SD Card */
	if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1)) {
	    printf("Error APPCMD for buswidth:%d\n", width);
	    return -1;
	}
	else {
	    if (width == 1) {	// 1-bits
		bitmode = 0;
		if (!issue_command(MMC_SWITCH, 0, 0, MMC_RSP_R1B)) {
		    printf("Error SWTICH to buswidth:%d\n", 1);
		    return -1;
		}
	    }
	    else {	// 4-bits
		bitmode = 1;
		if (!issue_command(MMC_SWITCH, 2, 0, MMC_RSP_R1B)) {
		    printf("Error SWTICH to buswidth:%d\n", 4);
		    return -1;
		}
	    }
	}
 
#if 0     /* comment by whg HHTECH */
	if (!issue_command(MMC_APP_CMD, rca<<16, 0, MMC_RSP_R1)) {
	    printf("Error APPCMD2 for buswidth:%d\n", width);
	    return -1;
	}
	else {
	    if(!issue_command(SD_APP_SET_BUS_WIDTH, SD_BUS_WIDTH_4, 0,
		MMC_RSP_R1 | MMC_CMD_AC)) {
		printf("Error APP_SET_BUS_WIDTH for buswidth:%d\n", width);
		return -1;
	    }
	}
#endif    /* comment by WangGang   */
    }

    if (bitmode == 2)
	reg |= 1 << 5;
    else
	reg |= bitmode << 1;

    s3c_hsmmc_writeb(reg, HM_HOSTCTL);
    card_irq_enable(1);
    dbg(" transfer rHM_HOSTCTL(0x28) = 0x%02x\n", s3c_hsmmc_readb(HM_HOSTCTL));

    return 0;
}

static int set_sd_ocr (void)
{
	uint i, ocr;

	issue_command(MMC_APP_CMD, 0x0, 0, MMC_RSP_R1);
	issue_command(SD_APP_OP_COND, 0x0, 0, MMC_RSP_R3);
	ocr = s3c_hsmmc_readl(HM_RSPREG0);
	dbg("ocr1: %08x\n", ocr);
//	printf("%s:%d, ocr=0x%x\n", __func__, __LINE__, ocr);

	issue_command(MMC_GO_IDLE_STATE, 0x00, 0, 0);
	if(sd_if_cond(ocr) == 0) ocr |= (1 << 30);
	else {
	    issue_command(MMC_GO_IDLE_STATE, 0x00, 0, 0);
	    issue_command(MMC_APP_CMD, 0x0, 0, MMC_RSP_R1);
	    issue_command(SD_APP_OP_COND, 0x0, 0, MMC_RSP_R3);
	    ocr = s3c_hsmmc_readl(HM_RSPREG0);
	}

	for (i = 0; i < 250; i++) {
		issue_command(MMC_APP_CMD, 0x0, 0, MMC_RSP_R1);
		issue_command(SD_APP_OP_COND, ocr, 0, MMC_RSP_R3);

		ocr = s3c_hsmmc_readl(HM_RSPREG0);
//	printf("%s:%d, ocr=0x%x\n", __func__, __LINE__, ocr);
		dbg("ocr2: %08x\n", ocr);
		if (ocr & (0x1 << 31)) {
			dbg("Voltage range: ");
			if (ocr & (1 << 21))
				dbg("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				dbg("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				dbg("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				dbg("2.7V ~ 3.1V");

			if (ocr & (1 << 7))
				dbg(", 1.65V ~ 1.95V\n");
			else
				dbg('\n');

			mmc_card = 0;
			return 1;
		}
		udelay(1000);
	}

	// The current card is MMC card, then there's time out error, need to be cleared.
	ClearErrInterruptStatus();
	return 0;
}

static int set_mmc_ocr (void)
{
	uint i, ocr;

	for (i = 0; i < 250; i++) {
		issue_command(MMC_SEND_OP_COND, 0x40FF8000, 0, MMC_RSP_R3);

		ocr = s3c_hsmmc_readl(HM_RSPREG0);
		dbg("ocr1: %08x\n", ocr);

		if (ocr & (0x1 << 31)) {
			dbg("Voltage range: ");
			if (ocr & (1 << 21))
				dbg("2.7V ~ 3.4V");
			else if (ocr & (1 << 20))
				dbg("2.7V ~ 3.3V");
			else if (ocr & (1 << 19))
				dbg("2.7V ~ 3.2V");
			else if (ocr & (1 << 18))
				dbg("2.7V ~ 3.1V");
			mmc_card = 1;
			if (ocr & (1 << 7))
				dbg(", 1.65V ~ 1.95V\n");
			else
				dbg('\n');
			return 1;
		}
	}

	// The current card is SD card, then there's time out error, need to be cleared.
	ClearErrInterruptStatus();
	return 0;
}

static void clock_config (uint clksrc, uint div)
{
	uint base_freq = 0, freq;

	if (clksrc == SD_HCLK)
		base_freq = HCLK;
	else if (clksrc == SD_EPLL)	/* Epll Out 48MHz */
		base_freq = 100000000;
	else
		clksrc = HCLK;

	freq = base_freq / (div * 2);
	dbg("freq = %dMHz\n", freq / (1000000));

	if (mmc_card) {
		if (mmc_spec == 4) {
			/* It is necessary to enable the high speed mode
			 * in the card before changing the clock freq
			 * to a freq higher than 20MHz.
			 */
			if (freq > 20000000) {
				set_mmc_speed(HIGH);
				dbg("Set MMC High speed mode OK!!\n");
			} else {
				set_mmc_speed(NORMAL);
				dbg("Set MMC Normal speed mode OK!!\n");
			}
		} else		// old version
			dbg("Old version MMC card can not support working frequency higher than 20MHz\n");
	} else {
		get_sd_scr();
		if (sd_spec == 1) {
			if (freq > 25000000) {
				set_sd_speed(HIGH);	//Higher than 25MHz, should use high speed mode. Max 50MHz and 25MB/sec
				dbg("Set SD High speed mode OK!!\n");
			} else {
				set_sd_speed(NORMAL);
				dbg("Set SD Normal speed mode OK!!\n");
			}
		} else
			dbg("Old version SD card can not support working frequency higher than 25MHz\n");
	}

	/* When Higher than 25MHz, it is necessary
	 * to enable high speed mode of the host controller.
	 */
	if (freq > 25000000) {
		set_hostctl_speed(HIGH);
	} else {
		set_hostctl_speed(NORMAL);
	}

	/* when change the sd clock frequency, need to stop sd clock. */
	hsmmc_clock_onoff(1);
	set_clock(clksrc, div);
	dbg("clock config rHM_HOSTCTL(0x28) = 0x%02x\n", s3c_hsmmc_readb(HM_HOSTCTL));

}

static int check_dma_int (void)
{
    uint i, ret = -1;

    for (i = 0; i < 0x400000; i++) {
	if (s3c_hsmmc_readw(HM_NORINTSTS) & 0x0002) {
	    dbg("Transfer Complete\n");
	    s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | 0x0002, HM_NORINTSTS);
	    ret = 0;
	    break;
	}
	if (s3c_hsmmc_readw(HM_NORINTSTS) & 0x8000) {
	    printf("error found: %04x\n", s3c_hsmmc_readw(HM_ERRINTSTS));
	    ret = -2;
	    break;
	}
	udelay(1);
    }

    if(ret) HSMMC_DPRINT("Err,i = %d, ret = %d, reg=0x%x\n", i, ret, s3c_hsmmc_readw(HM_NORINTSTS));
    return ret;
}

#if 0     /* comment by whg HHTECH */
static uint process_ext_csd (void)
{
	u8 ext_csd[512];
	
	memset(ext_csd, 0, sizeof(ext_csd));

	if (ext_csd >= (u8 *)0xc0000000)
		SetSystemAddressReg(virt_to_phys((ulong)ext_csd));
	else
		SetSystemAddressReg((ulong)ext_csd);

	set_blksize_register(7, 512);
	set_blkcnt_register(1);
	set_transfer_mode_register(0, 1, 0, 1, 1);

	while(!issue_command(MMC_SEND_EXT_CSD, 0, 1, MMC_RSP_R1 | MMC_CMD_ADTC));

	check_dma_int();
	while (!HS_DMA_END);

	return (((ext_csd[215] << 24) | (ext_csd[214] << 16) | (ext_csd[213] << 8) | ext_csd[212]) / (2 * 1024));
}
#endif    /* comment by WangGang   */

static void display_card_info (void)
{	
    uint card_size;
    mmc_csd_t mmc_csd;
    uint i, resp[4];
    uint c_size, c_size_multi, read_bl_len, read_bl_partial, blk_size;

    for (i=0; i<4; i++) {
	    resp[i] = s3c_hsmmc_readl(HM_RSPREG0+i*4);
	    dbg("%08x\n", resp[i]);
    }

    read_bl_len = ((resp[2] >> 8) & 0xf);
    read_bl_partial = ((resp[2] >> 7) & 0x1);
    c_size = ((resp[2] & 0x3) << 10) | ((resp[1] >> 22) & 0x3ff);
    c_size_multi = ((resp[1] >> 7) & 0x7);

    card_size = (1 << read_bl_len) * (c_size + 1) * (1 << (c_size_multi + 2)) / 1048576;
    blk_size = (1 << read_bl_len);

    dbg(" read_bl_len: %d\n", read_bl_len);
    dbg(" read_bl_partial: %d\n", read_bl_partial);
    dbg(" c_size: %d\n", c_size);
    dbg(" c_size_multi: %d\n", c_size_multi);

    dbg(" One Block Size: %dByte\n", blk_size);
    dbg(" Total Card Size: %dMByte\n\n", card_size + 1);

    memset(&mmc_csd, 0, sizeof(mmc_cid_t));
    MOVLEFT_CID_CSD(resp);
//	printf("mov:RESP:%02x %02x %02x %02x\n",  resp[0], resp[1], resp[2], resp[3]);
    if(1 == mmc_decode_csd(resp, &mmc_csd))
	printf("SDHC ");

    card_size = (mmc_csd.capacity>>10) * (1 << mmc_csd.read_blkbits);     // Kb 
    card_size = card_size >> 10; // Mb
    mmc_dev.lba = mmc_csd.capacity*(mmc_csd.read_blkbits-8);

    printf("card size: %d MB(blocks=%d) on channel %d ,rca=0x%x", card_size + 1, 
	mmc_csd.capacity*(mmc_csd.read_blkbits-8), dev_select, rca);

    if (card_mid == 0x15)
	    printf("(MoviNAND)");

    printf("\n");
}

#define SD_SEND_IF_COND           8   /* bcr  [11:0] See below   R7  */
#define MMC_RSP_R7      (MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
static int sd_if_cond(ulong ocr)
{
    u8 test_pattern = 0xAA;
    ulong arg, resp0;

    arg = ((ocr & 0xFF8000) != 0) << 8 | test_pattern;

    issue_command(SD_SEND_IF_COND, arg, 0, MMC_RSP_R7 | MMC_CMD_BCR);

    resp0 = s3c_hsmmc_readl(HM_RSPREG0);

    dbg("if_cond: resp0=0x%x\n", resp0);

    if ((resp0 & 0xFF) != test_pattern) return MMC_ERR_FAILED;

    return MMC_ERR_NONE;
}

#if 0     /* comment by whg HHTECH */
static void mmc_decode_cid(u32 *raw_cid, mmc_cid_t *cid)
{
	u32 *resp = raw_cid;

	memset(cid, 0, sizeof(struct mmc_cid));
	/*
	 * SD doesn't currently have a version field so we will
	 * have to assume we can parse this.
	 */
	cid->manfid          = UNSTUFF_BITS(resp, 120, 8);
	cid->oemid           = UNSTUFF_BITS(resp, 104, 16);
	cid->prod_name[0]    = (char)UNSTUFF_BITS(resp, 96, 8);
	cid->prod_name[1]    = (char)UNSTUFF_BITS(resp, 88, 8);
	cid->prod_name[2]    = (char)UNSTUFF_BITS(resp, 80, 8);
	cid->prod_name[3]    = (char)UNSTUFF_BITS(resp, 72, 8);
	cid->prod_name[4]    = (char)UNSTUFF_BITS(resp, 64, 8);
	cid->hwrev           = UNSTUFF_BITS(resp, 60, 4);
	cid->fwrev           = UNSTUFF_BITS(resp, 56, 4);
	cid->serial          = UNSTUFF_BITS(resp, 24, 32);
	cid->year            = UNSTUFF_BITS(resp, 12, 8);
	cid->month           = UNSTUFF_BITS(resp, 8, 4);
	cid->year += 2000; /* SD cards year offset */
}
#endif    /* comment by WangGang   */

/*
 * Given a 128-bit response, decode to our card CSD structure.
 */
static int mmc_decode_csd(u32 *raw_csd, mmc_csd_t *csd)
{
	unsigned int e, m, csd_struct;
	u32 *resp = raw_csd;

	csd_struct = UNSTUFF_BITS(resp, 126, 2);

	switch (csd_struct) {
	case 0:
		m = UNSTUFF_BITS(resp, 115, 4);
		e = UNSTUFF_BITS(resp, 112, 3);
		csd->tacc_ns	 = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
		csd->tacc_clks	 = UNSTUFF_BITS(resp, 104, 8) * 100;

		m = UNSTUFF_BITS(resp, 99, 4);
		e = UNSTUFF_BITS(resp, 96, 3);
		csd->max_dtr	  = tran_exp[e] * tran_mant[m];
		csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

		e = UNSTUFF_BITS(resp, 47, 3);
		m = UNSTUFF_BITS(resp, 62, 12);
		csd->capacity	  = (1 + m) << (e + 2);

		csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
		csd->read_partial = UNSTUFF_BITS(resp, 79, 1);
		csd->write_misalign = UNSTUFF_BITS(resp, 78, 1);
		csd->read_misalign = UNSTUFF_BITS(resp, 77, 1);
		csd->r2w_factor = UNSTUFF_BITS(resp, 26, 3);
		csd->write_blkbits = UNSTUFF_BITS(resp, 22, 4);
		csd->write_partial = UNSTUFF_BITS(resp, 21, 1);
		break;
	case 1:
		/*
		 * This is a block-addressed SDHC card. Most
		 * interesting fields are unused and have fixed
		 * values. To avoid getting tripped by buggy cards,
		 * we assume those fixed values ourselves.
		 */
		block_addr = 1;

		csd->tacc_ns	 = 0; /* Unused */
		csd->tacc_clks	 = 0; /* Unused */

		m = UNSTUFF_BITS(resp, 99, 4);
		e = UNSTUFF_BITS(resp, 96, 3);
		csd->max_dtr	  = tran_exp[e] * tran_mant[m];
		csd->cmdclass	  = UNSTUFF_BITS(resp, 84, 12);

		m = UNSTUFF_BITS(resp, 48, 22);
		csd->capacity     = (1 + m) << 10;

		csd->read_blkbits = 9;
		csd->read_partial = 0;
		csd->write_misalign = 0;
		csd->read_misalign = 0;
		csd->r2w_factor = 4; /* Unused */
		csd->write_blkbits = 9;
		csd->write_partial = 0;
		break;
	default:
		printf("unrecognised CSD structure version %d\n",
			csd_struct);
		return -2;
	}

	return (int)csd_struct;
}

#define  DELAY_LOOPS (0x10000)
unsigned long mmc_read_block(int dev_num, ulong start_blk, ulong blknum, ulong *addr)
{
    uint blksize; //j, , Addr_temp = start_blk;
    uint cmd, multi; //, TotalReadByte, read_blk_cnt = 0;
    int i;

    HSMMC_DPRINT("%s, start=%ld,num=%ld\n", __func__, start_blk, blknum);

    if(blknum > 32786) printf("Too block for read:%d\n", blknum);

    blksize = SD_BLOCK_SIZE;

    for( i = 0; i < DELAY_LOOPS; i++) {
	if(check_card_status()) break;
	udelay(100);
    }
    if(i >= DELAY_LOOPS) printf("check_card_starus error\n");

    s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTSEN) & ~(DMA_STS_INT_EN | BLOCKGAP_EVENT_STS_INT_EN), HM_NORINTSTSEN);
    s3c_hsmmc_writew((HM_NORINTSIGEN & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, HM_NORINTSIGEN);

#ifdef CONFIG_ENABLE_MMU
    SetSystemAddressReg(virt_to_phy_smdk6410(addr));		// AHB System Address For Write
#else
    SetSystemAddressReg((uint)addr);		// AHB System Address For Write
#endif
    set_blksize_register(7, SD_BLOCK_SIZE);		// Maximum DMA Buffer Size, Block Size
    set_blkcnt_register(blknum);	// Block Numbers to Write

    if (block_addr)
	set_arg_register(start_blk);		// Card Start Block Address to Write
    else
	set_arg_register(start_blk * SD_BLOCK_SIZE);	// Card Start Block Address to Write

    cmd = (blknum > 1) ? 18 : 17;
    multi = (blknum > 1);
    
    udelay(99);  // WHG it is important for iNand, or COMMAND index error
    set_transfer_mode_register(multi, 1, multi, 1, 1);
    set_cmd_register(cmd, 1, MMC_RSP_R1);

    if (wait_for_cmd_done()) {
	printf(("Command NOT Complete\n"));
    }
    ClearCommandCompleteStatus();

    udelay(blknum*10);
    if( check_dma_int())
	printf("Wait DMA END error,blknum=%d\n", blknum);

    ClearTransferCompleteStatus();
    s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 3), HM_NORINTSTS);

    HSMMC_DPRINT("func=%s,%d\n", __func__, __LINE__);
    return 1;
}

unsigned long mmc_write_block(int dev_num, ulong start_blk, ulong blknum, ulong *addr)
{
    uint blksize; //j,

    blksize = SD_BLOCK_SIZE;

    s3c_hsmmc_writew((s3c_hsmmc_readw(HM_NORINTSTSEN) & ~(0xffff)) |
	    BUFFER_READREADY_STS_INT_EN |
	    BUFFER_WRITEREADY_STS_INT_EN |
	    TRANSFERCOMPLETE_STS_INT_EN | COMMANDCOMPLETE_STS_INT_EN, HM_NORINTSTSEN);
    
    s3c_hsmmc_writew((s3c_hsmmc_readw(HM_NORINTSIGEN) & ~(0xffff)) | TRANSFERCOMPLETE_SIG_INT_EN, HM_NORINTSIGEN);

#ifdef CONFIG_ENABLE_MMU
    SetSystemAddressReg(virt_to_phy_smdk6410(addr));		// AHB System Address For Write
#else
    SetSystemAddressReg((uint)addr);		// AHB System Address For Write
#endif
    set_blksize_register(7, SD_BLOCK_SIZE);		// Maximum DMA Buffer Size, Block Size
    set_blkcnt_register(blknum);	// Block Numbers to Write

    if (block_addr)
	    set_arg_register(start_blk);
    else
	    set_arg_register(start_blk * SD_BLOCK_SIZE);

    udelay(99); 
    if (blknum == 1) {
	set_transfer_mode_register(0, 0, 1, 1, 1);
	set_cmd_register(24, 1, MMC_RSP_R1);
    } else {
	set_transfer_mode_register(1, 0, 1, 1, 1);
	set_cmd_register(25, 1, MMC_RSP_R1);
    }

    if (wait_for_cmd_done()) {
	printf("\nCommand is NOT completed3\n");
    }
    ClearCommandCompleteStatus();

    /* wait for DMA transfer */
    check_dma_int();

    if (!wait_for_data_done()) {
	printf(("Transfer is NOT Complete\n"));
    }
    ClearTransferCompleteStatus();

    s3c_hsmmc_writew(s3c_hsmmc_readw(HM_NORINTSTS) | (1 << 3), HM_NORINTSTS);

    return 0;
}


void mmc_uninit(void)
{
}

int hsmmc_init (void)
{
	u32 reg;
	uint width;

	memset(&mmc_dev, 0, sizeof(block_dev_desc_t));

	width = 4;

	HCLK = get_HCLK();
	dbg("HCLK = %08lu\n", HCLK);

	hsmmc_clock_onoff(0);

	reg = readl(SCLK_GATE);
	if(0 == dev_select)
	    writel(reg | (1<<27), SCLK_GATE);
	else if(1 == dev_select)
	    writel(reg | (1<<28), SCLK_GATE);

	set_clock(SD_EPLL, 0x80);
	s3c_hsmmc_writeb(0xe, HM_TIMEOUTCON);
	set_hostctl_speed(NORMAL);

	InterruptEnable(0xff, 0xff);

	dbg("HM_NORINTSTS = %x\n", s3c_hsmmc_readw(HM_NORINTSTS));

	/* MMC_GO_IDLE_STATE */
	issue_command(MMC_GO_IDLE_STATE, 0x00, 0, 0);

	ocr_check = 1;

	if (set_sd_ocr()) {
		mmc_card = 0;
		dbg("SD card is detected\n");
	} else 	if (set_mmc_ocr()) {
		mmc_card = 1;
		dbg("MMC card is detected\n");
	} else {
		printf("ERROR ocr 0 MB\n");
		return -11;
	}

	ocr_check = 0;

	/* Check the attached card and place the card
	 * in the IDENT state rHM_RSPREG0
	 */
	issue_command(MMC_ALL_SEND_CID, 0, 0, MMC_RSP_R2);

	/* Manufacturer ID */
	card_mid = (s3c_hsmmc_readl(HM_RSPREG3) >> 16) & 0xFF;

    #if 0 
	{
	     u32 raw_cid[4]; mmc_cid_t mmc_cid;
	     int i;
	     for(i = 0; i < 4; i++) raw_cid[i] = s3c_hsmmc_readl(HM_RSPREG0+i*4);
	     MOVLEFT_CID_CSD(raw_cid);
	     mmc_decode_cid(raw_cid, &mmc_cid);
	     if (1) {
		 printf("MMC/SD card desciption is:\n"
			 "  Manufacturer ID = %08x\n"
			 "  Serial Number = %08x\n"
			 "  HW/FW Revision = %x/%x\n"
			 "  Product Name = %s\n"
			 "  M/Y = %d/%d\n",
			 mmc_cid.manfid, mmc_cid.serial,
			 mmc_cid.hwrev, mmc_cid.fwrev,
			 mmc_cid.prod_name, mmc_cid.month, mmc_cid.year);
	     }

	}
    #endif
	/* fill in device description */
	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	mmc_dev.blksz = SD_BLOCK_SIZE;
	mmc_dev.lba = ~0x00;
	sprintf((char*)mmc_dev.vendor, "Man %08x Snr %08x", __DATE__, __TIME__);
	sprintf((char*)mmc_dev.product,"%s", "HHTECH");
	sprintf((char*)mmc_dev.revision,"%x %x", 2, 0);
	mmc_dev.block_read = mmc_read_block;
	mmc_dev.removable = 1;

	dbg("Product Name : %c%c%c%c%c%c\n", ((s3c_hsmmc_readl(HM_RSPREG2) >> 24) & 0xFF),
	       ((s3c_hsmmc_readl(HM_RSPREG2) >> 16) & 0xFF), ((s3c_hsmmc_readl(HM_RSPREG2) >> 8) & 0xFF), (s3c_hsmmc_readl(HM_RSPREG2) & 0xFF),
	       ((s3c_hsmmc_readl(HM_RSPREG1) >> 24) & 0xFF), ((s3c_hsmmc_readl(HM_RSPREG1) >> 16) & 0xFF));

	// Send RCA(Relative Card Address). It places the card in the STBY state
	rca = (mmc_card) ? 0x0001 : 0x0000;
	issue_command(MMC_SET_RELATIVE_ADDR, rca<<16, 0, MMC_RSP_R1);

	if (!mmc_card) {
		rca = (s3c_hsmmc_readl(HM_RSPREG0) >> 16) & 0xFFFF;
		dbg("=>  rca=0x%08x\n", rca);
	}

	dbg("Enter to the Stand-by State\n");

	issue_command(MMC_SEND_CSD, rca<<16, 0, MMC_RSP_R2);

	if (mmc_card) {
		mmc_spec = (s3c_hsmmc_readl(HM_RSPREG3) >> 18) & 0xF;
		dbg("mmc_spec=%d\n", mmc_spec);
	}

	issue_command(MMC_SELECT_CARD, rca<<16, 0, MMC_RSP_R1);
	dbg("Enter to the Transfer State\n");

	display_card_info();

	/* Operating Clock setting */
	clock_config(SD_EPLL, 2);	// Divisor 1 = Base clk /2      ,Divisor 2 = Base clk /4, Divisor 4 = Base clk /8 ...

	while (set_bus_width(width));
	while (!check_card_status());

	/* MMC_SET_BLOCKLEN */
	while (!issue_command(MMC_SET_BLOCKLEN, SD_BLOCK_SIZE, 0, MMC_RSP_R1));

	s3c_hsmmc_writew(0xffff, HM_NORINTSTS);	

	return 0;
}

void mmc_release_dev(int dev)
{
    memset(&mmc_dev, 0, sizeof(mmc_dev));
    hsmmc_reset();
}

block_dev_desc_t * mmc_get_dev(int dev)
{
    if (dev != dev_select) {
	if( mmc_dev.if_type == IF_TYPE_MMC) 
	    mmc_release_dev(dev);

	dev_select = dev;
	hsmmc_set_gpio(dev);
	hsmmc_reset();
	if(hsmmc_init()) return NULL;
    }

    return ((block_dev_desc_t *)&mmc_dev);
}

#endif
