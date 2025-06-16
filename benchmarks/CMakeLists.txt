cmake_minimum_required(VERSION 3.16)
project(RiveOpenVGBenchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Find Rive (from external build)
set(RIVE_INCLUDE_DIR "${CMAKE_PREFIX_PATH}/include/rive")
set(RIVE_LIBRARIES 
    "${CMAKE_PREFIX_PATH}/lib/librive.a"
    "${CMAKE_PREFIX_PATH}/lib/librive_harfbuzz.a"
    "${CMAKE_PREFIX_PATH}/lib/librive_sheenbidi.a"
    "${CMAKE_PREFIX_PATH}/lib/librive_yoga.a"
    "${CMAKE_PREFIX_PATH}/lib/libminiaudio.a"
)

# Find Skia (from external build)
set(SKIA_INCLUDE_DIR "${CMAKE_PREFIX_PATH}/include/skia")
set(SKIA_LIBRARY "${CMAKE_PREFIX_PATH}/lib/libskia.a")

# Platform-specific libraries
if(TARGET_PLATFORM STREQUAL "imx93")
    # i.MX93 embedded libraries
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(OPENVG REQUIRED openvg)
    set(PLATFORM_LIBRARIES ${OPENVG_LIBRARIES} pthread dl m)
else()
    # Desktop libraries
    find_package(OpenGL REQUIRED)
    find_package(X11 REQUIRED)
    set(PLATFORM_LIBRARIES ${OPENGL_LIBRARIES} ${X11_LIBRARIES} GLX pthread dl m)
endif()

# Include directories
include_directories(
    ${RIVE_INCLUDE_DIR}
    ${SKIA_INCLUDE_DIR}
)

# Console benchmark (no graphics)
add_executable(rive_console_benchmark
    console_benchmark.cpp
)

target_link_libraries(rive_console_benchmark
    ${RIVE_LIBRARIES}
    ${PLATFORM_LIBRARIES}
)

# Visual benchmark (with graphics)
if(NOT TARGET_PLATFORM STREQUAL "imx93")
    add_executable(rive_visual_benchmark
        visual_benchmark.cpp
    )
    
    target_link_libraries(rive_visual_benchmark
        ${RIVE_LIBRARIES}
        ${SKIA_LIBRARY}
        ${PLATFORM_LIBRARIES}
    )
endif()

# Install targets
install(TARGETS rive_console_benchmark
    RUNTIME DESTINATION bin
)

if(TARGET rive_visual_benchmark)
    install(TARGETS rive_visual_benchmark
        RUNTIME DESTINATION bin
    )
endif()

# Install test assets
install(FILES fire_button.riv simple_animation.riv
    DESTINATION share/rive-openvg
)