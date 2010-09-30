#include "config.h"

OUTPUT_ARCH(arm)
ENTRY(_entry)
SECTIONS
{
	. = MEM_START;
	.text : {
		wrap.o(*)
		
		. = ALIGN(256);
		tags.o(*)
		boot.o(*)

		. = ALIGN(0x8000);
		kernel.o(*)
		*(.text)
	}
}
