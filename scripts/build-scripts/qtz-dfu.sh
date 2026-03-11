#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

if [ "$1" != "cm4" ] && [ "$1" != "cm7" ]; then
	echo "ERROR: Invalid argument received!"
	echo "- cm4"
	echo "- cm7"
	echo "Usage: qtz-dfu.sh <arch> <.bin to upload>"
	exit
fi

DFU_ADDRESS="invalid!"
if [ $1 == "cm4" ]; then
	DFU_ADDRESS="--dfuse-address=0x08100000:leave"
elif [ $1 == "cm7" ]; then
	DFU_ADDRESS="--dfuse-address=0x08040000:leave"
fi

set -exu
sudo dfu-util --device 0x2341:0x035b -D "$2" -a0 "$DFU_ADDRESS"
