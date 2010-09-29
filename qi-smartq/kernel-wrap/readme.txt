Kernel Wrap
-----------

Wrap a kernel image suitable for raw-download into RAM.

This provides the entry parameters suitable to get a kernel
started without a Linux capable bootloader present.


Usage
-----

make KERNEL=/path/to/kernel CROSS_COMPILE=/prefix/for/crossgcc INITRD=/path/to/initrd KERNEL_CMDLINE="console=ttySAC0,115200"

example:
	make KERNEL=/kernel/linux CROSS_COMPILE=/opt/cross/arm-linux-

