
	.global	initrd_start
	.global	initrd_end

	.text
initrd_start:
	.incbin	INITRD
initrd_end:

