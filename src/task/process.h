#pragma once
#include <stdint.h>
#include "../kernel.h"
#include "../keyboard/keyboard.h"
#include <stddef.h>

#define PROCESS_FILETYPE_ELF 0
#define PROCESS_FILETYPE_BINARY 1

typedef uint8_t PROCESS_FILETYPE;

struct process_allocation{
  void *ptr;
  size_t size;
};

struct process{
        uint16_t id;

        char filename[FILE_MAX_PATH];

        struct task *task;
        //all allocations made by process
        struct process_allocation allocations[MAX_PROGRAM_ALLOCATIONS];

        PROCESS_FILETYPE filetype;

        union {
          //physical pointer to the proccess memory
          void *ptr;
          struct elf_file *elf_file;
        };
                //physical pointer to the stack memory
        void *stack;
        //Size of data pointed by ptr
        uint32_t size;

        struct keyboard_buffer{
          char buffer[KEYBOARD_BUFFER_SIZE];
          uint32_t tail;
          uint32_t head;
        } keyboard;
};

int process_load_for_slot(const char *filename, struct process **process, int process_slot);
int process_load(const char *filename, struct process **process);
int process_load_switch(const char *filename, struct process **process);
int process_switch(struct process *process);
void *process_malloc(struct process *process, size_t size);
void process_free(struct process *process, void *ptr);

