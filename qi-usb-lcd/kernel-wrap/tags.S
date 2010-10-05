/* SMDK2440 Linux Kernel Wrapper
 *
 * (c) 2004 Ben Dooks <ben-smdk2440@fluff.org>
 *
 * Based on original 2.4.20 port by Samsung
*/

#include "config.h"
#include "tags.h"
		
	.text

	.macro  tag type, size
		.word   ((\size) + 8)/4
		.word   \type
        .endm

	.global	taglist
	.global	initrd_tag
taglist:
	tag	ATAG_CORE, 12
	.word	1
	.word	4*1024
	.word	0
	
	tag	ATAG_MEM, 8
	.word	MEM_SIZE
	.word	MEM_START

	tag	ATAG_INITRD2, 8
initrd_tag:	
	.word	MEM_START+0x1000000
	.word	4*1024*1024

	tag     ATAG_CMDLINE, (atag_cmdline_end - atag_cmdline_start)

atag_cmdline_start:
	.asciz  KERNEL_CMDLINE
	.byte   0
	.align
atag_cmdline_end:
		
	tag	ATAG_NONE, -8	

