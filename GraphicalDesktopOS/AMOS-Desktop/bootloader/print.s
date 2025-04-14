[bits 16]
; ebx = location of string
print_bios:
    pusha
    mov ah, 0x0e
    pb_loop:
        mov al, [ebx]
        cmp al, 0
        je pb_done
        int 0x10
        inc ebx
        jmp pb_loop
    pb_done:
        popa
        ret
print_nl_bios:
    push bx
    mov bx, NEWLINE
    call print_bios
    pop bx
    ret
; bl = char
put_char_bios:
    push ax
    mov al, bl
    mov ah, 0x0e
    int 0x10
    pop ax
    ret
; bl = number, as well as return value
get_hex_char_from_int:
    cmp bl, 10
    jb get_hex_char_from_int_lower
    get_hex_char_from_int_higher:
        add bl, 55 ; Sub 10, Add 65
        ret
    get_hex_char_from_int_lower:
        add bl, 48
        ret
; bl = byte to print
print_byte_bios:
    push bx
    mov bl, '0'
    call put_char_bios
    mov bl, 'x'
    call put_char_bios
    pop bx
    pusha
    mov al, bl
    mov ah, 0
    mov cl, 16
    div cl ; This divides ax by 16
    mov bl, al
    call get_hex_char_from_int
    call put_char_bios ; Note that, conveniently, the return value of get_hex_char_from_int's return value is stored in bl, and put_char_bios takes bl as its parameter
    mov bl, ah
    call get_hex_char_from_int
    call put_char_bios
    popa
    ret
[bits 64]
; rbx = location of string
print_vga:
    push rax
    push rbx
    push rdx
    pv_loop:
        mov ah, 0x0f ; White on black
        mov al, [rbx]
        cmp al, 0
        je pv_done
        mov rdx, [VIDEO_MEMORY]
        mov [rdx], ax
        inc rbx
        add qword [VIDEO_MEMORY], 2
        inc qword [TEXT_POS_X]
        jmp pv_loop
    pv_done:
        pop rdx
        pop rbx
        pop rax
        ret
print_nl_vga:
    push rcx
    mov cl, [TEXT_POS_X]
    add dword [VIDEO_MEMORY], 160
    sub [VIDEO_MEMORY], cl
    sub [VIDEO_MEMORY], cl
    mov dword [TEXT_POS_X], 0
    inc dword [TEXT_POS_Y]
    pop rcx
    ret
VIDEO_MEMORY dq 0xb8000
TEXT_POS_X dq 0x0
TEXT_POS_Y dq 0x0
NEWLINE db 0x0a, 0x0d, 0
print equ print_vga
print_nl equ print_nl_vga