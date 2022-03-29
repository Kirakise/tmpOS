#include "paging.h"
#include "kheap.h"
#include "status.h"
#include "../utils.h"



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


void paging_switch(uint32_t *directory){
        paging_load_dir(directory);
        current_dir = directory;
}

uint32_t *paging_4gb_chunk_get_dir(struct paging_4gb_chunk *chunk){
        return chunk->directory_entry;
}

uint8_t paging_is_aligned(void *addr){
        return !((uint32_t)addr / PAGING_PAGE_SIZE);
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
        uint32_t *table = (uint32_t *)(entry & 0xffff0000); //get address only
        table[table_index] = val;
        return 0;
}
