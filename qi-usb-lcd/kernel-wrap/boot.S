/* SMDK2440 Linux Kernel Wrapper
 *
 * (c) 2004 Ben Dooks <ben-smdk2440@fluff.org>
 *
 * Based on original 2.4.20 port by Samsung
*/

#include "config.h"
#include "tags.h"
		
	.text

	.global	go	
go:
	@ setup UART0 from UART1s settings

	mov	r0, #0x50000000
	add	r1, r0, #0x4000

	ldr	r14, [ r1, #0x00 ]
	str	r14, [ r0, #0x00 ]
	ldr	r14, [ r1, #0x28 ]
	str	r14, [ r0, #0x28 ]
	ldr	r14, [ r1, #0x04 ]
	str	r14, [ r0, #0x04 ]	
	ldr	r14, [ r1, #0x08 ]
	str	r14, [ r0, #0x08 ]
	ldr	r14, [ r1, #0x0C ]
	str	r14, [ r0, #0x0C ]

	@ copy initrd to initrd place

	ldr	r0, =initrd_start
	ldr	r1, =initrd_end
	sub	r1, r1, r0
	ldr	r2, =initrd_tag
	str	r1, [ r2, # 4 ]
	ldr	r2, [ r2, # 0 ]
	add	r2, r2, r1
	add	r0, r0, r1
	
copy_initrd_loop:
	ldmdb	r0!, { r4 - r7 }
	stmdb	r2!, { r4 - r7 }
	subs	r1, r1, #4*4
	bge	copy_initrd_loop
	
	@ ensure the cache is cleaned
	
	adr	r0, go
	add	r1, r0, #0x4000
clean_loop:
	ldr	r2, [ r0 ], #4
	cmp	r0, r1
	bcc	clean_loop

	mov	r0, #0
	mcr	p15, 0, r0, c7, c10, 4  @ drain WB
	mcr	p15, 0, r0, c7, c7, 0   @ flush I & D caches
	
	@ jump into linux kernel
	
	mov	r0, #0			@ code 
	ldr	r1, =MACHINE_ID		@ Linux ID
	ldr	r2, =taglist		@ Base of the code
	ldr	pc, =(MEM_START+0x8000)		@ jump into the kernel
