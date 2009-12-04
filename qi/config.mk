#
# Include the make variables (CC, etc...)
#

#CROSS_PATH=/usr/local/openmoko/arm
#CROSS_COMPILE=${CROSS_PATH}/bin/arm-angstrom-linux-gnueabi-

####
#COMPILER_LIB_PATH_PRE=${CROSS_PATH}/lib/gcc/arm-angstrom-linux-gnueabi
#COMPILER_LIB_PATH=${COMPILER_LIB_PATH_PRE}/`ls ${COMPILER_LIB_PATH_PRE}`

CPU     = s3c6410

## SmartQ supports these memory/clock configs:
## Fout/HClk/Pclk
SPEED=CONFIG_CLK_800_166_55
# SPEED=CONFIG_CLK_800_133_66
# SPEED=CONFIG_CLK_666_166_55
# SPEED=CONFIG_CLK_666_166_55_SYNC
# SPEED=CONFIG_CLK_666_133_66
# SPEED=CONFIG_CLK_532_133_66


#CROSS_PATH=/usr/local/gcc-arm-linux-4.3.2
#CROSS_COMPILE=${CROSS_PATH}/bin/arm-none-linux-gnueabi-
#COMPILER_LIB_PATH_PRE=${CROSS_PATH}/lib/gcc/arm-none-linux-gnueabi/4.3.2
#COMPILER_LIB_PATH=${COMPILER_LIB_PATH_PRE}

AS	= $(CROSS_COMPILE)as
LD	= $(CROSS_COMPILE)ld
CC	= $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
HOSTCC	= gcc

# we need the mkudfu tool from U-Boot build
#MKUDFU = ../uboot/u-boot/tools/mkudfu

export CROSS_COMPILE AD LD CC OBJCOPY OBJDUMP MKUDFU
