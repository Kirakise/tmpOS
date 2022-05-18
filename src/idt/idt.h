#pragma once
#include <stdint.h>


#define TOTAL_IDESCS 512
#define MAX_ISR80H_COMMANDS 1024
#define MAX_INTERRUPTS 512
typedef void (*INTERRUPT_CALLBACK_FUNC)();
struct idt_desc
{
        uint16_t offset_1; //0-15
        uint16_t selector;
        uint8_t zero;
        uint8_t type_attr;
        uint16_t offset_2; //16-31
} __attribute__((packed)); 


struct idtr_desc
{
        uint16_t limit;
        uint32_t base;
} __attribute__((packed));


struct interrupt_frame{
  uint32_t edi;
  uint32_t esi;
  uint32_t ebp;
  uint32_t reserved; //esp which pushad pushed to stack, don't care about it
  uint32_t ebx;
  uint32_t edx;
  uint32_t ecx;
  uint32_t eax;
  uint32_t ip;
  uint32_t cs;
  uint32_t flags;
  uint32_t esp;
  uint32_t ss;
}__attribute__((packed));


typedef void*(*ISR80H_COMMAND)(struct interrupt_frame *frame);

void idt_init();
extern void enable_interrupts();
extern void disable_interrupts();
void isr80h_register_command(int command_id, ISR80H_COMMAND command);
int idt_register_interrupt_callback(int interrupt, INTERRUPT_CALLBACK_FUNC interrupt_callback);
