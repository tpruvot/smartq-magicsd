#!/bin/ash

LANG=fr
[ "$1" = "" ] || LANG=$1

echo "SmartQ SD Magic Keyboard Tool : $LANG keymap"
echo

if [ -f /etc/console/$LANG.kmap ]; then
	loadkmap < /etc/console/$LANG.kmap
else
	echo "/etc/console/$LANG.kmap not found"
fi

