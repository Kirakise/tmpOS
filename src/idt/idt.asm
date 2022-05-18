section .asm

global idt_load
global no_interrupt
global disable_interrupts
global enable_interrupts
global isr80h_wrapper
global interrupt_pointer_table
extern isr80h_handler
extern int21h_handler
extern no_interrupt_handler
extern interrupt_handler

idt_load:
        push ebp
        mov ebp, esp
        
        mov ebx, [ebp + 8]
        lidt [ebx]

        pop ebp
        ret

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

%macro interrupt 1
        global int%1
        int%1:
            pushad
            push esp
            push dword %1
            call interrupt_handler
            add esp, 8
            popad
            iret
%endmacro

%assign i 0
%rep 512
  interrupt i
%assign i i + 1
%endrep

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

%macro interrupt_array_entry 1
        dd int%1
%endmacro

interrupt_pointer_table:
%assign i 0
%rep 512
        interrupt_array_entry i
%assign i i + 1
%endrep
