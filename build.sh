#!/bin/bash

# C++ Code Review Agent Build Script
# This script helps build the project with proper error handling

set -e  # Exit on error

echo "╔══════════════════════════════════════════════════════════════════════╗"
echo "║      C++ Code Review Agent - Build Script                       ║"
echo "╚══════════════════════════════════════════════════════════════════════╝"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: CMake is not installed${NC}"
    echo "Please install CMake 3.15 or later"
    exit 1
fi

# Check for C++ compiler
if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo -e "${RED}Error: No C++ compiler found${NC}"
    echo "Please install GCC or Clang"
    exit 1
fi

echo -e "${GREEN}✓ CMake found: $(cmake --version | head -n1)${NC}"

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}Build directory exists. Cleaning...${NC}"
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Configuring CMake..."
echo "-------------------"

# Configure with CMake
if cmake ..; then
    echo -e "${GREEN}✓ CMake configuration successful${NC}"
else
    echo -e "${RED}✗ CMake configuration failed${NC}"
    echo ""
    echo "Common issues:"
    echo "1. LLVM/Clang not installed or not found"
    echo "   Ubuntu/Debian: sudo apt-get install llvm-14-dev clang-14 libclang-14-dev"
    echo "   macOS: brew install llvm"
    echo ""
    echo "2. LLVM/Clang path not set"
    echo "   Try setting: export LLVM_DIR=/path/to/llvm/lib/cmake/llvm"
    exit 1
fi

echo ""
echo "Building project..."
echo "-------------------"

# Build
NPROC=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
if make -j$NPROC; then
    echo ""
    echo -e "${GREEN}╔══════════════════════════════════════════════════════════════════════╗${NC}"
    echo -e "${GREEN}║              Build Successful!                                   ║${NC}"
    echo -e "${GREEN}╚══════════════════════════════════════════════════════════════════════╝${NC}"
    echo ""
    echo "Executable: $(pwd)/cpp-agent"
    echo ""
    echo "Try it out:"
    echo "  ./cpp-agent --help"
    echo "  ./cpp-agent scan ../examples/test_code.cpp"
    echo ""
else
    echo -e "${RED}✗ Build failed${NC}"
    exit 1
fi
