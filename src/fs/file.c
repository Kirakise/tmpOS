#include "file.h"
#include "../memory/status.h"
#include "../memory/kheap.h"
#include "../utils.h"

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
                //panic();
        }
        fs = fs_get_free_filesystem();
        if (!fs)
                //panic
        *fs = filesystem;
}

static void fs_static_load(){
        //fs_insert_filesystem(fat16_init());
}

void fs_load(){
        memset(filesystems, 0, sizeof(filesystems));
        fs_static_load();

}

void fs_init(){
        memset(file_descriptors, 0, sizeof(file_descriptors));
        fs_load();
}


static int file_new_descriptor(struct file_descriptor **desc_out){
        for (int i = 0; i < MAX_FILE_DESCRIPTORS; i++)
                if (file_descriptors[i] == 0){
                        struct file_descriptor *fd = kzalloc(sizeof(struct file_descriptor));
                        fd->index = i + 1;
                        file_descriptors[i] = fd;
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
                if (filesystems[i] != 0 && !filesystems[i]->resolve(disk))
                        return filesystems[i];
        return 0;
}
