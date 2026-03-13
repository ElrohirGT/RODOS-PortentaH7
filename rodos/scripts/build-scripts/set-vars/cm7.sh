#!/usr/bin/env bash

export TARGET_LIB=cm7 # used as name for the generated lib
export SUB_ARCH=cm7   # CM4 and CM7

export HWCFLAGS=" -mcpu=cortex-m7 -mfloat-abi=softfp -mfpu=fpv5-d16"
export MCU_FLAG="CORE_CM7"

# FIXME: Update this values to correct ARCH
# export INCLUDES=${INCLUDES}" -I ${RODOS_SRC}/bare-metal/stm32f4/platform-parameter/discovery " # only for platform-parameter.h

# NOTE: Do we need this? Which value?
export OSC_CLK=8000000 # STM32F4Discovery, STM32F429Discovery

source $RODOS_VARS_DIR/stm32h7.sh
