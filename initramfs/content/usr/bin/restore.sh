#!/bin/ash

echo "SmartQ Magic Restore Tool"
echo

PART=mmcblk1p2
[ "$1" = "" ] || PART=$1

if [ ! "" = "`mount | grep $PART`" ]; then
	umount /mnt/$PART
fi

dd.sh of=/dev/$PART if=/sdcard/dump0p1.img conv=noerror

#check partition
e2fsck -y /dev/$PART

#needed if image is smaller than disk
resize2fs /dev/$PART
