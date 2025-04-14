[org 0x700]
[bits 16]
KERNEL_SEG equ 0x1000
KERNEL_OFFSET equ 0x0 ; This is essentially the equivalent of 0x10000
ep:
    ; Getting drive data
    mov [BOOT_DRIVE], dl ; Bios sets dl to the boot drive for us
    mov ah, 8 ; Getting drive geometry
    mov dl, [BOOT_DRIVE] ; Not required, but makes code more readable
    int 0x13
    mov [HEAD_COUNT], dh
    and cl, 0b111111 ; Mask out unused data
    mov [SECTORS_PER_TRACK], cl
    mov bl, [HEAD_COUNT]
    call print_byte_bios
    ; Hello world message
    mov ebx, SECOND_STAGE_HELLO_STR
    call print_bios
    call print_nl_bios
    ; Load kernel
    push es
    mov dh, 63 ; Sector count
    mov dl, [BOOT_DRIVE]
    mov ax, KERNEL_SEG
    mov es, ax
    mov bx, KERNEL_OFFSET
    mov cl, 0x05 ; Fourth sector past the boot sector (a different segment of code takes up the boot sector, and this code takes up the three sectors after it)
    call disk_load
    pop es
    ; Print success message
    mov bx, RM_INIT_COMPLETE_STR
    call print_bios
    call print_nl_bios
    ; Switch to protected mode
    call switch_to_pm
    jmp $ ; Technically never reached, but a good practice
%include "bootloader/print.s"
%include "bootloader/disk.s"
%include "bootloader/gdt.s"
RM_INIT_COMPLETE_STR db "Real mode initialization complete", 0
SECOND_STAGE_HELLO_STR db "Successfully booted into the second stage bootloader", 0
BOOT_DRIVE db 0
SECTORS_PER_TRACK db 0
HEAD_COUNT db 0
[bits 64]
HELLO_WORLD_STR db "Atx-OS successfully booted into 64 bit long mode", 0
pm_done:
    mov rbx, HELLO_WORLD_STR
    call print_vga
    call print_nl_vga
    call KERNEL_OFFSET+(KERNEL_SEG*16)
    jmp $
; Fill with three sectors worth of zeros minus the size of the previous code
times (512*3)-($-$$) db 0