[BITS 32]

global _start
global _problem
global kernel_registers
extern kernel_start

KER_CODE_SEG equ 0x08
KER_DATA_SEG equ 0x10

_start:
        mov ax, KER_DATA_SEG ; ENTER PROTECTED MODE
        mov ds, ax
        mov es, ax
        mov fs, ax
        mov gs, ax
        mov ss, ax
        mov ebp, 0x00200000
        mov esp, ebp

        in al, 0x92 ;; A20 LINE
        or al, 2
        out 0x92, al

        ;remap the master PIC to 0x20
        mov al, 00010001b
        out 0x20, al ; tell the master we want to remap

        mov al, 0x20
        out 0x21, al ; tell the master where ISR should start

        mov al, 00000001b
        out 0x21, al ; End remap

        xor ebp, ebp

        call kernel_start

        jmp $ ; HLT

kernel_registers:
        mov ax, 0x10
        mov ds, ax
        mov es, ax
        mov gs, ax
        mov fs, ax
        ret

times 512 - ($ - $$) db 0
