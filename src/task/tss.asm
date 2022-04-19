section .asm

global tss_load

tss_load:
        push ebp
        mov ebp, esp

        mov ax, [ebp + 8]
        ltr ax

        pop ebp
        ret


global walk_stack
walk_stack:
    ; Create stack frame & save caller's EDI and EBX.
    push ebp
    mov  ebp,       esp
    sub  esp,       8
    mov  [ebp - 4], edi
    mov  [ebp - 8], ebx
    ; Set up local registers.
    xor  eax,       eax         ; EAX = return value (number of stack frames found).
    mov  ebx,       [esp +  8]  ; EBX = old EBP.
    mov  edi,       [esp + 16]  ; Destination array pointer in EDI.
    mov  ecx,       [esp + 20]  ; Maximum array size in ECX.
.walk:
    ; Walk backwards through EBP linked list, storing return addresses in EDI array.
    test ebx,       ebx
    jz   .done
    mov  edx,       [ebx +  4]  ; EDX = previous stack frame's IP.
    mov  ebx,       [ebx +  0]  ; EBX = previous stack frame's BP.
    mov  [edi],     edx         ; Copy IP.
    add  edi,       4
    inc  eax
    loop .walk
.done:
    ; Restore caller's EDI and EBX, leave stack frame & return EAX.
    mov  edi,       [ebp - 4]
    mov  ebx,       [ebp - 8]
    leave
    ret
