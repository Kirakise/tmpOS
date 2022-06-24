#pragma once
#include <stdint.h>
#include "../fs/file.h"

#define DISK_TYPE_REAL 0
#define DISK_TYPE_VIRTUAL 1
#define SECTOR_SIZE 512

struct disk{
        uint32_t type;
        uint32_t sector_size;
        uint32_t id;

        struct filesystem *fs;
        //private data of filesystem;
        void *fs_private;
};

struct disk *disk_get(int index);
int disk_read_block(struct disk *idisk, int lba, int total, void *buf);
int disk_write_block(struct disk *idisk, int lba, int total, void *buf);

void disk_search_and_init();

