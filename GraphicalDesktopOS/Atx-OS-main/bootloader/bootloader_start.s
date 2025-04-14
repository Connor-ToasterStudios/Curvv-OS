[org 0x7c00]
mov [BOOT_DRIVE], dl ; dl is automatically set to the boot drive
mov dh, 3 ; Sector count
mov bx, 0x700 ; Buffer location
mov cl, 0x02 ; First sector after the boot sector
call disk_load
mov bx, SECOND_STAGE_DISK_LOAD_STR
call print_bios
call print_nl_bios
jmp 0x700
mov bx, INVALID_FLOW_STR
mov dl, [BOOT_DRIVE]
call print_bios
jmp $ ; Not needed, but good practice
BOOT_DRIVE db 0
SECOND_STAGE_DISK_LOAD_STR db "Successfully loaded the second stage bootloader, attempting to boot into it...", 0
INVALID_FLOW_STR db "Execution has reached an invalid area!", 0
%include "bootloader/print.s"
%include "bootloader/disk.s"
; Fill with 510 zeros minus the size of the previous code
times 510-($-$$) db 0
; Magic number
dw 0xaa55