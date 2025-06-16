#!/bin/bash
set -e

# Rive OpenVG Project - Simplified Baseline Setup
# Uses Ubuntu's system Skia packages for faster setup

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="${SCRIPT_DIR}/baseline_build_simple"

echo "🚀 Rive OpenVG Simplified Baseline Setup"
echo "Using system packages for faster development setup"
echo "Work directory: ${WORK_DIR}"

# Clean start
if [ -d "${WORK_DIR}" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf "${WORK_DIR}"
fi
mkdir -p "${WORK_DIR}"
cd "${WORK_DIR}"

# System dependencies including development packages
echo "📦 Installing system dependencies..."
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    clang \
    ninja-build \
    pkg-config \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libfreetype6-dev \
    libfontconfig1-dev \
    libharfbuzz-dev \
    libjpeg-dev \
    libpng-dev \
    libwebp-dev \
    zlib1g-dev

# Python symlink
if [ ! -f /usr/bin/python ]; then
    echo "🐍 Creating python symlink..."
    sudo ln -sf /usr/bin/python3 /usr/bin/python
fi

# Build Rive C++ runtime (official, no modifications)
echo "🎬 Building official Rive C++ runtime..."
git clone https://github.com/rive-app/rive-cpp.git
cd rive-cpp

# Build Rive (it will build its own Skia)
echo "⚙️  Compiling Rive runtime with built-in Skia..."
cd build
./build_rive.sh
echo "✅ Rive build complete"

# Store Rive paths
RIVE_DIR="${WORK_DIR}/rive-cpp"
RIVE_INCLUDE="${RIVE_DIR}/include"
RIVE_LIBS="${RIVE_DIR}/out/debug"
RIVE_SKIA="${RIVE_DIR}/skia"

cd "${WORK_DIR}"

# Create simple benchmark application
echo "🏃 Creating benchmark application..."
mkdir -p benchmarks
cd benchmarks

# Copy test assets if available
if [ -f "${RIVE_DIR}/../fire_button.riv" ]; then
    cp "${RIVE_DIR}/../fire_button.riv" .
fi

# Create simple CMakeLists.txt using Rive's built-in Skia
cat > CMakeLists.txt << EOF
cmake_minimum_required(VERSION 3.16)
project(RiveBaselineBenchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find system dependencies
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

# Rive paths
set(RIVE_INCLUDE_DIR "${RIVE_INCLUDE}")
set(RIVE_LIB_DIR "${RIVE_LIBS}")
set(RIVE_SKIA_DIR "${RIVE_SKIA}")

# Include directories
include_directories(
    \${RIVE_INCLUDE_DIR}
    \${RIVE_SKIA_DIR}
)

# Rive libraries (check which ones exist)
set(RIVE_LIBRARIES)
foreach(lib rive rive_harfbuzz rive_sheenbidi rive_yoga miniaudio)
    if(EXISTS "\${RIVE_LIB_DIR}/lib\${lib}.a")
        list(APPEND RIVE_LIBRARIES "\${RIVE_LIB_DIR}/lib\${lib}.a")
    endif()
endforeach()

# Platform libraries
set(PLATFORM_LIBRARIES 
    \${OPENGL_LIBRARIES} 
    \${X11_LIBRARIES} 
    GLX 
    pthread 
    dl 
    m
)

# Console benchmark (minimal)
add_executable(baseline_console_benchmark
    console_benchmark.cpp
    \${RIVE_INCLUDE_DIR}/../utils/no_op_factory.cpp
)

target_link_libraries(baseline_console_benchmark
    \${RIVE_LIBRARIES}
    \${PLATFORM_LIBRARIES}
)

# Visual benchmark (if Skia is available)
if(EXISTS "\${RIVE_SKIA_DIR}/out/Release/libskia.a")
    add_executable(baseline_visual_benchmark
        visual_benchmark.cpp
        \${RIVE_INCLUDE_DIR}/../utils/no_op_factory.cpp
    )

    target_link_libraries(baseline_visual_benchmark
        \${RIVE_LIBRARIES}
        \${RIVE_SKIA_DIR}/out/Release/libskia.a
        \${PLATFORM_LIBRARIES}
    )
    
    message(STATUS "✅ Visual benchmark enabled with Rive's Skia")
else()
    message(STATUS "ℹ️  Skia not found, building console benchmark only")
endif()

EOF

# Create simple console benchmark
cat > console_benchmark.cpp << 'EOF'
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

#include "rive/file.hpp"
#include "rive/animation/linear_animation_instance.hpp"
#include "rive/factory.hpp"
#include "utils/no_op_factory.hpp"

int main() {
    std::cout << "🎬 Rive Baseline Console Benchmark" << std::endl;
    std::cout << "Testing vanilla Rive setup" << std::endl;
    
    try {
        // Create NoOp factory for headless testing
        rive::NoOpFactory factory;
        
        // Simple performance test
        auto start = std::chrono::high_resolution_clock::now();
        
        for (int i = 0; i < 1000; i++) {
            // Basic factory operations
            auto path = factory.makeRenderPath(rive::RenderPath::RuleType::nonZero);
            auto paint = factory.makeRenderPaint();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        
        std::cout << "✅ 1000 factory operations in " << duration.count() << " μs" << std::endl;
        std::cout << "✅ Baseline setup working!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
EOF

# Create minimal visual benchmark
cat > visual_benchmark.cpp << 'EOF'
#include <iostream>
#include <chrono>

// X11 and OpenGL
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>

// Rive headers
#include "rive/file.hpp"
#include "rive/factory.hpp"
#include "utils/no_op_factory.hpp"

int main() {
    std::cout << "🎨 Rive Baseline Visual Benchmark" << std::endl;
    std::cout << "Testing vanilla Rive + OpenGL setup" << std::endl;
    
    try {
        // Basic OpenGL setup (minimal)
        Display* display = XOpenDisplay(nullptr);
        if (!display) {
            std::cerr << "❌ Cannot open X display" << std::endl;
            return -1;
        }
        
        std::cout << "✅ X11 display opened" << std::endl;
        std::cout << "✅ Rive linked successfully" << std::endl;
        
        // Test Rive factory
        rive::NoOpFactory factory;
        auto path = factory.makeRenderPath(rive::RenderPath::RuleType::nonZero);
        std::cout << "✅ Rive factory working" << std::endl;
        
        XCloseDisplay(display);
        
        std::cout << "🎉 Baseline visual benchmark setup working!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
        return -1;
    }
    
    return 0;
}
EOF

# Build benchmarks
echo "🔨 Building baseline benchmarks..."
mkdir -p build
cd build
cmake ..
make -j$(nproc)

echo ""
echo "🎉 SIMPLIFIED BASELINE SETUP COMPLETE!"
echo ""
echo "📍 Location: ${WORK_DIR}"
echo "🎬 Rive: ${RIVE_DIR}"
echo "🏃 Benchmarks: ${WORK_DIR}/benchmarks/build"
echo ""
echo "🧪 Test the baseline:"
echo "  cd ${WORK_DIR}/benchmarks/build"
echo "  ./baseline_console_benchmark"
if [ -f "./baseline_visual_benchmark" ]; then
    echo "  ./baseline_visual_benchmark"
fi
echo ""
echo "✅ Ready for OpenVG development!"
EOF