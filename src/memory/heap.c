#include "heap.h"
#include "kheap.h"
#include <stdbool.h>
#include "status.h"
#include "../utils.h"





struct heap uheap;

static bool heap_validate_table(void *ptr, void *end, struct heap_table *table){
        uint32_t table_size = (uint32_t)(end - ptr);
        uint32_t total_blocks = table_size / BLOCK_SIZE;
        if (table->total != total_blocks)
                return -EINVARG;
        return false;
}

static inline bool heap_validate_aligment(void *ptr){
        return ((uint32_t) ptr % BLOCK_SIZE) == 0;
}

int heap_create(struct heap *heap, void *ptr, void *end, struct heap_table *table){
        if (!heap_validate_aligment(ptr) || !heap_validate_aligment(end)) //Not aligned
                return -EINVARG;

        memset(heap, 0, sizeof(struct heap));
        heap->saddr = ptr;
        heap->table = table;

        if (heap_validate_table(ptr, end, table)) //Bad table
                return -EINVARG;

        memset(table->entries, HEAP_BLOCK_TABLE_ENTRY_FREE, table->total * sizeof(uint8_t)); //All blocks are free to use now

        return 0;
}

static uint32_t heap_align_value(uint32_t val) {
        if(val % BLOCK_SIZE == 0)
                return val;
        return (val / BLOCK_SIZE + 1) * BLOCK_SIZE;
}


static inline void *block_to_addr(uint32_t block, struct heap *heap){
        return (void *)(heap->saddr + block * BLOCK_SIZE);
}


void mark_blocks_taken(uint32_t start, uint32_t n, struct heap *heap)
{
        heap->table->entries[start] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
        for (uint32_t i = start; i < start + n - 1; i++)
                heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT; 
        heap->table->entries[start + n - 1] = HEAP_BLOCK_TABLE_ENTRY_TAKEN; //no next block for the last one
}

void *heap_malloc_blocks(uint32_t total_blocks, struct heap *heap){
        for (uint32_t i = 0; i < heap->table->total; i++){
                if ((heap->table->entries[i] & 0x0f) != 0)
                        continue;
                for (uint32_t j = i; j < heap->table->total; j++) {
                        if ((heap->table->entries[j] & 0x0f) != 0){
                                i = j;
                                break;
                        }
                        else if (j - i + 1 == total_blocks){
                                mark_blocks_taken(i, total_blocks, heap);
                                return block_to_addr(i, heap);
                        }
                }
        }
        return 0;
}

void *heap_malloc(uint32_t size, struct heap *heap){
        uint32_t tmp = heap_align_value(size) / BLOCK_SIZE;
        return heap_malloc_blocks(tmp, heap);
}

void heap_free(void *ptr, struct heap *heap){
        for (uint32_t i = ((uint32_t)(ptr - heap->saddr) / BLOCK_SIZE); heap->table->entries[i] & HEAP_BLOCK_HAS_NEXT; ++i){
                heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        }
}
