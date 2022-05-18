#pragma once
#include <stdint.h>

#define PAGING_CACHE_DISABLED   0b00010000
#define PAGING_WRITE_THROUGH    0b00001000
#define PAGING_ACESS_FROM_ALL   0b00000100
#define PAGING_IS_WRITABLE      0b00000010
#define PAGING_IS_PRESENT       0b00000001


#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024
#define PAGING_PAGE_SIZE 4096

struct paging_4gb_chunk{
        uint32_t *directory_entry;
};


struct paging_4gb_chunk *paging_new_chunk(uint8_t flags);
void paging_switch(uint32_t *directory);
uint32_t *paging_4gb_chunk_get_dir(struct paging_4gb_chunk *chunk);
uint32_t paging_set(uint32_t *dir, void *virtual_adress, uint32_t val);
uint32_t paging_get_indexes(void *virtual_adress, uint32_t *dir_index_out, uint32_t *table_index);
void paging_free_chunk(struct paging_4gb_chunk *chunk);
int paging_map_to(struct paging_4gb_chunk *dir, void *virt, void *phys, void *phys_end, int flags);
int paging_map_range(struct paging_4gb_chunk *dir, void *virt, void *phys, int count, int flags);
int paging_map(struct paging_4gb_chunk *dir, void *virt, void *phys, int flags);
uint32_t paging_set(uint32_t *dir, void *virtual_adress, uint32_t val);
uint32_t paging_get(uint32_t *directory, void *virt);


extern void enable_paging();
extern void paging_load_dir(uint32_t *directory);

