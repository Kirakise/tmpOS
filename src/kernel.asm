[BITS 32]

global _start

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

        jmp $ ; HLT
