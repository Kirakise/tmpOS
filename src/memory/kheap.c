#include "kheap.h"
#include "../utils.h"
#include "heap.h"
#include "paging.h"
#include "../kernel.h"

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

void *kzalloc(uint32_t size){
        char *tmp = kmalloc(size);
        if (!tmp)
                return 0;
        memset(tmp, 0x0, size);
        return tmp;
}

void *kmalloc(uint32_t size){
        return heap_malloc(size, &kernel_heap);
}

void kfree(void *ptr) {
        heap_free(ptr, &kernel_heap);
}

uint32_t ksize(const void *ptr){
  uint32_t ret = 0;
  if (!ptr)
    return 0;

  for (uint32_t i = ((uint32_t)(ptr - kernel_heap.saddr) / BLOCK_SIZE); kernel_heap.table->entries[i] & HEAP_BLOCK_HAS_NEXT; ++i)
    ret += BLOCK_SIZE;
  return ret;
}

void vfree(void *ptr){
  heap_free(ptr, &kernel_heap);
}

void *vmalloc(uint32_t size){
  return heap_malloc(size, &kernel_heap);
}

uint32_t vsize(const void *ptr){
  uint32_t ret = 0;
  if (!ptr)
    return 0;
  void *ptr1 = paging_get_physical_address(kernel_chunk->directory_entry, ptr);
  

  for (uint32_t i = ((uint32_t)(ptr1 - kernel_heap.saddr) / BLOCK_SIZE); kernel_heap.table->entries[i] & HEAP_BLOCK_HAS_NEXT; ++i)
    ret += BLOCK_SIZE;
  return ret;
}
