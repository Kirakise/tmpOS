#include "task.h"
#include "../utils.h"
#include "../memory/kheap.h"
#include "../kernel.h"
#include "../print/print.h"
#include "../idt/idt.h"

//Running task
struct task *curr_task = 0;

//For linked list
struct task *task_tail = 0;
struct task *task_head = 0;


struct task *task_current(){
        return curr_task;
}

int task_switch(struct task *task){
        curr_task = task;
        paging_switch(task->page_dir->directory_entry);
        return 0;
}

int task_save_state(struct task *task, struct interrupt_frame *frame){
  task->registers.ip = frame->ip;
  task->registers.cs = frame->cs;
  task->registers.flags = frame->flags;
  task->registers.esp = frame->esp;
  task->registers.ss = frame->ss;
  task->registers.eax = frame->eax;
  task->registers.ebp = frame->ebp;
  task->registers.ecx = frame->ecx;
  task->registers.edi = frame->edi;
  task->registers.edx = frame->edx;
  task->registers.esi = frame->esi;
  return 0;
}

void task_current_save_state(struct interrupt_frame *frame){
  if (task_current() == 0)
    panic("No current task to save", 1);
  
  struct task* task = task_current();
  task_save_state(task, frame);
}


int copy_string_from_task(struct task *task, void *virtual, void *phys, int max){
  if (max >= PAGING_PAGE_SIZE)
    return -EINVARG;
  int res = 0;
  char *tmp = kzalloc(max);
  if (!tmp)
    return -ENOMEM;
  uint32_t *task_directory = task->page_dir->directory_entry;
  uint32_t old_entry = paging_get(task_directory, tmp);
  paging_map(task_directory, tmp, tmp, PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL);
  paging_switch(task->page_dir->directory_entry);
  strncpy(tmp, virtual, max);
  kernel_page();

  res = paging_set(task_directory, tmp, old_entry);
  if (res < 0){
    kfree(tmp);
    return -EIO;
  }
  strncpy(phys, tmp, max);
  return 0;
}

int task_page(){
        user_registers();
        task_switch(curr_task);
        return 0;
}

void task_run_first_task(){
        if (!curr_task)
                panic("No first task to run\n", 1);
        task_switch(task_head);
        printk("%p\n", task_head);
        //hexdump(PROGRAM_VIRTUAL_STACK_ADDRESS_START, 10);
        task_return(&task_head->registers);
}

int task_init(struct task *task, struct process *process){
        memset(task, 0, sizeof(struct task));
        //Gib task it's own paging table
        task->page_dir = paging_new_chunk(PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL);
        if (task->page_dir == 0)
                return -ENOMEM;
        task->registers.ip = PROGRAM_VIRTUAL_ADDRESS;
        task->registers.ss = USER_DATA_SEGMENT;
        task->registers.cs = USER_CODE_SEGMENT;
        task->registers.esp = PROGRAM_VIRTUAL_STACK_ADDRESS_START;
        task->process = process;


        return 0;
}

struct task *task_new(struct process *process){
        int res = 0;
        struct task *task = kzalloc(sizeof(struct task));
        if (!task)
                return ERROR(-ENOMEM);
        res = task_init(task, process);
        if (res != 0){
                kfree(task);
                return ERROR(res);
        }

        if (task_head == 0){
                task_head = task;
                task_tail = task;
                curr_task = task;
                return task;
        }

        task_tail->next = task;
        task->prev = task;
        task_tail = task;
        return task;
}

struct task *task_get_next(){
        if (!curr_task->next)
                return task_head;
        return curr_task->next;
}


static void task_list_remove(struct task *task){
        if (task->prev)
                task->prev->next = task->next;
        if (task == task_head)
                task_head = task->next;
        if (task == task_tail)
                task_tail = task->prev;

        if (task == curr_task)
                curr_task = task_get_next();
}

void task_free(struct task *task){
        paging_free_chunk(task->page_dir);
        task_list_remove(task);

        kfree(task);
}

void task_page_task(struct task *task){
  user_registers();
  paging_switch(task->page_dir->directory_entry);
}


void *task_get_stack_item(struct task *task, int index){
        void *result = 0;
        uint32_t *sp_ptr = (uint32_t *) task->registers.esp;
        task_page_task(task);
        result = (void *)sp_ptr[index];
        kernel_page();
        return result;
}
