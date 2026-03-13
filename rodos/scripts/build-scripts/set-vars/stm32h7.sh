#!/usr/bin/env bash

# Author Sergio Montenegro (uni Wuerzburg), Sept 2019

if [ -z $TARGET_LIB ]; then
	cat <<EOT1
  ****************
  *** Do not use this architecture directly, please use
  *** one of its derivates:
  ***    cm4
  ***		 cm7
  *********************
EOT1
	exit
fi

export ARCH=stm32h7 # used to select compile directories

#  From here, we can have different sub achictures. this is for stm34f40x, discovery
#  other possibilites:
#     stm32f427
#     MINI-M4
#     stm32f429(STM32F429Discovery)
#     stm32f40x (STM32F4Discovery),
#     stm32f401 (STM32F401 NUCLEO)
#
# We user STM32F407VGT6  (168MHz, 192KByte SRAM) == Discovery board

export ST_LINK="v2_1" #stm32f40x
#export ST_LINK="v2_0" #stm32f401

#---------------------------------------------- from here the same for all sub architectures

RODOS_ARCH_ROOT="${RODOS_SRC}/bare-metal/${ARCH}"
RODOS_SUB_ARCH_ROOT="${RODOS_ARCH_ROOT}/${SUB_ARCH}"

SRCS[1]="${RODOS_SRC}/bare-metal-generic"
SRCS[2]="${RODOS_ARCH_ROOT}"
SRCS[3]="${RODOS_SUB_ARCH_ROOT}"
SRCS[4]="${RODOS_ARCH_ROOT}/hal/Src"
SRCS[5]="${RODOS_ARCH_ROOT}/rodos-hal"
SRCS[6]="${RODOS_ARCH_ROOT}/CMSIS/Device/ST/STM32H7xx/Source/Templates"
SRCS[7]="${RODOS_ARCH_ROOT}/CMSIS/Device/ST/STM32H7xx/Source/Templates/arm"

export INCLUDES=${INCLUDES}" -I ${RODOS_ARCH_ROOT} \
	-I ${RODOS_ARCH_ROOT}/CMSIS/Device/ST/STM32H7xx/Include \
	-I ${RODOS_ARCH_ROOT}/CMSIS/Core/Include \
	-I ${RODOS_ARCH_ROOT}/rodos-hal \
	-I ${RODOS_ARCH_ROOT}/hal/Inc "

export INCLUDES_TO_BUILD_LIB=" -I ${RODOS_SRC}/bare-metal-generic  \
    -I ${RODOS_ARCH_ROOT} \
		-I ${RODOS_ARCH_ROOT}/rodos-hal \
		-I ${RODOS_ARCH_ROOT}/CMSIS/Device/ST/STM32H7xx/Include \
    -I ${RODOS_ARCH_ROOT}/CMSIS/Core/Include \
    -I ${RODOS_ARCH_ROOT}/hal/Inc "

export LINKER_SCRIPT="${RODOS_ARCH_ROOT}/scripts/STM32H747XIHX_FLASH.ld"

# arm-none-eabi-gcc -c stm32h7xx_hal.c -o stm32h7xx_hal.o \
# 	-mcpu=cortex-m4 -mthumb -mfloat-abi=softfp -mfpu=fpv4-sp-d16 \
# 	-DCORE_CM4 -DSTM32H747xx -DUSE_FULL_LL_DRIVER \
# 	-I./include -I../CM4/include -Os -g

# FIXME: Do we need the OSC_CLK param?
# export CFLAGS_BASICS_COMMON=" -g3 -gdwarf-2 -DHSE_VALUE=${OSC_CLK}"

# Enable debug information
# export CFLAGS_BASICS_COMMON=" -g3 -gdwarf-2"

export CFLAGS_BASICS="${CFLAGS_BASICS_COMMON} -D${MCU_FLAG} -DSTM32H747xx -DUSE_HAL_DRIVER -DATOMIC_VARIANT=ATOMIC_VARIANT_STD_FALLBACK_CUSTOM"
# NOTE: WITH gc-sections
export LINKFLAGS=" -T${LINKER_SCRIPT} -nostartfiles -nodefaultlibs -nostdlib -Xlinker --undefined=g_pfnVectors -Xlinker -L${RODOS_LIBS}/${TARGET_LIB} -Xlinker --gc-sections -fno-unwind-tables -fno-asynchronous-unwind-tables -lrodos -lm"
# NOTE: WITHOUT gc-sections
# export LINKFLAGS=" -T${LINKER_SCRIPT} -nostartfiles -nodefaultlibs -nostdlib -Xlinker --undefined=g_pfnVectors -Xlinker -L${RODOS_LIBS}/${TARGET_LIB} -fno-unwind-tables -fno-asynchronous-unwind-tables -lrodos -lm"
export CFLAGS=${CFLAGS}" ${CFLAGS_BASICS} ${HWCFLAGS} "
export CPPFLAGS=${CPPFLAGS}" -Wno-register " # ignore register keyword in CMSIS header when included in cpp files

#export ARM_TOOLS="/opt/arm-tools/bin/"
export ARM_TOOLS=""

export CPP_COMP="${CXX:-${ARM_TOOLS}arm-none-eabi-g++} "
export C_COMP="${CC:-${ARM_TOOLS}arm-none-eabi-gcc} " # only to compile BSP and Drivers from chip provider
export AR="${AR:-${ARM_TOOLS}arm-none-eabi-ar} "

# Print all commands are they are being executed!
# set -x
