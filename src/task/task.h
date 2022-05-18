#pragma once
#include "../kernel.h"
#include "../memory/paging.h"
#include "../memory/status.h"
#include "process.h"

struct interrupt_frame;

struct registers{
        uint32_t edi;
        uint32_t esi;
        uint32_t ebp;
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


struct process;

struct task{
        struct paging_4gb_chunk *page_dir;
        struct registers registers;
        struct process *process;
        struct task *next;
        struct task *prev;
};

void task_free(struct task *task);
struct task *task_get_next();
struct task *task_new(struct process *process);
int task_init(struct task *task, struct process *process);
struct task *task_current();

void task_return(struct registers *regs);
void restore_general_purpose_registers(struct registers *regs);
void user_registers();
int task_switch(struct task *task);
int task_page();
void task_run_first_task();

void task_current_save_state(struct interrupt_frame *frame);
int copy_string_from_task(struct task *task, void *virt, void *phys, int max);
void *task_get_stack_item(struct task *task, int index);
void task_page_task(struct task *task);
