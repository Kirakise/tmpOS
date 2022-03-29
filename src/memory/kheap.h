#pragma once
#include "heap.h"

extern struct heap kernel_heap;
extern struct heap_table kernel_heap_table;

#define HEAP_SIZE 104857600 //100MB
#define BLOCK_SIZE 4096
#define HEAP_ADDRESS 0x1000000
#define HEAP_TABLE_ADDRESS 0x0007E00


void *kmalloc(uint32_t size);
void kheap_init();
void kfree(void *ptr);
void *kzalloc(uint32_t size);
