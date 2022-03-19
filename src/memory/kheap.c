#include "kheap.h"
#include "../utils.h"
#include "heap.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;


void kheap_init(){
        size_t total_number_entries = HEAP_SIZE / BLOCK_SIZE;
        kernel_heap_table.entries = (uint8_t *)HEAP_TABLE_ADDRESS;
        kernel_heap_table.total = total_number_entries; 

        void *end = (void *)( HEAP_ADDRESS + HEAP_SIZE);
        if(heap_create(&kernel_heap, (void *)HEAP_ADDRESS, end, &kernel_heap_table) < 0)
                print("Error creating kernel heap\n");
}


void *kmalloc(uint32_t size){
        return heap_malloc(size, &kernel_heap);
}

void kfree(void *ptr) {
        heap_free(ptr, &kernel_heap);
}
