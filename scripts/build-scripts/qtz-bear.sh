#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

bear -o $RODOS_SRC/bare-metal/stm32h7/cm4/compile_commands.json -- qtz-lib.sh cm4
bear -o $RODOS_SRC/bare-metal/stm32h7/cm7/compile_commands.json -- qtz-lib.sh cm7

PREV=$PWD
cd $QTZ_CM4
bear -o compile_commands.json -- qtz-elf.sh cm4 hal_led.cpp
cd "$PREV"

PREV=$PWD
cd $QTZ_CM7
bear -o compile_commands.json -- qtz-elf.sh cm7 main.cpp
cd "$PREV"
