#pragma once

#include <stddef.h>
#include <stdint.h>

#define HEAP_BLOCK_TABLE_ENTRY_TAKEN    0x01
#define HEAP_BLOCK_TABLE_ENTRY_FREE     0x00


#define HEAP_BLOCK_HAS_NEXT             0b10000000
#define HEAP_BLOCK_IS_FIRST             0b01000000

struct heap_table{
        uint8_t* entries;
        uint32_t total;
};


struct heap{
        struct heap_table *table;
        void *saddr;
};


int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table);
void heap_free(void *ptr, struct heap *heap);
void *heap_malloc(uint32_t size, struct heap *heap);
