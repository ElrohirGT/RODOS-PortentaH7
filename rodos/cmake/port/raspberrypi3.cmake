#
# tell cmake to use arm-none-eabi cross compile toolchain...
#

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR ARM)

set(TOOLCHAIN_PREFIX arm-none-eabi-)

set(CMAKE_C_COMPILER   ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}g++)

set(CMAKE_OBJCOPY      ${TOOLCHAIN_PREFIX}objcopy CACHE INTERNAL "objcopy tool")
set(CMAKE_OBJDUMP      ${TOOLCHAIN_PREFIX}objdump CACHE INTERNAL "objdump tool")
set(CMAKE_SIZE_UTIL    ${TOOLCHAIN_PREFIX}size    CACHE INTERNAL "size tool")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

#
# ... and configure RODOS
#

set(port_dir "bare-metal/raspberrypi3")
set(is_port_baremetal TRUE)

set(RODOS_DIR "${CMAKE_CURRENT_LIST_DIR}/../..")

list(APPEND compile_options
    -mcpu=cortex-a53
    -mfloat-abi=soft
    -ffunction-sections
    -fdata-sections
    -fno-strict-aliasing
    -fno-unwind-tables
    -fno-asynchronous-unwind-tables
)

list(APPEND link_options
    -mcpu=cortex-a53
    -mfloat-abi=soft
    -static
    -nostartfiles
    -nostdlib
    LINKER:--gc-sections
    LINKER:--entry=_exception_table
    LINKER:-T${RODOS_DIR}/src/bare-metal/raspberrypi3/scripts/linkerscript.ld
)

list(APPEND sources_to_add
    ${RODOS_DIR}/src/bare-metal/raspberrypi3/startup/*.c
    ${RODOS_DIR}/src/bare-metal/raspberrypi3/startup/*.S
)

set(libraries_to_link c m gcc)

