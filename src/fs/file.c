#include "file.h"
#include "../memory/status.h"
#include "../disk/disk.h"
#include "../memory/kheap.h"
#include "../utils.h"
#include "fat/fat16.h"
#include "../kernel.h"
#include <stdint.h>

struct filesystem *filesystems[MAX_FILE_SYSTEMS];
struct file_descriptor* file_descriptors[MAX_FILE_DESCRIPTORS];

static struct filesystem **fs_get_free_filesystem(){
        int i = 0;
        for (i = 0; i < MAX_FILE_SYSTEMS; i++)
                if(filesystems[i] == 0)
                        return &filesystems[i];
        return 0;
}


void fs_insert_filesystem(struct filesystem *filesystem){
        struct filesystem **fs;
        if (filesystem == 0){
                panic("NO FILESYSTEM PROVIDED\n", 1);
        }
        fs = fs_get_free_filesystem();
        if (!fs)
                panic("COULDN'T INSERT FILESYSTEM", 1);
        *fs = filesystem;
}

static void fs_static_load(){
        fs_insert_filesystem(fat16_init());
}

void fs_load(){
        memset(filesystems, 0, sizeof(struct filesystem) * MAX_FILE_SYSTEMS);
        fs_static_load();

}

void fs_init(){
        memset(file_descriptors, 0, sizeof(file_descriptors));
        fs_load();
}


static void file_free_descriptor(struct file_descriptor *desc){
        file_descriptors[desc->index - 1] = 0;
        kfree(desc);
}

static int file_new_descriptor(struct file_descriptor **desc_out){
        for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++)
                if (file_descriptors[i] == 0){
                        struct file_descriptor *fd = kzalloc(sizeof(struct file_descriptor));
                        fd->index = i + 1;
                        file_descriptors[i] = fd;
                        *desc_out = fd;
                        return 0;
                }
        return -ENOMEM;
}


static struct file_descriptor *file_get_descriptor(int fd){
        if (fd <= 0 || fd >= MAX_FILE_DESCRIPTORS)
                return 0;

        return file_descriptors[fd - 1];
}


struct filesystem *fs_resolve(struct disk *disk){
        for (int i = 0; i < MAX_FILE_SYSTEMS; i++)
                if (filesystems[i] && !(filesystems[i]->resolve(disk)))
                        return filesystems[i];
        return 0;
}

uint32_t file_get_mode_by_string(const char *str) {
        uint32_t mode = FILE_INVALID;
        
        if (!strncmp((char *)str, "r", 1))
                mode = FILE_READ;
        else if (!strncmp((char *)str, "w", 1))
                mode = FILE_WRITE;
        else if (!strncmp((char *)str, "a", 1))
                mode = FILE_APPEND;
        return mode;
}

int fopen(const char *filename, const char *mode){
        struct path_root *root_path = parser_parse(filename, NULL);
        int res = 0;
        if (!root_path){
                res = -EINVARG;
                goto out;
        }

        if (!root_path->first){
                res =-EINVARG;
                goto out;
        }

        struct disk *disk = disk_get(root_path->drive_num);
        if (!disk){
                res = -EIO;
                goto out;
        }

        if (!disk->fs){
                res = -EIO;
                goto out;
        }

        uint32_t mode_num = file_get_mode_by_string(mode);
        if (mode_num == FILE_INVALID){
                res = -EINVARG;
                goto out;
        }

        void *decriptor_private_data = disk->fs->open(disk, root_path->first, mode_num);
        if (ISERR(decriptor_private_data)){
                res = ERROR_I(decriptor_private_data);
                goto out;
        }

        struct file_descriptor *desc = 0;
        res = file_new_descriptor(&desc);
        if (res < 0)
                goto out;
        desc->filesystem = disk->fs;
        desc->private_data = decriptor_private_data;
        desc->disk = disk;
        res = desc->index;
out:
        if (res < 0)
                res = 0;
        return res;
}

int fread(void *ptr, uint32_t size, uint32_t nmemb, int fd){
        int res = 0;

        if (size == 0 || nmemb == 0 || fd < 1)
                return -EINVARG;

        struct file_descriptor *desc = file_get_descriptor(fd);
        if (!desc)
                return -EINVARG;
        res = desc->filesystem->read(desc->disk, desc->private_data, size, nmemb, (char *)ptr);
        return res;
}

int fseek(int fd, int offset, uint32_t seek_mode){
        struct file_descriptor *desc = file_get_descriptor(fd);
        if (!desc)
                return -ENOMEM;

        return desc->filesystem->seek(desc->private_data, offset, seek_mode);
}


int fstat(int fd, struct file_stat *stat){
        struct file_descriptor *desc = file_get_descriptor(fd);
        if (!desc)
                return -EINVARG;
        return desc->filesystem->stat(desc->disk, desc->private_data, stat);
}

int fclose(int fd){
        int res;
        struct file_descriptor *desc = file_get_descriptor(fd);
        if (!desc)
                return -EINVARG;
        res = desc->filesystem->close(desc->private_data);
        if (res >= 0)
                file_free_descriptor(desc);
        return res;
}
