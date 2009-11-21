#
#  This makefile forcably makes the initramfs image
#

DIR=./initramfs
# this mkimage  is compiled to run on ARM
# thus this makefile needs to run on ARM
MKIMAGE=./mkimage
IMAGE_NAME="SmartQ initramfs"
MKIMAGE_ARGS= -A ARM -O Linux -C gzip -T RAMDisk -n $(IMAGE_NAME)

all:	initramfs.igz

initramfs.gz:	initramfs/bin/upgrade
	(cd initramfs ; find . -print  | cpio -o --format=newc --owner=0) | gzip -c -9 > $@

%.igz:	%.gz
	mkimage -d $^ $(MKIMAGE_ARGS) $@

