#!/bin/sh
# 6410 SD Boot formatter
# (C) 2008 Openmoko, Inc
# Author: Andy Green <andy@openmoko.com>

# LAYOUT
# Partition table, then
# VFAT takes up remaining space here
# then...
#
EXT3_ROOTFS_SECTORS=$(( 256 * 1024 * 2 ))
EXT3_BACKUP_FS_SECTORS=$(( 8 * 1024 * 2 ))
QI_ALLOCATION=$(( 256 * 2 ))
#
# lastly fixed stuff: 8KByte initial boot, sig, padding

# ----------------------

BOOT_ONLY="1"

echo "s3c6410 bootable SD partitioning utility"
echo "(C) Openmoko, Inc  Andy Green <andy@openmoko.com>"
echo

# these are fixed in iROM
QI_INITIAL=$(( 8 * 2 ))
SIG=1

FDISK_SCRIPT=/tmp/_fds

if [ -z "$1" -o -z "$2" -o -z "$3" ] ; then
  echo "This formats a SD card for usage on SD Card boot"
  echo "  on 6410 based systems"
  echo
  echo "Usage:"
  echo
  echo "$0 <device for SD, eg, sde> <sd|sdhc> <qi> <bootloader>"
  exit 1
fi

if [ $2 = "sdhc" ] ; then
PADDING=1025
else
PADDING=1
fi

F_BOOT=$4
F_QI=$3

EXT3_TOTAL_SECTORS=$(( $EXT3_ROOTFS_SECTORS + $EXT3_BACKUP_FS_SECTORS ))
REARSECTORS=$(( $QI_ALLOCATION + $QI_INITIAL + $SIG + $PADDING ))

if [ ! -z "`grep $1 /proc/mounts`" ] ; then
  echo "ERROR $1 seems to be mounted, that ain't right"
  exit 2
fi

# set CUT_COLUMN for each OS
case "$(lsb_release --short --description)" in
  Ubuntu\ 7*)
    CUT_COLUMN=5
    ;;
  Ubuntu\ 8.04*)
    CUT_COLUMN=5
    ;;
  Ubuntu\ 10.*)
    CUT_COLUMN=5
    ;;
  Debian\ GNU/Linux*)
    CUT_COLUMN=8
    ;;
  *)
    CUT_COLUMN=4
    ;;
esac

DMESG_LINE=$(dmesg | grep "$1" | grep -E "512-byte (hardware|logical)" | tail -n 1)
SECTORS=$(echo "${DMESG_LINE}" | cut -d' ' -f"${CUT_COLUMN}")
SECTORS=$(echo "${DMESG_LINE}" | cut -d] -f3 |cut -d' ' -f2)

if ! echo "${SECTORS}" | grep '^[[:digit:]]\+$'
then
  echo "problem finding size for /dev/$1 check CUT_COLUMN value for your os"
  echo "CUT_COLUMN=${CUT_COLUMN}  -->  ${SECTORS}"
  echo "dmesg line was:"
  echo "${DMESG_LINE}"
  exit 3
fi

if [ $SECTORS -le 0 ] ; then
  echo "problem finding size for /dev/$1"
  exit 3
fi

echo "$1 is $SECTORS 512-byte blocks"

if [ ! -z "$5" ] ; then


  FATSECTORS=$(( $SECTORS - $EXT3_TOTAL_SECTORS + $REARSECTORS ))
  FATMB=$(( $FATSECTORS / 2048 - 16 ))

  echo "Creating VFAT section $FATMB MB"

  # create the script for fdisk
  # clear the existing partition table
  echo "o" >$FDISK_SCRIPT

  # add main VFAT storage partition
  echo "n" >>$FDISK_SCRIPT
  echo "p" >>$FDISK_SCRIPT
  echo "1" >>$FDISK_SCRIPT
  # first partition == 1
  echo "" >>$FDISK_SCRIPT
  echo "+$FATMB"M >>$FDISK_SCRIPT

  # add the normal EXT3 rootfs
  echo "n" >>$FDISK_SCRIPT
  echo "p" >>$FDISK_SCRIPT
  echo "2" >>$FDISK_SCRIPT
  # continue after last
  echo "" >>$FDISK_SCRIPT
  echo "+$(( $EXT3_ROOTFS_SECTORS / 2048 ))"M >>$FDISK_SCRIPT

  # add the backup EXT3 rootfs
  echo "n" >>$FDISK_SCRIPT
  echo "p" >>$FDISK_SCRIPT
  echo "3" >>$FDISK_SCRIPT
  # continue after last
  echo "" >>$FDISK_SCRIPT
  echo "+$(( $EXT3_BACKUP_FS_SECTORS / 2048 ))"M >>$FDISK_SCRIPT

  # commit it and exit
  echo "w" >>$FDISK_SCRIPT
  echo "q" >>$FDISK_SCRIPT


  if [ "$(BOOT_ONLY)" != "1" ] ; then

  echo formating...
  pause

  # do the partitioning action
  fdisk /dev/$1 <$FDISK_SCRIPT

  # prep the filesystems

  mkfs.vfat "/dev/$1"1 -n main-vfat
  mkfs.ext3 "/dev/$1"2 -L rootfs
  mkfs.ext3 "/dev/$1"3 -L backupfs

  else

  echo "format partitions skipped... edit the BOOT_ONLY variable"

  fi

fi # if -z $4

# copy the full bootloader image to the right place after the
# partitioned area

echo -e "\tSEEK Write $F_BOOT: $SECTORS-$REARSECTORS = $(( $SECTORS - $REARSECTORS)), cnt=512"
dd if=$F_BOOT of=/dev/$1 bs=512 count=512 \
  seek=$(( $SECTORS - $REARSECTORS ))

echo -e "\tSEEK Write $F_QI: $SECTORS-$REARSECTORS+$QI_ALLOCATION = $(( $SECTORS - $REARSECTORS + $QI_ALLOCATION)) cnt=$QI_INITIAL"
dd if=$F_QI of=/dev/$1 bs=512 \
  seek=$(( $SECTORS - $REARSECTORS + $QI_ALLOCATION )) \
  count=$QI_INITIAL
dd if=/dev/zero of=/dev/$1 bs=512 \
  seek=$(( $SECTORS - $REARSECTORS + $QI_ALLOCATION + $QI_INITIAL )) \
  count=$(( $SIG + $PADDING ))

# done
echo
echo "**** completed"
