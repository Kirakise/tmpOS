#include "process.h"
#include "../utils.h"
#include "task.h"
#include "../memory/kheap.h"
#include "../fs/file.h"
#include "../memory/paging.h"
struct process *cur_process = 0;

struct process *processes[MAX_PROCESSES];

static void process_init(struct process *process){
        memset(process, 0, sizeof(struct process));
}


struct process *process_current(){
        return cur_process;
}

struct process *process_get(int process_id){
        if (process_id < 0 || process_id > MAX_PROCESSES)
                return NULL;
        return  processes[process_id];
}


static int process_load_binary(const char *filename, struct process *process){
        int res = 0;
        int fd = fopen(filename, "r");
        if (!fd)
                return -EIO;

        struct file_stat stat;
        res = fstat(fd, &stat);
        if (res != 0)
                return res;
        void *programm_data_ptr = kzalloc(stat.file_size);
        if (!programm_data_ptr)
                return -ENOMEM;
        if (fread(programm_data_ptr, stat.file_size, 1, fd) != 1){
                kfree(programm_data_ptr);
                return -EIO;
        }
        process->ptr = programm_data_ptr;
        process->size = stat.file_size;
        fclose(fd);
        return 0;
}

static int process_load_data(const char *filename, struct process *process){
        int res = 0;
        res = process_load_binary(filename, process);
        return res;
}

static uint32_t paging_align_address(uint32_t addr){
        if(addr % PAGING_PAGE_SIZE == 0)
                return addr;
        return (addr + PAGING_PAGE_SIZE - (addr % PAGING_PAGE_SIZE));
}

int process_map_binary(struct process *process){
        return paging_map_to(process->task->page_dir->directory_entry, (void *)PROGRAM_VIRTUAL_ADDRESS, process->ptr,
                        (void *)paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL | PAGING_IS_WRITABLE); 
}

int process_map_memory_process(struct process *process){
        int res = 0;
        res = process_map_binary(process);
        return res;
}

int process_get_free_slot(){
        for (int i = 0; i < MAX_PROCESSES; i++)
                if (processes[i] == 0)
                        return i;
        return -EISTKN;
}

int process_load(const char *filename, struct process **process){
        int res = 0;
        int process_slot = process_get_free_slot();
        if (process_slot == -EISTKN)
                return -ENOMEM;
        res = process_load_for_slot(filename, process, process_slot);
        return res;
}

int process_load_for_slot(const char *filename, struct process **process, int process_slot){
        int res = 0;
        struct task *task = 0;
        struct process *_process = 0;
        void *stack_ptr = 0;

        if (process_get(process_slot))
                return -EISTKN;
        _process = kzalloc(sizeof(struct process));
        if (!_process)
                return -ENOMEM;
        process_init(_process);
        if(process_load_data(filename, _process)){
                kfree(_process);
                return -EIO;
        }

        stack_ptr = kzalloc(USER_PROGRAM_STACK_SIZE);
        if (!stack_ptr){
                kfree(_process);
                kfree(stack_ptr);
                return -ENOMEM;
        }

        strncpy(_process->filename, filename, sizeof(_process->filename));
        _process->id = process_slot;
        //Create Task now
        task = task_new(_process);
        if (ERROR_I(task) == 0){
                kfree(_process);
                kfree(stack_ptr);
                return ERROR_I(task);
        }

        _process->task = task;
        res = process_map_memory_process(_process);
        if (res < 0){
                kfree(_process);
                kfree((stack_ptr));
                return res;
        }
        *process = _process;
        processes[process_slot] = _process;
        return 0;
}
