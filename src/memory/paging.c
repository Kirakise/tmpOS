#include "paging.h"
#include "kheap.h"
#include "status.h"
#include "../utils.h"
#include "../print/print.h"
#include <stdint.h>
#include "../kernel.h"



uint32_t *current_dir = 0;

struct paging_4gb_chunk *paging_new_chunk(uint8_t flags){
        uint32_t *directory = kzalloc(sizeof(uint32_t) * 1024);
        uint32_t offset = 0;

        for (uint32_t i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++){
                uint32_t *entry = kzalloc(sizeof(uint32_t) * PAGING_TOTAL_ENTRIES_PER_TABLE);
                for (uint32_t j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
                        entry[j] = (offset + (j * PAGING_PAGE_SIZE)) | flags;
                offset += PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE;
                directory[i] = (uint32_t)entry | flags | PAGING_IS_WRITABLE;
        }
        struct paging_4gb_chunk *chunk = kzalloc(sizeof(struct paging_4gb_chunk));
        chunk->directory_entry = directory;
        return chunk;
}


void paging_free_chunk(struct paging_4gb_chunk *chunk){
        for (int i = 0; i < 1024; i++){
                uint32_t entry = chunk->directory_entry[i];
                uint32_t *table = (uint32_t *)(entry & 0xfffff000);
                kfree(table);
        }
        kfree(chunk->directory_entry);
        kfree(chunk);
}


void paging_switch(uint32_t *directory){
        paging_load_dir(directory);
        current_dir = directory;
}

uint32_t *paging_4gb_chunk_get_dir(struct paging_4gb_chunk *chunk){
        return chunk->directory_entry;
}

uint8_t paging_is_aligned(void *addr){
        return !((uint32_t)addr % PAGING_PAGE_SIZE);
}

void *paging_align_to_lower_page(void *addr){
  uint32_t _addr = (uint32_t)addr;
  _addr -= _addr % PAGING_PAGE_SIZE;
  return (void *)addr;
}

uint32_t paging_get_indexes(void *virtual_adress, uint32_t *dir_index_out, uint32_t *table_index){
        if (!paging_is_aligned(virtual_adress))
                return -EINVARG;
        *dir_index_out = ((uint32_t)virtual_adress / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE));
        *table_index = ((uint32_t)virtual_adress % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGING_PAGE_SIZE) / PAGING_PAGE_SIZE);
        return 0;
}


uint32_t paging_set(uint32_t *dir, void *virtual_adress, uint32_t val){
        if (!paging_is_aligned(virtual_adress))
                return -EINVARG;
        uint32_t table_index = 0;
        uint32_t dir_index = 0;
        if (paging_get_indexes(virtual_adress, &dir_index, & table_index))
                return -EINVARG;
        uint32_t entry = dir[dir_index];
        uint32_t *table = (uint32_t *)(entry & 0xfffff000); //get address only
        table[table_index] = val;
        return 0;
}

int paging_map(struct paging_4gb_chunk *dir, void *virt, void *phys, int flags){
        if (((uint32_t)virt % PAGING_PAGE_SIZE) || ((uint32_t)phys % PAGING_PAGE_SIZE))
                return -EINVARG;
        return paging_set(dir->directory_entry, virt, (uint32_t)phys | flags);
}

int paging_map_range(struct paging_4gb_chunk *dir, void *virt, void *phys, int count, int flags){
        int res = 0;
        for (int i = 0; i < count; i++){
                res = paging_map(dir, virt, phys, flags);
                if (res < 0)
                        break;
                virt += PAGING_PAGE_SIZE;
                phys += PAGING_PAGE_SIZE;
        }
        return res;
}

int paging_map_to(struct paging_4gb_chunk *dir, void *virt, void *phys, void *phys_end, int flags){
        int res;
        if ((uint32_t)virt % PAGING_PAGE_SIZE)
                return -EINVARG;
        if ((uint32_t)phys_end % PAGING_PAGE_SIZE)
                return -EINVARG;
        if ((uint32_t)phys % PAGING_PAGE_SIZE)
                return -EINVARG;
        if ((uint32_t)phys_end < (uint32_t)phys)
                return -EINVARG;
        uint32_t total_bytes = phys_end - phys;
        int total_pages = total_bytes / PAGING_PAGE_SIZE;
        res = paging_map_range(dir, virt, phys, total_pages, flags);
        return res;
}

uint32_t paging_get(uint32_t *directory, void *virt){
  uint32_t dir_index = 0;
  uint32_t table_index = 0;
  int res = paging_get_indexes(virt, &dir_index, &table_index);
  uint32_t entry = directory[dir_index];
  uint32_t *table = (uint32_t*)(entry & 0xfffff000);
  return table[table_index];
}


void* paging_get_physical_address(uint32_t* directory, void* virt)
{
    void* virt_addr_new = (void*) paging_align_to_lower_page(virt);
    void* difference = (void*)((uint32_t) virt - (uint32_t) virt_addr_new);
    return (void*)((paging_get(directory, virt_addr_new) & 0xfffff000) + difference);
}
