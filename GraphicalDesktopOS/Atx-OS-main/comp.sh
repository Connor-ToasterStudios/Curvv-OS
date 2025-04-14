gcc -m64 kernel/kernel.c -c -fno-pie -o kernel/kernel.o -ffreestanding -Ikernel -Os -s -fno-ident
nasm -felf64 kernel/kernel_asm.s
nasm -fbin bootloader/ep.s
nasm -fbin bootloader/bootloader_start.s
ld kernel/kernel_asm.o kernel/kernel.o -Ttext 0x10000 -o kernel/kernel -melf_x86_64 --oformat=binary -e main
cat bootloader/bootloader_start bootloader/ep > bootloader/boot
cat bootloader/boot kernel/kernel > atxos.bin
rm -f kernel/kernel.o
rm -f kernel/kernel_asm.o