section .asm

global idt_load
global int21h
global no_interrupt
global disable_interrupts
global enable_interrupts
global isr80h_wrapper
extern isr80h_handler
extern int21h_handler
extern no_interrupt_handler

idt_load:
        push ebp
        mov ebp, esp
        
        mov ebx, [ebp + 8]
        lidt [ebx]

        pop ebp
        ret

int21h:
        pushad
        call int21h_handler
        popad
        sti
        iret

no_interrupt:
        pushad
        call no_interrupt_handler 
        popad
        sti
        iret
        
enable_interrupts:
        sti
        ret

disable_interrupts:
        cli
        ret

isr80h_wrapper:
        ; ip cs flags sp and ss are already pushed by interrupt
        pushad ; but we still need to push general purpose registers

        push esp

        push eax
        call isr80h_handler
        mov dword[tmp_res], eax
        add esp, 8

        popad
        mov eax, [tmp_res]
        iretd

section .data
tmp_res: dd 0
