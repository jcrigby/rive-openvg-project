cmake_minimum_required(VERSION 3.20)
project(RiveOpenVGProject VERSION 1.0.0)

# Project configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Build configuration options
option(BUILD_FOR_EMBEDDED "Build for embedded targets (i.MX93)" OFF)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_DEMOS "Build demo applications" ON)
option(USE_SYSTEM_RIVE "Use system-installed Rive instead of building" OFF)
option(USE_LOCAL_RIVE "Use local Rive build instead of downloading" OFF)
set(LOCAL_RIVE_PATH "" CACHE PATH "Path to local rive-cpp repository with built libraries")

# Target architecture detection/configuration
if(BUILD_FOR_EMBEDDED)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/toolchains/imx93.cmake")
    set(TARGET_PLATFORM "imx93")
else()
    set(TARGET_PLATFORM "desktop")
endif()

# External project dependencies
include(ExternalProject)
include(FetchContent)

# Set up external dependencies directory
set(EXTERNAL_INSTALL_DIR "${CMAKE_BINARY_DIR}/external")
set(CMAKE_PREFIX_PATH "${EXTERNAL_INSTALL_DIR}" ${CMAKE_PREFIX_PATH})

# Include external project configurations
include(external/SkiaOpenVG.cmake)
include(external/RiveCPP.cmake)

# Main project targets (built after dependencies)
ExternalProject_Add(rive-openvg-benchmarks
    SOURCE_DIR "${CMAKE_SOURCE_DIR}/benchmarks"
    CMAKE_ARGS
        -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
        -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/install
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DTARGET_PLATFORM=${TARGET_PLATFORM}
    DEPENDS skia-openvg rive-cpp
    BUILD_ALWAYS TRUE
)

# Installation and packaging
install(DIRECTORY ${CMAKE_BINARY_DIR}/install/
    DESTINATION .
    USE_SOURCE_PERMISSIONS
)

# CPack configuration for creating packages
include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
set(CPACK_PACKAGE_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(CPACK_PACKAGE_CONTACT "your.email@example.com")

if(BUILD_FOR_EMBEDDED)
    set(CPACK_GENERATOR "TGZ")
    set(CPACK_PACKAGE_NAME "rive-openvg-imx93")
else()
    set(CPACK_GENERATOR "DEB;TGZ")
    set(CPACK_PACKAGE_NAME "rive-openvg-desktop")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libgl1-mesa-dev, libx11-dev")
endif()

include(CPack)