
HOSTCC ?= gcc
## CROSS ?= /opt/arm-2009q1/bin/arm-none-linux-gnueabi-

CC = $(CROSS)gcc
STRIP = $(CROSS)strip

CFLAGS = -DNDEBUG -Wall -pipe -O2 

all: upgrade mkSmartQ 
# all: upgrade mkSmartQ debug cmd_encrypt cmd_identify cmd_encrypt2

mkSmartQ:	mkSmartQ.c
	$(HOSTCC) $(CFLAGS) -o $@ $<
	@ln -s $@ mkSmartQ5
	@ln -s $@ mkSmartQ7


debug: debug.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
	## cp $@ ../rootfs/bin/

upgrade: extract.o upgrade.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
	cp $@ ../initramfs/bin/

clean: ; rm -rf upgrade.o extract.o mkSmartQ.o debug.o ll_port.o  upgrade mkSmartQ debug
