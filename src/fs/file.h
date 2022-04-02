#pragma once
#include "parser.h"

struct disk;
struct file_stat;

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

enum{
        FILE_STAT_READ_ONLY = 0xb00000001
};
typedef uint32_t FILE_STAT_FLAGS;

typedef void *(*FS_OPEN_FUNCTION)(struct disk *disk, struct path_part *path, uint32_t mode);
typedef int (*FS_RESOLVE_FUNCTION)(struct disk *disk);
typedef int (*FS_READ_FUNCTION)(struct disk *disk, void *private_data, uint32_t size, uint32_t nmemb, char *out);
typedef int (*FS_SEEK_FUNCTION)(void *private_data, uint32_t offset, uint32_t seek_mode);
typedef int (*FS_STAT_FUNCTION)(struct disk *disk, void *private_data, struct file_stat *stat);
typedef int (*FS_CLOSE_FUNCTION)(void *private_data);
struct filesystem{
        FS_RESOLVE_FUNCTION resolve;
        FS_OPEN_FUNCTION open;
        FS_READ_FUNCTION read;
        FS_SEEK_FUNCTION seek;
        FS_STAT_FUNCTION stat;
        FS_CLOSE_FUNCTION close;

        char name[20];
};

struct file_descriptor{
        uint32_t index;
        struct filesystem *filesystem;

        void *private_data;
        //Disk the desc should be used on
        struct disk *disk;
};

struct file_stat{
        FILE_STAT_FLAGS flags;
        uint32_t file_size;
};


void fs_init();
int fopen(const char *filename, const char *mode);
void fs_insert_filesystem(struct filesystem *filesystem);
struct filesystem *fs_resolve(struct disk *disk);
int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd);
int fseek(int fd, int offset, uint32_t seek_mode);
int fstat(int fd, struct file_stat *stat);
int fclose(int fd);
