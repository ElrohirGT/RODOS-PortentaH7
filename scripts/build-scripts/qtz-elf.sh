#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

if [ "$1" != "cm4" ] && [ "$1" != "cm7" ]; then
	echo "ERROR: Invalid argument received!"
	echo "- cm4"
	echo "- cm7"
	echo "Usage: qtz-elf.sh <arch> <...files to compile>"
	exit
fi

# INCLUDES="-I $QTZ_RODOS/build/libs/$1"
# set -exu
# rodos-executable.sh $1 "$RODOS_SRC/bare-metal/stm32h7/CMSIS/Device/ST/STM32H7xx/Source/Templates/arm/startup_stm32h747xx.s" ${@:2}
rodos-executable.sh $1 ${@:2}
arm-none-eabi-objcopy -O binary tst tst.bin
