#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

bear -o $RODOS_SRC/bare-metal/stm32h7/cm4/compile_commands.json -- qtz-lib.sh cm4
bear -o $RODOS_SRC/bare-metal/stm32h7/cm7/compile_commands.json -- qtz-lib.sh cm7

bear -o $QTZ_CM4/compile_commands.json -- qtz-elf.sh cm4 $QTZ_CM4/main.cpp
bear -o $QTZ_CM7/compile_commands.json -- qtz-elf.sh cm7 $QTZ_CM7/main.cpp
