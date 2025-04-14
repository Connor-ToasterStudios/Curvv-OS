[bits 64]
global loadIdt
global remapPic
global outPort
global inPort
global asm_cli
global asm_sti
extern print
extern printnl
extern main
extern printint
call main
jmp $
loadIdt:
    lidt [rdi]
    sti
    ret
remapPic:
    mov al, 0x11 ; 0x11 is the initialize command
    out PIC1, al
    out PIC2, al
    ; The next three pieces of output data correspond to PIC settings as follows
    ; 1: Vector offset
    ; 2: Wiring (is it a master or slave?)
    ; 3: Extra environment info
    mov al, 32 ; Set the vector offset to 32
    out PIC1DATA, al
    add al, 8
    out PIC2DATA, al ; 8 above the PIC1 offset
    mov al, 4 ; Tell PIC1 where the slave chip is
    out PIC1DATA, al
    mov al, 2 ; Notify PIC2 that it is in cascade mode (or that it is a slave)
    out PIC2DATA, al
    mov al, 1 ; Tell the PICs to enter 8086/88 mode
    out PIC1DATA, al
    out PIC2DATA, al
    ; Remove masking
    mov al, 0
    out PIC1DATA, al
    out PIC2DATA, al

    ret
; Param 1: IO port
; Param 2: Value
outPort:
    mov rdx, rdi
    mov rax, rsi
    out dx, eax
    ret
; Param 1: IO port
inPort:
    mov rdx, rdi
    in eax, dx
    ret ; Note that rax holds the return value, so no additional instructions are needed to properly return a value from the instruction
asm_cli:
	cli
	ret
asm_sti:
	sti
	ret
PIC1 equ 0x20 ; Note that these two constants are also the command ports
PIC2 equ 0xA0
PIC1DATA equ PIC1 + 1
PIC2DATA equ PIC2 + 1
TEST_STR db "Hello", 0
IDT_SUCCESS_STR db "Idt successfully loaded", 0
%include "kernel/interrupts/idthandlers.s"