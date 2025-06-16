# Skia with OpenVG backend external project

set(SKIA_OPENVG_GIT_URL "https://github.com/jcrigby/skia-openvg.git")
set(SKIA_OPENVG_GIT_TAG "main")

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
    
    # For now, just copy headers since we have skia-lite
    CONFIGURE_COMMAND ""
    
    BUILD_COMMAND ""
    
    BUILD_IN_SOURCE TRUE
    
    INSTALL_COMMAND
        ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/include/skia &&
        ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${EXTERNAL_INSTALL_DIR}/include/skia &&
        ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/lib &&
        ${CMAKE_COMMAND} -E touch ${EXTERNAL_INSTALL_DIR}/lib/libskia.a
    
    UPDATE_COMMAND ""
    
    LOG_DOWNLOAD TRUE
    LOG_CONFIGURE TRUE
    LOG_BUILD TRUE
)