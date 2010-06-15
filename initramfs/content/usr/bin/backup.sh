#!/bin/sh

echo "SmartQ Magic SDCard Backup Tool"
echo
umount /mnt/mmc*0p1 2>/dev/null

dd.sh if=/dev/mmcblk0p1 of=/sdcard/dump0p1.img conv=notrunc
