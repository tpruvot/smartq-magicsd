CROSS_COMPILE ?= arm-linux-
KERNEL_CMDLINE ?= root=/dev/ram rw init=/sbin/init console=s3cfb nosplash debug
KERNEL_IMAGE ?= ${KERNEL}/arch/arm/boot/Image

SRCS	= boot.S tags.S kernel.S wrap.S
CFLAGS  = -Wall -DKERNEL_CMDLINE="\"${KERNEL_CMDLINE}\""

ifneq (${INITRD},)
    SRCS += initrd.S
    CFLAGS += -DINITRD="\"${INITRD}\""
else
    SRCS += initrd-empty.S
endif

OBJS    = $(patsubst %.S,%.o,$(SRCS))

ifdef MACHINE
	CFLAGS += -D${MACHINE}
endif


gcc     = ${CROSS_COMPILE}gcc
objdump = ${CROSS_COMPILE}objdump
objcopy = ${CROSS_COMPILE}objcopy
ld      = ${CROSS_COMPILE}ld

.PHONY:	${KERNEL}

all: output.dat

clean:
	-rm -f output.dat *.o tags.h link.ld

kernel.o: kernel.S ${KERNEL_IMAGE}
	${gcc} ${CFLAGS} -DKERNEL="\"${KERNEL_IMAGE}\"" -c -o $@ kernel.S

%.o: %.S
	${gcc} ${CFLAGS} -c -o $@ $^

# turn kernel's setup.h into a file we can include
tags.h:	${KERNEL}/include/asm-arm/setup.h
	grep \#define $^ | grep ATAG > tags.h

link.ld: link.lds
	${gcc} ${CFLAGS} -E - < $^ | grep -v '^#' > $@

# build kernel data file from zImage in specified directory
output.dat:	tags.h link.ld ${OBJS}
	${ld} -o output.o -T link.ld ${OBJS}
	${objcopy} -O binary output.o $@
