#!/bin/sh

#export CROSS_COMPILE=/usr/local/software/arm-2010q1-188-arm-none-eabi/bin/arm-none-eabi-


# !! NOT WORKING FOR THE MOMENT !!


CROSS_PATH=/usr/local/software/toolchain-arm-karmic.rgs0
export CROSS_COMPILE=$CROSS_PATH/bin/arm-none-linux-gnueabi-


make ARCH=arm smdk6410_config all
