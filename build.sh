#!/bin/bash

# Kangaroo Kernel build script thanks to CyanogenMod, anarkia1976

# Kernel Version
BASE_VER=".KANGAROO"
VER=".M7.v076"
VER=$BASE_VER$VER

# AK Variables
export LOCALVERSION=""`echo $VER`

# toolchain
export CROSS_COMPILE=${HOME}/Toolchains/arm-cortex_a15-linux-gnueabihf-linaro_4.9.1-2014.05/bin/arm-cortex_a15-linux-gnueabihf-

# make .config
make m7_defconfig
env KCONFIG_NOTIMESTAMP=true \
make ARCH=arm export SUBARCH=arm CROSS_COMPILE=arm-eabi-

# build the kernel

make -j4
