#!/bin/ash

echo "SmartQ SD Magic Keyboard Tool"
echo

LANG=fr
[ "$1" = "" ] || LANG=$1

if [ -f /etc/console/$LANG.kmap ]; then
	loadkmap < /etc/console/$LANG.kmap
else
	echo "/etc/console/$LANG.kmap doesn't exists"
fi

