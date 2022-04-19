#include "task.h"
#include "../utils.h"
#include "../memory/kheap.h"
#include "../kernel.h"
#include "../print/print.h"

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

int task_page(){
        user_registers();
        task_switch(curr_task);
        return 0;
}

void task_run_first_task(){
        if (!curr_task)
                panic("No first task to run\n", 1);
        task_switch(task_head);
        printk("%i", task_head->registers.ss);
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
