#!/bin/bash
# AMOS Desktop OS - Native Renderer Build Script

# Create output directories
mkdir -p build/core/graphics
mkdir -p build/core/3d

# Set compiler flags
CFLAGS="-Wall -Wextra -g -O2 -I."
ASFLAGS="-f elf32"
LDFLAGS="-lm"

echo "Building AMOS Desktop OS Native Rendering System..."

# Compile framebuffer implementation
echo "  Compiling core/graphics/framebuffer.c..."
gcc $CFLAGS -c core/graphics/framebuffer.c -o build/core/graphics/framebuffer.o

# Compile window system
echo "  Compiling core/graphics/window.c..."
gcc $CFLAGS -c core/graphics/window.c -o build/core/graphics/window.o

# Compile 3D renderer
echo "  Compiling core/3d/renderer3d.c..."
gcc $CFLAGS -c core/3d/renderer3d.c -o build/core/3d/renderer3d.o

# Assemble 3D renderer assembly optimizations
echo "  Assembling core/3d/renderer3d_asm.s..."
nasm $ASFLAGS core/3d/renderer3d_asm.s -o build/core/3d/renderer3d_asm.o

# Link everything into a static library
echo "  Creating libamos_renderer.a..."
ar rcs build/libamos_renderer.a \
    build/core/graphics/framebuffer.o \
    build/core/graphics/window.o \
    build/core/3d/renderer3d.o \
    build/core/3d/renderer3d_asm.o

echo "Build complete. Library available at build/libamos_renderer.a"