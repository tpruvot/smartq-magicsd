
HOSTCC ?= gcc
## CROSS ?= /opt/arm-2009q1/bin/arm-none-linux-gnueabi-

CC = $(CROSS)gcc
STRIP = $(CROSS)strip

CFLAGS = -DNDEBUG -Wall -pipe -O2 

all: upgrade compress 
# all: upgrade compress debug cmd_encrypt cmd_identify cmd_encrypt2

compress: compress.c ; $(HOSTCC) $(CFLAGS) -o $@ $<

debug: debug.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
	## cp $@ ../rootfs/bin/

upgrade: uncompress.o upgrade.o
	$(CC) $(LDFLAGS) -o $@ $^
	$(STRIP) $@
	cp $@ ../initramfs/bin/

cmd_encrypt: ll_port.o cmd_encrypt.o 
	$(CC) $(CFLAGS) -o $@ ll_port.c cmd_encrypt.c lib_CM.a lib_Support.o
	$(STRIP) $@

cmd_encrypt2: ll_port.o cmd_encrypt2.o 
	$(CC) $(CFLAGS) -o $@ ll_port.c cmd_encrypt2.c lib_CM.a lib_Support.o
	$(STRIP) $@

cmd_identify: ll_port.o cmd_identify.o 
	$(CC) $(CFLAGS) -o $@ ll_port.c cmd_identify.c lib_CM.a lib_Support.o
	$(STRIP) $@

clean: ; rm -rf upgrade.o uncompress.o compress.o debug.o ll_port.o cmd_encrypt.o cmd_identify.o upgrade compress debug cmd_encrypt cmd_identify cmd_encrypt2.o cmd_encrypt2
