#!/bin/bash
set -e

# Rive OpenVG Project - Baseline Setup Using Existing Working Build
# This leverages the already-working Rive + benchmarks we have

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
WORK_DIR="${SCRIPT_DIR}/baseline_build_existing"
EXISTING_RIVE="/home/jcrigby/work/rivehack/rive-cpp"
EXISTING_BENCHMARKS="/home/jcrigby/work/rivehack/hello-rive"

echo "🚀 Rive OpenVG Baseline Setup (Using Existing)"
echo "Leveraging existing working Rive + benchmarks"
echo "Work directory: ${WORK_DIR}"

# Verify existing builds exist
if [ ! -d "${EXISTING_RIVE}" ]; then
    echo "❌ Existing Rive not found at: ${EXISTING_RIVE}"
    echo "Please build Rive first or update the path"
    exit 1
fi

if [ ! -d "${EXISTING_BENCHMARKS}" ]; then
    echo "❌ Existing benchmarks not found at: ${EXISTING_BENCHMARKS}"
    echo "Please build benchmarks first or update the path"
    exit 1
fi

# Clean start
if [ -d "${WORK_DIR}" ]; then
    echo "🧹 Cleaning previous build..."
    rm -rf "${WORK_DIR}"
fi
mkdir -p "${WORK_DIR}"
cd "${WORK_DIR}"

echo "📦 Ensuring system dependencies..."
sudo apt-get update >/dev/null 2>&1 || true
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    libgl1-mesa-dev \
    libx11-dev \
    pkg-config >/dev/null 2>&1 || true

# Copy existing working Rive
echo "🎬 Copying existing Rive build..."
cp -r "${EXISTING_RIVE}" ./rive-cpp-baseline
RIVE_DIR="${WORK_DIR}/rive-cpp-baseline"

# Copy existing working benchmarks
echo "🏃 Copying existing benchmarks..."
cp -r "${EXISTING_BENCHMARKS}" ./benchmarks-baseline
BENCHMARKS_DIR="${WORK_DIR}/benchmarks-baseline"

# Update benchmark paths to use our copied Rive
cd "${BENCHMARKS_DIR}"
echo "🔧 Updating benchmark configuration..."

# Create updated CMakeLists.txt pointing to our baseline Rive
cat > CMakeLists_baseline.txt << EOF
cmake_minimum_required(VERSION 3.16)
project(RiveBaselineBenchmarks)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find system dependencies
find_package(OpenGL REQUIRED)
find_package(X11 REQUIRED)

# Use our baseline Rive build
set(RIVE_INCLUDE_DIR "${RIVE_DIR}/include")
set(RIVE_LIB_DIR "${RIVE_DIR}/out/debug")
set(RIVE_UTILS_DIR "${RIVE_DIR}/utils")

# Include directories
include_directories(
    \${RIVE_INCLUDE_DIR}
)

# Rive libraries
set(RIVE_LIBRARIES 
    \${RIVE_LIB_DIR}/librive.a
    \${RIVE_LIB_DIR}/librive_harfbuzz.a
    \${RIVE_LIB_DIR}/librive_sheenbidi.a
    \${RIVE_LIB_DIR}/librive_yoga.a
    \${RIVE_LIB_DIR}/libminiaudio.a
)

# Platform libraries
set(PLATFORM_LIBRARIES 
    \${OPENGL_LIBRARIES} 
    \${X11_LIBRARIES} 
    GLX 
    pthread 
    dl 
    m
)

# Console benchmark
add_executable(baseline_console_benchmark
    console_benchmark.cpp
    \${RIVE_UTILS_DIR}/no_op_factory.cpp
)

target_link_libraries(baseline_console_benchmark
    \${RIVE_LIBRARIES}
    \${PLATFORM_LIBRARIES}
)

# Visual benchmark
add_executable(baseline_visual_benchmark
    visual_main.cpp
    \${RIVE_UTILS_DIR}/no_op_factory.cpp
)

target_link_libraries(baseline_visual_benchmark
    \${RIVE_LIBRARIES}
    \${PLATFORM_LIBRARIES}
)

EOF

# Build the baseline benchmarks
echo "🔨 Building baseline benchmarks..."
mkdir -p build_baseline
cd build_baseline
cmake .. -DCMAKE_BUILD_TYPE=Release -f ../CMakeLists_baseline.txt
make -j$(nproc)

echo ""
echo "🎉 BASELINE SETUP COMPLETE!"
echo ""
echo "📍 Location: ${WORK_DIR}"
echo "🎬 Rive (copied): ${RIVE_DIR}"
echo "🏃 Benchmarks: ${BENCHMARKS_DIR}/build_baseline"
echo ""
echo "🧪 Test the baseline:"
echo "  cd ${BENCHMARKS_DIR}/build_baseline"
echo "  ./baseline_console_benchmark"
echo "  ./baseline_visual_benchmark"
echo ""
echo "📊 Compare performance:"
echo "  Original: cd ${EXISTING_BENCHMARKS} && ./run_benchmark.sh"
echo "  Baseline: cd ${BENCHMARKS_DIR}/build_baseline && ./baseline_visual_benchmark"
echo ""
echo "✅ Ready to add OpenVG backend to this working baseline!"

# Create a simple test script
cat > ${WORK_DIR}/test_baseline.sh << 'EOF'
#!/bin/bash
echo "🧪 Testing Baseline Setup"
cd "$(dirname "$0")/benchmarks-baseline/build_baseline"

echo "Running console benchmark..."
./baseline_console_benchmark

echo ""
echo "Running visual benchmark..."
./baseline_visual_benchmark

echo ""
echo "✅ Baseline working!"
EOF

chmod +x ${WORK_DIR}/test_baseline.sh

echo ""
echo "🚀 Quick test: ${WORK_DIR}/test_baseline.sh"