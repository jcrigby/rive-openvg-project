# Cross-compilation toolchain for NXP i.MX93

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# Toolchain paths - adjust these for your environment
set(CROSS_COMPILE_PREFIX "aarch64-linux-gnu")

# Find the toolchain
find_program(CMAKE_C_COMPILER ${CROSS_COMPILE_PREFIX}-gcc)
find_program(CMAKE_CXX_COMPILER ${CROSS_COMPILE_PREFIX}-g++)
find_program(CMAKE_STRIP ${CROSS_COMPILE_PREFIX}-strip)

if(NOT CMAKE_C_COMPILER)
    message(FATAL_ERROR "Cross compiler not found. Install with: sudo apt-get install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu")
endif()

# Set sysroot if available (BSP specific)
if(DEFINED ENV{IMX93_SYSROOT})
    set(CMAKE_SYSROOT $ENV{IMX93_SYSROOT})
    set(CMAKE_FIND_ROOT_PATH ${CMAKE_SYSROOT})
endif()

# Compiler flags for i.MX93 optimization
set(CMAKE_C_FLAGS_INIT "-mcpu=cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard")
set(CMAKE_CXX_FLAGS_INIT "-mcpu=cortex-a55 -mfpu=neon-fp-armv8 -mfloat-abi=hard")

# OpenVG library paths for i.MX93
set(OPENVG_INCLUDE_DIR "/usr/include/VG" CACHE PATH "OpenVG include directory")
set(OPENVG_LIBRARY "OpenVG" CACHE STRING "OpenVG library")

# Search for programs in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# For libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)