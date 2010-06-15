#!/bin/ash

if [ "$1" = "" ] ; then
	mount a MMC partition in /mnt 
	echo "usage : $0 0p1"
	exit 0
fi

mount -t auto /dev/mmcblk$1 /mnt/mmcblk$1
#cd /mnt/mmc$1
