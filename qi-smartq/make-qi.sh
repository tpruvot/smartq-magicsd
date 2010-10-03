make clean
make
for drive in [1-9] ; do umount /dev/sdb[1-9] ; done
./install-smartq-qi.sh /dev/sdb image/qi-s3c6410-20101002-SmartQ5
