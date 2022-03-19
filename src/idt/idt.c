#include "idt.h"
#include "../utils.h"
#include "../io/io.h"

struct idt_desc idt_descs[TOTAL_IDESCS];
struct idtr_desc idtr_desr;

extern void idt_load(struct idtr_desc *ptr);
extern void int21h();
extern void no_interrupt();

void no_interrupt_handler(){
        outb(0x20, 0x20);
}

void int21h_handler()
{
        print("Keyboard pressed\n");
        outb(0x20, 0x20);
}


void idt_zero(){
        print("Division by zero error\n");
}

void idt_set(int intNum, void *adress)
{
        struct idt_desc *desc = &idt_descs[intNum];

        desc->offset_1 = (uint32_t) adress & 0x0000ffff;
        desc->selector = 0x08; //KER_CODE
        desc->zero = 0;
        desc->type_attr = 0b11101110;
        desc->offset_2 = (uint32_t) adress >> 16;
}

void idt_init()
{
        memset(idt_descs, 0, sizeof(idt_descs));
        idtr_desr.limit = sizeof(idt_descs) - 1;
        idtr_desr.base = (uint32_t)idt_descs;
        for (int i = 0; i < TOTAL_IDESCS; i++)
                idt_set(i, no_interrupt);
        idt_set(0, idt_zero);
        idt_set(0x21, int21h);
        idt_load(&idtr_desr);
}
