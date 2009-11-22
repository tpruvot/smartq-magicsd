#include <common.h>

#ifdef CONFIG_MOVINAND

#include <command.h>
#include <movi.h>

struct movi_offset_t ofsinfo;

#if 0
int do_testhsmmc(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uint start_blk, blknum;
	start_blk = simple_strtoul(argv[1], NULL, 10);
	blknum = simple_strtoul(argv[2], NULL, 10);

	if(argc < 3)
		return 0;
	
	printf("COMMAND --> start_blk = 0x%x, blknum = %d\n", start_blk, blknum);	
	test_hsmmc(4, 1, start_blk, blknum);
	return 1;
}

U_BOOT_CMD(
	testhsmmc,	4,	0,	do_testhsmmc,
	"testhsmmc - hsmmc testing write/read\n",
	NULL
);
#endif

int do_movi(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *cmd;
	ulong addr;
	uint last_blkpos;

	cmd = argv[1];	
	addr = (ulong)simple_strtoul(argv[3], NULL, 16);

	if (addr >= 0xc0000000)
		addr = virt_to_phys(addr);

	if (strcmp(cmd, "init") == 0) {
		if (argc < 4)
			goto usage;
		
		last_blkpos = (uint)simple_strtoul(argv[2], NULL, 10);
		movi_hc = (uint)simple_strtoul(argv[3], NULL, 10);

		if (movi_set_ofs(last_blkpos))
			movi_init();
	} else {
		if (argc < 4)
			goto usage;

		if (ofsinfo.last == 0) {
			printf("### Execute 'movi init' command first!\n");
			return -1;
		}
		
		if (strcmp(cmd, "read") == 0) {
			if (strcmp(argv[2], "u-boot") == 0) {
				printf("Reading Bootloader.. ");
				movi_read((uint)addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
				printf("Completed!\n\n");
			} else if (strcmp(argv[2], "zImage") == 0) {
				printf("Reading zImage from block %d.. ", ofsinfo.zimage);
				movi_read((uint)addr, ofsinfo.zimage, MOVI_ZIMAGE_BLKCNT);
				printf("Completed!\n\n");
			} else {
				goto usage;
			}
		} else if (strcmp(cmd, "write") == 0) {
			if (strcmp(argv[2], "u-boot") == 0) {
				printf("Writing 1st Bootloader at block %d (%d blocks).. ", ofsinfo.bl1, MOVI_BL1_BLKCNT);
				movi_write((uint)addr, ofsinfo.bl1, MOVI_BL1_BLKCNT);
				printf("Completed!\nWriting 2nd Bootloader at block %d (%d blocks).. ", ofsinfo.bl2, MOVI_BL2_BLKCNT);
				movi_write((uint)addr, ofsinfo.bl2, MOVI_BL2_BLKCNT);
				printf("Completed!\n\n");
			} else if (strcmp(argv[2], "zImage") == 0) {
				printf("Writing zImage at block %d (%d blocks).. ", ofsinfo.zimage, MOVI_ZIMAGE_BLKCNT);
				movi_write((uint)addr, ofsinfo.zimage, MOVI_ZIMAGE_BLKCNT);
				printf("Completed!\n\n");
			} else if (strcmp(argv[2], "env") == 0) {
				printf("Writing env at block %d (%d blocks).. ", ofsinfo.env, MOVI_ENV_BLKCNT);
				movi_write((uint)addr, ofsinfo.env, MOVI_ENV_BLKCNT);
				printf("Completed!\n\n");
			} else {
				goto usage;
			}
		} else {
			goto usage;
		}
	}

	return 1;

usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return -1;
}

U_BOOT_CMD(
	movi,	4,	0,	do_movi,
	"movi\t- MoviNAND sub-system\n",
	"init  {total block} {hc} - Initialize MoviNAND\n"
	"movi read  {u-boot | zImage} {addr} - Read data from MoviNAND\n"
	"movi write {u-boot | zImage} {addr} - Write data to MoviNAND\n"
);

#endif /* (CONFIG_COMMANDS & CFG_CMD_MOVINAND) */
