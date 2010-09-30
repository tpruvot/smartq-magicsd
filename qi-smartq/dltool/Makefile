# dltool/Makefile
#
# Makefile for SMDK24XX dltool
#
# (c) 2004,2006 Ben Dooks, Ben Dooks <ben-smdk2440@fluff.org>
#

CFLAGS=-O2 -Wall -g -lusb

ifneq (${VERSION},)
	CFLAGS+=-DVERSION=\"${VERSION}\"
endif

.PHONY:	clean

dltool:	dltool.c
	$(CC) ${CFLAGS} -o smdk-usbdl dltool.c
	-echo Remember to apt-get install libusb-dev

clean:
	-rm -f smdk-usbdl
