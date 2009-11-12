#!/bin/sh

#modprobe jz4740_udc use_dma=0; modprobe g_ether

modprobe dm9000; sleep 2;
if  ifconfig eth0 192.168.2.251 up; then echo; else
    rmmod dm9000; modprobe g_ether; sleep 2;
    ifconfig usb0 192.168.2.251 up
fi

route add default gw 192.168.2.254
exit
ROOTFS=arm-eabi-v6el-vfp-mojo; while true; do
    if mount -o nolock,proto=tcp 192.168.2.239:/srv/rootfs/$ROOTFS /mnt/disk; then

cd /mnt/disk;	# XXX:

mount --rbind /sys sys;	mount --rbind /proc proc;
mount --rbind /dev dev; mount --bind /dev/pts dev/pts;

mount --bind  /proc/bus/usb proc/bus/usb
mount --rbind /dev/shm dev/shm

chroot /mnt/disk /bin/sh	# XXX:
exit

mkdir -p var/volatile; cd var/volatile && mkdir -p cache lock log run tmp 

[ -x /etc/init.d/rc ] && /etc/init.d/rc 5	# XXX: for PokyLinux
[ -x /init || grep rdinit /proc/cmdline ] || switch_root /mnt/disk /bin/sh

	break; else sleep 3; fi; done

