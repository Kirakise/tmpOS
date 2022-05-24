#include "process.h"
#include "../utils.h"
#include "task.h"
#include "../memory/kheap.h"
#include "../fs/file.h"
#include "../memory/paging.h"
#include "../print/print.h"
#include "../loader/formats/elfloader.h"
#include "../loader/formats/elf.h"
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

static int process_load_elf(const char *filename, struct process *process){
  int res = 0;
  
  struct elf_file *elf_file = 0;
  res = elf_load(filename, &elf_file);
  if (res < 0)
    return res;
  process->filetype = PROCESS_FILETYPE_ELF;
  process->elf_file = elf_file;
  return res;
}

static int process_load_data(const char *filename, struct process *process){
        int res = 0;
        res = process_load_elf(filename, process);
        if (res == -EINFORMAT)
          res = process_load_binary(filename, process);
        return res;
}

static uint32_t paging_align_address(uint32_t addr){
        if(addr % PAGING_PAGE_SIZE == 0)
                return addr;
        return (addr + PAGING_PAGE_SIZE - (addr % PAGING_PAGE_SIZE));
}

int process_switch(struct process *process){
  cur_process = process;
  return 0;
}

static int process_find_free_alloc_index(struct process *process){
  for (int i = 0; i < MAX_PROGRAM_ALLOCATIONS; i++){
    if (process->allocations[i].ptr == 0)
      return i;
  }
  return -ENOMEM;
}

void *process_malloc(struct process *process, size_t size){
  void *ptr = kzalloc(size);
  if (!ptr)
    return 0;
  int index = process_find_free_alloc_index(process);
  if (index < 0){
    kfree(ptr);
    return 0;
  }
  int res = paging_map_to(process->task->page_dir, ptr, ptr, paging_align_address(ptr + size),
      PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL);
  process->allocations[index].ptr = ptr;
  process->allocations[index].size = size;
  return ptr;
}

static bool is_process_ptr(struct process *process, void *ptr){
  for (int i = 0; i < MAX_PROGRAM_ALLOCATIONS; i++)
    if (process->allocations[i].ptr == ptr)
      return true;
  return false;
}


static struct process_allocation *process_get_allocation_by_addr(struct process *process, void *addr){
  for (int i = 0; i < MAX_PROGRAM_ALLOCATIONS; i++)
    if (process->allocations[i].ptr == addr)
      return &process->allocations[i];
  return 0;
}

void process_free(struct process *process, void *ptr){
  struct process_allocation *pa = process_get_allocation_by_addr(process, ptr);
  if (!pa)
    return ;

  if (paging_map_to(process->task->page_dir, pa->ptr, pa->ptr, paging_align_address(pa->ptr + pa->size),
        0x0) < 0)
    return ;


  for (int i = 0; i < MAX_PROGRAM_ALLOCATIONS; i++)
    if (process->allocations[i].ptr == ptr){
      process->allocations[i].ptr = 0;
      process->allocations[i].size = 0;
    }
  kfree(ptr);
}

int process_map_binary(struct process *process){
        return paging_map_to(process->task->page_dir, (void *)PROGRAM_VIRTUAL_ADDRESS, process->ptr,
                        (void *)paging_align_address(process->ptr + process->size), PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL | PAGING_IS_WRITABLE); 
}

int process_map_elf(struct process *process){
  int res = 0;
  struct elf_file* elf_file = process->elf_file;
  struct elf_header *header = elf_header(elf_file);
  struct elf32_phdr *phdrs = elf_pheader(header);
  for (int i = 0; i < header->e_phnum; i++){
    struct elf32_phdr *phdr = &phdrs[i];
    void *phdr_phys_addr = elf_phdr_phys_addr(elf_file, phdr);
    int flags = PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL;
    if (phdr->p_flags & PF_W)
      flags |= PAGING_IS_WRITABLE;
    res = paging_map_to(process->task->page_dir, paging_align_to_lower_page((void *)phdr->p_vaddr), 
      paging_align_to_lower_page((void *)phdr_phys_addr), (void *)paging_align_address(phdr_phys_addr + phdr->p_memsz), flags);
    if (res < 0)
      break;
  }
    return res;
}

int process_map_memory_process(struct process *process){
        int res = 0;

        if (process->filetype == PROCESS_FILETYPE_BINARY)
          res = process_map_binary(process);
        else if (process->filetype == PROCESS_FILETYPE_ELF)
          res = process_map_elf(process);
        else
          panic("process_map_memory: Invalid filetype\n", 1);
        if (res < 0) 
          return res;
        //map stack
        res = paging_map_to(process->task->page_dir, (void *)PROGRAM_VIRTUAL_STACK_ADDRESS_END, paging_align_address(process->stack),
           (void *)paging_align_address(process->stack + USER_PROGRAM_STACK_SIZE), PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL | PAGING_IS_WRITABLE);
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
                return -EISTKN;
        res = process_load_for_slot(filename, process, process_slot);
        return res;
}

int process_load_switch(const char *filename, struct process **process){
        int res = process_load(filename, process);
        if (res){
          return res;
        }
        return process_switch(*process);
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
        _process->stack = stack_ptr;
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
