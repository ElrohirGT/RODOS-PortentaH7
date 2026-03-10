#!/usr/bin/env bash

set -e

DIR_NAME=$(dirname "$0")
source "$DIR_NAME"/../quit-if-environment-is-wrong.sh

bear -o $RODOS_SRC/bare-metal/stm32h7/cm4/compile_commands.json -- rodos-lib.sh cm4
bear -o $RODOS_SRC/bare-metal/stm32h7/cm7/compile_commands.json -- rodos-lib.sh cm7
