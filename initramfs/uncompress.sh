#!/bin/sh

#use this script first to extract files in ./$outdir/  from an initramfs.igz file

outdir=dump
initrd=./initramfs.igz

[ -f "$initrd" ] || exit 1

echo extracting $1 to $2"\\"
mkdir $outdir;(cd $outdir; dd if=../$initrd bs=64 skip=1 | zcat | cpio -i --no-absolute-filenames)

