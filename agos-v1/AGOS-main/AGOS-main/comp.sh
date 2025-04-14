# Compile graphics components
gcc -m64 kernel/graphics/*.c -c -fno-pie -o kernel/graphics.o -ffreestanding -Ikernel -lm

# Compile UI toolkit
gcc -m64 kernel/ui/ui_toolkit.c -c -fno-pie -o kernel/ui_toolkit.o -ffreestanding -Ikernel

# Compile multitasking
gcc -m64 kernel/multitask.c -c -fno-pie -o kernel/multitask.o -ffreestanding -Ikernel

# Compile UI components
gcc -m64 kernel/ui/*.c -c -fno-pie -o kernel/ui.o -ffreestanding -Ikernel

# Compile window system
gcc -m64 kernel/window.c kernel/bitmap.c kernel/disk_io.c -c -fno-pie -o kernel/window.o -ffreestanding -Ikernel

# Compile font and UI components
gcc -m64 kernel/font.c kernel/ui/ui.c kernel/app_manager.c -c -fno-pie -o kernel/ui_components.o -ffreestanding -Ikernel

# Compile core kernel
gcc -m64 kernel/kernel.c -c -fno-pie -o kernel/kernel.o -ffreestanding -Ikernel

# Assemble bootloader and kernel assembly
nasm -felf64 kernel/kernel_asm.s
nasm -fbin bootloader/ep.s
nasm -fbin bootloader/bootloader_start.s

# Link everything together
ld kernel/kernel_asm.o kernel/kernel.o kernel/graphics.o kernel/ui.o -Ttext 0x10000 -o kernel/kernel -melf_x86_64 --oformat=binary -e main

# Create final binary
cat bootloader/bootloader_start bootloader/ep > bootloader/boot
cat bootloader/boot kernel/kernel > atxos.bin

# Cleanup
rm -f kernel/*.o