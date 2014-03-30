#!/bin/bash

# Kangaroo Kernel build script thanks to CyanogenMod, anarkia1976

DEFCONFIG_FILE=m7_defconfig

if [ -z "$DEFCONFIG_FILE" ]; then
	echo "Need defconfig file(j1v-perf_defconfig)!"
	exit -1
fi

if [ ! -e arch/arm/configs/$DEFCONFIG_FILE ]; then
	echo "No such file : arch/arm/configs/$DEFCONFIG_FILE"
	exit -1
fi


# Kernel Version
BASE_VER=".KANGAROO"
VER=".M7.v008"
VER=$BASE_VER$VER

# AK Variables
export LOCALVERSION=""`echo $VER`

# toolchain
export CROSS_COMPILE=${HOME}/repos/arm-cortex_a15-linux-gnueabihf-linaro_4.8.3-2014.03/bin/arm-cortex_a15-linux-gnueabihf-


# make .config
env KCONFIG_NOTIMESTAMP=true \
make ARCH=arm export SUBARCH=arm CROSS_COMPILE=arm-eabi-
make m7_defconfig

# build the kernel

make -j3
