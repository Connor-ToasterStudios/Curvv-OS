#!/bin/bash
# AMOS Desktop OS - Main Build Script

# Set compiler flags
CFLAGS="-Wall -Wextra -g -O2"
LDFLAGS="-lm"

# Create build directory structure
mkdir -p build
mkdir -p build/core
mkdir -p build/core/graphics
mkdir -p build/core/3d
mkdir -p build/desktop
mkdir -p build/desktop/integration
mkdir -p build/kernel

echo "Building AMOS Desktop OS..."

# First, build the native rendering system
echo "Building native rendering system..."
./build_native.sh

# Compile kernel files
echo "Compiling kernel..."
gcc $CFLAGS -c kernel/kernel.c -o build/kernel/kernel.o

# Compile desktop integration files
echo "Compiling desktop integration..."
gcc $CFLAGS -c desktop/integration/desktop_init.c -o build/desktop/integration/desktop_init.o

# Link everything into the final executable
echo "Linking AMOS Desktop OS..."
gcc -o bin/amos-desktop build/kernel/kernel.o build/desktop/integration/desktop_init.o build/libamos_renderer.a $LDFLAGS

# Check if build was successful
if [ $? -eq 0 ]; then
    echo "Build successful! Executable is at bin/amos-desktop"
    # Make executable
    chmod +x bin/amos-desktop
else
    echo "Build failed!"
    exit 1
fi