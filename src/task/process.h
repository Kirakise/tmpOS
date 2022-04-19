#pragma once
#include <stdint.h>
#include "../kernel.h"

struct process{
        uint16_t id;

        char filename[FILE_MAX_PATH];

        struct task *task;
        //all allocations made by process
        void *allocations[MAX_PROGRAM_ALLOCATIONS];
        //physical pointer to the proccess memory
        void *ptr;
        //physical pointer to the stack memory
        void *stack;
        //Size of data pointed by ptr
        uint32_t size;
};

int process_load_for_slot(const char *filename, struct process **process, int process_slot);
int process_load(const char *filename, struct process **process);

