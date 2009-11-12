#!/bin/sh

#[ -w / ] || mount -o remount,rw /

FILE=/etc/pango/pango.modules
[ -f $FILE ] || pango-querymodules > $FILE

FILE=/etc/gtk-2.0/gdk-pixbuf.loaders
[ -f $FILE ] || gdk-pixbuf-query-loaders > $FILE

FILE=/var/lib/dbus/machine-id
[ -f $FILE ] || (mkdir -p `dirname $FILE` && dbus-uuidgen --ensure)



mv $0 $0-done	# XXX:

#mount -o remount,ro /

