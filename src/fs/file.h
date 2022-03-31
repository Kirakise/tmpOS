#pragma once
#include "parser.h"

struct disk;

#define MAX_FILE_SYSTEMS 12
#define MAX_FILE_DESCRIPTORS 512

enum{
        SEEK_SET,
        SEEK_CUR,
        SEEK_END
};


enum{
        FILE_READ,
        FILE_WRITE,
        FILE_APPEND,
        FILE_INVALID
};

typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, struct path_part *path, uint32_t mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk *disk);

struct filesystem{
        FS_RESOLVE_FUNCTION resolve;
        FS_OPEN_FUNCTION open;

        char name[20];
};

struct file_descriptor{
        uint32_t index;
        struct filesystem *filesystem;

        void *private_data;
        //Disk the desc should be used on
        struct disk *disk;
};


void fs_init();
int fopen(const char *filename, const char *mode);
void fs_insert_filesystem(struct filesystem *filesystem);
struct filesystem *fs_resolve(struct disk *disk);
