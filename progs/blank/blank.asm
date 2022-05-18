[BITS 32]


global __start

__start:

label:
        push str
        mov eax, 1
        int 0x80
        add esp, 4

        jmp $


section .data
str: db 'Some String', 0
