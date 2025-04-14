[bits 16]
; dh = sectors to be read
; dl = drive (implicitly read by the BIOS interrupt, and therefore not found in the below code)
; es:bx = buffer location (also implicitly used by BIOS)
; cl = sector (yet again implicitly used by BIOS). Note that the second sector is the first sector past the boot sector
; Note that drive 0x80 is the first hard disk drive
; This file depends on print.s
disk_load:
    pusha
    push dx
    mov ah, 0x02 ; Interrupt number
    mov al, dh ; Set sector count
    mov ch, 0x00 ; Zeroth cylinder
    mov dh, 0x00 ; Zeroth head
    ; Remember that dl is passed as an argument, but is also the registry the interrupt reads. Therefore it isn't in this code
    int 0x13 ; Bios read interrupt
    jc disk_err ; Jump if error (stored in carry bit)
    pop dx
    cmp al, dh
    jne sect_err
    popa
    ret
disk_err:
    mov bx, DISK_ERR_STR
    call print
    call print_nl
    jmp disk_err_loop
sect_err:
    mov bx, SECT_ERR_STR
    call print
    call print_nl
    jmp disk_err_loop ; Technically not needed, but makes the code more readable
disk_err_loop:
    jmp $
; THE CODE BELOW DOES NOT WORK
; ax = sectors to be read
; dl = drive (implicitly used by BIOS)
; ebx = buffer
; ecx = start
disk_load_lba:
    pusha
    lba_data_packet:
        db 16 ; Size of packet
        db 0 ; Unused space (always zero)
        lba_data_packet_sectors: dw 0 ; Number of sectors
        lba_data_packet_buffer: dd 0 ; Buffer
        lba_data_packet_lba_num: dd 0 ; LBA start lower 32 bits
        dd 0 ; This is technically the higher 16 bits, but will probably never be used
    mov [lba_data_packet_sectors], ax
    mov [lba_data_packet_buffer], ebx
    mov [lba_data_packet_lba_num], ecx
    push cs
    pop ds
    mov si, lba_data_packet ; Set the packet to be read from
    mov ah, 0x42 ; Interrupt number
    int 0x13
    jc disk_err
    popa
    ret
DISK_ERR_STR db "Disk error!", 0 ; This should probably be changed at some point
SECT_ERR_STR db "Sector error!", 0 ; Look at above comment