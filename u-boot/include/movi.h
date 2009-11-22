#ifndef __MOVI_H__
#define __MOVI_H__

#if defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#define HSMMC_CHANNEL		1
#else
#define HSMMC_CHANNEL		0
#endif
#define MOVI_INIT_REQUIRED	0
#if defined(CONFIG_S3C6400) || defined(CONFIG_S3C6410)
#define	TCM_BASE		0x0C004000
#define BL2_BASE		0x57E00000
#elif defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#define	TCM_BASE		0x40004000
#define BL2_BASE		0x33E00000
#else
# error TCM_BASE or BL2_BASE is not defined
#endif

/* TCM function for bl2 load */
#if defined(CONFIG_S3C6400)
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(uint, ushort, uint *, uint, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif  defined(CONFIG_S3C6410)
#define CopyMovitoMem(a,b,c,d,e)	(((int(*)(int, uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d,e))
#elif defined(CONFIG_S3C2450) || defined(CONFIG_S3C2416)
#define CopyMovitoMem(a,b,c,d)		(((int(*)(uint, ushort, uint *, int))(*((uint *)(TCM_BASE + 0x8))))(a,b,c,d))
#endif

/* size information */
#if defined(CONFIG_S3C6400)
#define SS_SIZE			(4 * 1024)
#define eFUSE_SIZE		(2 * 1024)	// 1.5k eFuse, 0.5k reserved
#else
#define SS_SIZE			(8 * 1024)
#define eFUSE_SIZE		(1 * 1024)	// 0.5k eFuse, 0.5k reserved`
#endif

/* offset information */
#define PART_UBOOT_OFFSET	0x0
#define PART_ZIMAGE_OFFSET	0x40000
#define PART_ROOTFS_OFFSET	0x200000
#define PART_EXTRA_OFFSET	0x3200000

/* movinand definitions */
#define MOVI_BLKSIZE		512

#ifdef CONFIG_BOOT_MOVINAND
#define MOVI_TOTAL_BLKCNT	*((volatile unsigned int*)(TCM_BASE - 0x4))
#define MOVI_HIGH_CAPACITY	*((volatile unsigned int*)(TCM_BASE - 0x8))
#else
#define MOVI_TOTAL_BLKCNT	7864320 // 7864320 // 3995648 // 1003520 /* static movinand total block count: for writing to movinand when nand boot */
#define MOVI_HIGH_CAPACITY	0
#endif

#define MOVI_LAST_BLKPOS	(MOVI_TOTAL_BLKCNT - (eFUSE_SIZE / MOVI_BLKSIZE))
#define MOVI_BL1_BLKCNT		(SS_SIZE / MOVI_BLKSIZE)
#define MOVI_ENV_BLKCNT		(CFG_ENV_SIZE / MOVI_BLKSIZE)
#define MOVI_BL2_BLKCNT		(((PART_ZIMAGE_OFFSET - PART_UBOOT_OFFSET) / MOVI_BLKSIZE) - MOVI_ENV_BLKCNT)
#define MOVI_ZIMAGE_BLKCNT	((PART_ROOTFS_OFFSET - PART_ZIMAGE_OFFSET) / MOVI_BLKSIZE)
#define MOVI_BL2_POS		(MOVI_LAST_BLKPOS - MOVI_BL1_BLKCNT - MOVI_BL2_BLKCNT - MOVI_ENV_BLKCNT)

struct movi_offset_t {
	uint	last;
	uint	bl1;
	uint	env;
	uint	bl2;
	uint	zimage;
};

/* external functions */
extern void hsmmc_set_gpio(void);
extern void hsmmc_reset (void);
extern int hsmmc_init (void);
extern int movi_init(void);
extern void movi_set_capacity(void);
extern int movi_set_ofs(uint last);
extern void movi_write (uint addr, uint start_blk, uint blknum);
extern void movi_read (uint addr, uint start_blk, uint blknum);
extern void movi_write_env(ulong addr);
extern void movi_read_env(ulong addr);

#if defined(CONFIG_S3C2450)
extern ulong virt_to_phy_smdk2450(ulong addr);
#elif defined(CONFIG_S3C6400)
extern ulong virt_to_phy_smdk6400(ulong addr);
#elif defined(CONFIG_S3C6410)
extern ulong virt_to_phy_smdk6410(ulong addr);
#elif defined(CONFIG_S3C2416)
extern ulong virt_to_phy_smdk2416(ulong addr);
#endif

extern void test_hsmmc (uint width, uint test, uint start_blk, uint blknum);

/* external variables */
extern uint movi_hc;
extern struct movi_offset_t ofsinfo;

#endif /*__MOVI_H__*/
