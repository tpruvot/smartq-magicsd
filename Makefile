#
#  This makefile forcably makes the initramfs image
#  PS: i dont use it, go to initramfs folder
#

DIR=./initramfs
# this mkimage  is compiled to run on ARM
# thus this makefile needs to run on ARM
MKIMAGE=./mkimage
IMAGE_NAME="SmartQ initramfs"
MKIMAGE_ARGS= -A ARM -O Linux -C gzip -T RAMDisk -n $(IMAGE_NAME)

all:	initrd.igz

initramfs.gz:	initramfs/content/bin/upgrade
	(cd initramfs ; find . -print  | cpio -o --format=newc --owner=0) | gzip -c -9 > $@

%.igz:	%.gz
	mkimage -d $^ $(MKIMAGE_ARGS) $@

boot_sd:
	qi/format-sd.sh sdb sd qi/qi.nb0.smartQ5 u-boot/u-boot.bin.smartQ5
boot_sdhc:
	qi/format-sd.sh sdb sdhc qi/qi.nb0.smartQ5 u-boot/u-boot.bin.smartQ5

