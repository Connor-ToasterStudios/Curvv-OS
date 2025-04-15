[bits 16]
gdt32_start:
    dd 0x0 ; First two double words are null
    dd 0x0
gdt32_code:
    dw 0xffff ; Segment size
    dw 0x0 ; Segment base first half
    db 0x0 ; Segment base third quarter
    db 10011010b ; Flags: P = 1, DPL (bit 2 and 3) = 0, S = 1, E = 1, DC = 0, RW = 1, A = 0. Go to https://wiki.osdev.org/Global_Descriptor_Table for more info
    db 11001111b ; More flags: G = 1, DB = 1, L = 0, Reserved bit = 0 (dont touch), The rest = Segment length.
    db 0x0 ; Segment base fourth quarter
; Basically the same as the above segment, but one flag is changed (E)
gdt32_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b ; Flags: P = 1, DPL (bit 2 and 3) = 0, S = 1, E = 0, DC = 0, RW = 1, A = 0
    db 11001111b ; More flags: G = 1, DB = 0, L = 0, Reserved bit = 0 (dont touch), The rest = Segment length.
    db 0x0
gdt32_end:

gdt32_desc:
    dw gdt32_end - gdt32_start - 1 ; Size of GDT (minus one for some reason)
    dd gdt32_start ; Start location

gdt64_start:
    dd 0x0 ; First two double words are null
    dd 0x0
gdt64_code:
    dw 0xffff ; Segment size
    dw 0x0 ; Segment base first half
    db 0x0 ; Segment base third quarter
    db 10011010b ; Flags: P = 1, DPL (bit 2 and 3) = 0, S = 1, E = 1, DC = 0, RW = 1, A = 0. Go to https://wiki.osdev.org/Global_Descriptor_Table for more info
    db 10101111b ; More flags: G = 1, DB = 0, L = 1, Reserved bit = 0 (dont touch), The rest = Segment length.
    db 0x0 ; Segment base fourth quarter
; Basically the same as the above segment, but one flag is changed (E)
gdt64_data:
    dw 0xffff
    dw 0x0
    db 0x0
    db 10010010b ; Flags: P = 1, DPL (bit 2 and 3) = 0, S = 1, E = 0, DC = 0, RW = 1, A = 0
    db 10101111b ; More flags: G = 1, DB = 0, L = 1, Reserved bit = 0 (dont touch), The rest = Segment length.
    db 0x0
gdt64_end:

gdt64_desc:
    dw gdt64_end - gdt64_start - 1 ; Size of GDT (minus one for some reason)
    dd gdt64_start ; Start location
GDT32_CODE_SEG equ gdt32_code - gdt32_start
GDT32_DATA_SEG equ gdt32_data - gdt32_start
GDT64_CODE_SEG equ gdt64_code - gdt64_start
GDT64_DATA_SEG equ gdt64_data - gdt64_start
PML4T_OFFSET equ 0x1000
NO_LONG_MODE_STR db "64 bit mode is not supported by your processor!",0
YES_LONG_MODE_STR db "64 bit mode is supported by your processor",0
SWITCH_STR db "Switching to protected mode...",0
CPUID_AVAILABLE_STR db "CPUID is available",0
X64_SUCCESS_STR db "Successfully enabled 64 bit long mode", 0
switch_to_pm:
    cli ; Disable interrupts
    lgdt [gdt32_desc] ; Load gdt descriptor
    mov eax, cr0
    or eax, 0x1 ; Set 32 bit mode
    mov cr0, eax
    jmp GDT32_CODE_SEG:init_pm ; Far jump clears the CPU pipeline
[bits 32]
switch_to_lm:
    mov ebp, 0x90000
    mov esp, ebp

    ; Checking for CPUID
    pushfd ; Get flags via stack
    pop eax
    mov ecx, eax ; Save for later comparison
    xor eax, 1 << 21 ; Flip bit 21
    push eax
    popfd ; Transfer data back to flags...
    pushfd
    pop eax ; ...And then restore that data to check if the change was accepted
    push ecx
    popfd ; Restore original flag state
    cmp eax, ecx ; Check if flags changed. If they are equal, it means there was no change, and therefore CPUID is not supported
    je no_long_mode

    ; Check if long mode is supported via CPUID
    mov eax, 0x80000000
    cpuid
    cmp eax, 0x80000001
    jb no_long_mode
    mov eax, 0x80000001
    cpuid
    test edx, 1 << 29
    jz no_long_mode
    ; Initializing page tables
    mov edi, PML4T_OFFSET
    xor eax, eax
    mov ecx, 0x1000
    cld
    rep stosd
    ; Assigning pointers to lower level page tables
    mov eax, PML4T_OFFSET + 0x1000
    or eax, 1 | 2
    mov dword [PML4T_OFFSET], eax

    mov eax, PML4T_OFFSET + 0x2000
    or eax, 1 | 2 
    mov dword [PML4T_OFFSET + 0x1000], eax

    mov eax, PML4T_OFFSET + 0x3000
    or eax, 1 | 2 
    mov dword [PML4T_OFFSET + 0x2000], eax

    mov eax, PML4T_OFFSET + 0x4000
    or eax, 1 | 2 
    mov dword [PML4T_OFFSET + 0x2008], eax

    mov eax, PML4T_OFFSET + 0x3000
    mov ebx, 0x03 ; Enable present bit and rw bit
    mov ecx, 512 ; Set the loop count to 512*2
    switch_to_lm_pt_memmap_loop:
        mov [eax], ebx
        mov dword [eax + 4], 0
        add ebx, 0x1000
        add eax, 8
        loop switch_to_lm_pt_memmap_loop
    ; Set PAE and PGE
    mov eax, cr4
    or eax, 1 << 5 | 1 << 7
    mov cr4, eax
    ; Set PML4T location
    mov eax, PML4T_OFFSET
    mov cr3, eax
    ; Set MSRs
    mov ecx, 0xC0000080
    rdmsr ; This writes to eax
    or eax, 1 << 8
    wrmsr ; This reads from eax
    ; Setting CPU modes
    mov ebx, cr0
    or ebx, 1 << 31 | 1 << 0 ; Set 31st (PG) and 0th (PE) bit
    mov cr0, ebx
    ; Loading the GDT and disabling interrupts
    cli
    lgdt [gdt64_desc]
    ; Finalize
    mov ebp, 0x70000
    mov esp, ebp
    jmp GDT64_CODE_SEG:init_lm
no_long_mode:
    mov bx, NO_LONG_MODE_STR
    call print_bios
    jmp $
init_pm:
    mov ax, GDT32_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp switch_to_lm
[bits 64]
init_lm:
    mov ax, GDT64_DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp pm_done