#!/bin/sh

echo nameserver 89.2.0.1 > /etc/resolv.conf
echo nameserver 89.2.0.2 >> /etc/resolv.conf

modprobe plusb
ifup usb0

ifconfig usb0