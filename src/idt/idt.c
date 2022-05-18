#include "idt.h"
#include "../utils.h"
#include "../io/io.h"
#include "../task/task.h"

struct idt_desc idt_descs[TOTAL_IDESCS];
struct idtr_desc idtr_desr;

extern void idt_load(struct idtr_desc *ptr);
extern void int21h();
extern void no_interrupt();
extern void isr80h_wrapper();

static ISR80H_COMMAND isr80h_commands[MAX_ISR80H_COMMANDS];

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
        idt_set(0x80, isr80h_wrapper);
        idt_load(&idtr_desr);
}

void isr80h_register_command(int command_id, ISR80H_COMMAND command){
  if (command_id < 0 || command_id >= MAX_ISR80H_COMMANDS)
    panic("The command is out of bounds\n", 1);
  if (isr80h_commands[command_id])
    panic("The commands is already taken", 1);
  isr80h_commands[command_id] = command;
}

void *isr80h_handle_command(int command, struct interrupt_frame *frame){
  void *result = 0;

  if (command < 0 || command >= MAX_ISR80H_COMMANDS){
    return 0;
  }

  ISR80H_COMMAND command_func = isr80h_commands[command];
  if (!command_func){
    return 0;
  }
  result = command_func(frame);
  return result;
}

void *isr80h_handler(int command, struct interrupt_frame *frame){
  void *res = 0;
  kernel_page();
  task_current_save_state(frame);
  res = isr80h_handle_command(command, frame);
  task_page();
  return res;
}
