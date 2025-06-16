# Skia with OpenVG backend external project

set(SKIA_OPENVG_GIT_URL "https://github.com/YOUR_USERNAME/skia-openvg.git")
set(SKIA_OPENVG_GIT_TAG "openvg-backend")

# Skia build configuration
if(BUILD_FOR_EMBEDDED)
    set(SKIA_BUILD_ARGS
        is_official_build=true
        skia_use_system_freetype2=false
        skia_use_system_harfbuzz=false
        skia_use_openvg=true
        skia_enable_gpu=false
        target_cpu="arm64"
        target_os="linux"
    )
else()
    set(SKIA_BUILD_ARGS
        is_debug=false
        skia_use_openvg=true
        skia_enable_gpu=true
        skia_use_gl=true
    )
endif()

ExternalProject_Add(skia-openvg
    GIT_REPOSITORY ${SKIA_OPENVG_GIT_URL}
    GIT_TAG ${SKIA_OPENVG_GIT_TAG}
    GIT_SHALLOW TRUE
    
    # Skia uses GN/Ninja build system
    CONFIGURE_COMMAND 
        python3 tools/git-sync-deps &&
        bin/gn gen out/Release --args='${SKIA_BUILD_ARGS}'
    
    BUILD_COMMAND 
        ninja -C out/Release
    
    BUILD_IN_SOURCE TRUE
    
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/include/skia &&
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${EXTERNAL_INSTALL_DIR}/include/skia &&
        ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/lib &&
        ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/Release/libskia.a ${EXTERNAL_INSTALL_DIR}/lib/
    
    UPDATE_COMMAND ""
    
    LOG_DOWNLOAD TRUE
    LOG_CONFIGURE TRUE
    LOG_BUILD TRUE
)