# Rive C++ runtime external project

set(RIVE_CPP_GIT_URL "https://github.com/rive-app/rive-cpp.git")
set(RIVE_CPP_GIT_TAG "main")

if(USE_LOCAL_RIVE)
    # Use local Rive build
    if(NOT LOCAL_RIVE_PATH)
        message(FATAL_ERROR "LOCAL_RIVE_PATH must be set when USE_LOCAL_RIVE is ON")
    endif()
    
    # Verify the path exists
    if(NOT EXISTS "${LOCAL_RIVE_PATH}")
        message(FATAL_ERROR "LOCAL_RIVE_PATH does not exist: ${LOCAL_RIVE_PATH}")
    endif()
    
    # Create a dummy target that just copies files
    ExternalProject_Add(rive-cpp
        SOURCE_DIR "${LOCAL_RIVE_PATH}"
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        BUILD_IN_SOURCE TRUE
        
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/include/rive &&
            ${CMAKE_COMMAND} -E copy_directory ${LOCAL_RIVE_PATH}/include ${EXTERNAL_INSTALL_DIR}/include/rive &&
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/src/rive &&
            ${CMAKE_COMMAND} -E copy_directory ${LOCAL_RIVE_PATH}/utils ${EXTERNAL_INSTALL_DIR}/src/rive/utils &&
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/lib &&
            ${CMAKE_COMMAND} -E copy ${LOCAL_RIVE_PATH}/out/debug/librive.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy ${LOCAL_RIVE_PATH}/out/debug/librive_harfbuzz.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy ${LOCAL_RIVE_PATH}/out/debug/librive_sheenbidi.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy ${LOCAL_RIVE_PATH}/out/debug/librive_yoga.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy ${LOCAL_RIVE_PATH}/out/debug/libminiaudio.a ${EXTERNAL_INSTALL_DIR}/lib/
        
        UPDATE_COMMAND ""
    )
elseif(USE_SYSTEM_RIVE)
    # Use system-installed Rive
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RIVE REQUIRED rive-cpp)
    
    # Create imported target
    add_library(rive-cpp INTERFACE IMPORTED)
    set_target_properties(rive-cpp PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${RIVE_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${RIVE_LIBRARIES}"
    )
else()
    # Build Rive from source
    ExternalProject_Add(rive-cpp
        GIT_REPOSITORY ${RIVE_CPP_GIT_URL}
        GIT_TAG ${RIVE_CPP_GIT_TAG}
        GIT_SHALLOW FALSE
        GIT_SUBMODULES_RECURSE TRUE
        
        # Rive uses custom build system
        CONFIGURE_COMMAND ""
        
        BUILD_COMMAND
            bash -c "cd <SOURCE_DIR>/build && ./build_rive.sh"
        
        BUILD_IN_SOURCE TRUE
        
        INSTALL_COMMAND
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/include/rive &&
            ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include ${EXTERNAL_INSTALL_DIR}/include/rive &&
            ${CMAKE_COMMAND} -E make_directory ${EXTERNAL_INSTALL_DIR}/lib &&
            ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/debug/librive.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/debug/librive_harfbuzz.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/debug/librive_sheenbidi.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/debug/librive_yoga.a ${EXTERNAL_INSTALL_DIR}/lib/ &&
            ${CMAKE_COMMAND} -E copy <SOURCE_DIR>/out/debug/libminiaudio.a ${EXTERNAL_INSTALL_DIR}/lib/
        
        UPDATE_COMMAND ""
        
        LOG_DOWNLOAD TRUE
        LOG_BUILD TRUE
    )
endif()