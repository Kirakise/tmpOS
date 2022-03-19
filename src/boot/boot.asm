org 0x7c00


[BITS 16]
jmp 0:_start


KER_CODE_SEG equ gdt_ker_code - gdt_start
KER_DATA_SEG equ gdt_ker_data - gdt_start
USR_CODE_SEG equ gdt_usr_code - gdt_start
USR_DATA_SEG equ gdt_usr_data - gdt_start


_start:
        jmp short start
        nop

times 33 db 0 ; for bios not to ruin the bootloader

start:
        jmp 0:_true_start



;times 0x800 - ($ - $$) db 0

gdt_start:
gdt_null:
        dd 0x0
        dd 0x0

gdt_ker_code: ; CS here pls
        dw 0xffff ;limit 0-15 bits
        dw 0; base 0-15 bits
        db 0; base 16-23 bits
        db 0x9a ; Acessbyte
        db 0xCF ; flags and limit
        db 0 ; base 24-31

gdt_ker_data: ; DS, SS, ES, FS, GS
        dw 0xffff ;limit 0-15 bits
        dw 0; base 0-15 bits
        db 0; base 16-23 bits
        db 0x92 ; Acessbyte
        db 0xCF ; flags and limit
        db 0 ; base 24-31

gdt_usr_code: ; DS, SS, ES, FS, GS
        dw 0xffff ;limit 0-15 bits
        dw 0; base 0-15 bits
        db 0; base 16-23 bits
        db 0xFE ; Acessbyte
        db 0xCF ; flags and limit
        db 0 ; base 24-31

gdt_usr_data: ; DS, SS, ES, FS, GS
        dw 0xffff ;limit 0-15 bits
        dw 0; base 0-15 bits
        db 0; base 16-23 bits
        db 0xF6 ; Acessbyte
        db 0xCF ; flags and limit
        db 0 ; base 24-31

gdt_end:


gdt_descriptor:
        dw gdt_end - gdt_start - 1
        dd gdt_start



_true_start:
        cli ; clear interupts
        mov ax, 0x00
        mov ds, ax
        mov es, ax
        mov ss, ax ;stack offset 0x0
        mov sp, 0x7c00 ;stack segment 0x7c00, basicaly stack continues form 0x7c00 to 0x0
        sti ; return the interupts
        
.load_protected:
        cli
        lgdt[gdt_descriptor]
        mov eax, cr0
        or eax, 0x1 
        mov cr0, eax
        jmp KER_CODE_SEG:load32


[BITS 32]
load32:
        mov eax, 1
        mov ecx, 100
        mov edi, 0x0100000
        call ata_lba_read
        jmp KER_CODE_SEG:0x0100000

ata_lba_read:
        mov ebx, eax
        shr eax, 24 ; send highest 8 bits to disk controller
        or eax, 0xE0 ; master drive
        mov dx, 0x1F6 ; port or smth, remember to read OSDEV ATA for info
        out dx, al ; now they are sent

        mov eax, ecx ; send amount of sectors
        mov dx, 0x1F2
        out dx, al ; now  they are sent

        mov eax, ebx ; more bits of the LBA
        mov dx, 0x1F3
        out dx, al

        mov dx, 0x1F4 ; restore the buckup
        mov eax, ebx
        shr eax, 8
        out dx, al

        mov dx, 0x1F5
        mov eax, ebx
        shr eax, 16
        out dx, al

        mov dx, 0x1F7
        mov al, 0x20
        out dx, al


.next_sector: ;read all segments into memory
        push ecx

.try_again: ;check if we need to read
        mov dx, 0x1F7
        in al, dx
        test al, 8
        jz .try_again

        mov ecx, 256 ;read 256 bits from segment
        mov dx, 0x1F0
        rep insw ; do insw instruction 256 times from 0x1F0
        pop ecx
        loop .next_sector
        ; end of reading
        ret
        


;[BITS 16]


times 510 - ($ - $$) db 0
dw 0xAA55

