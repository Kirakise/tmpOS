#include "fat16.h"
#include "../../memory/status.h"
#include "../../utils.h"
#include "../../disk/disk.h"
#include "../../memory/status.h"
#include "../../memory/kheap.h"
#include "../../disk/streamer.h"


#define FAT16_SIG 0x29
#define FAT16_FAT_ENTRY_SIZE 0x02
#define FAT16_BAD_SECTOR 0xFF7
#define FAT16_UNUSED 0x00


#define FAT_ITEM_TYPE_DIRECTORY 0
#define FAT_ITEM_TYPE_FILE 1

#define FAT_FILE_READ_ONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME_LABLE 0x08
#define FAT_FILE_SUBDIRECTORY 0x10
#define FAT_FILE_ARCHIVED 0x20
#define FAT_FILE_DEVICE 0x40
#define FAT_FILE_RESERVED 0x80


struct fat_header_ext{
        uint8_t drive_num;
        uint8_t win_nt_bit;
        uint8_t sig;
        uint32_t volume_id;
        uint8_t volume_id_string[11];
        uint8_t system_id_string[8];
}__attribute__((packed));


struct fat_header{
        uint8_t short_jump_ins[3];
        uint8_t oem_id[8];
        uint16_t bytes_per_sector;
        uint8_t sectors_per_cluser;
        uint16_t reserved_sectors;
        uint8_t fat_copies;
        uint16_t rood_dir_entries;
        uint16_t num_of_sectors;
        uint8_t media_type;
        uint16_t sectors_per_fat;
        uint16_t sectors_per_track;
        uint16_t num_of_heads;
        uint32_t hidden_sectors;
        uint32_t sectors_big;
}__attribute__((packed));


struct fat_h{
        struct fat_header primary_header;
        union fat_h_e{
                struct fat_header_ext ext_header;
        } shared;
};

struct fat_directory_item{
        uint8_t filename[8];
        uint8_t ext[3];
        uint8_t attr;
        uint8_t reserved;
        uint8_t creation_time_tenths_of_sec;
        uint16_t creation_time;
        uint16_t creation_date;
        uint16_t last_access;
        uint16_t high_16_bits_first_cluster;
        uint16_t last_mod_time;
        uint16_t last_mod_date;
        uint16_t low_16_bits_first_cluster;
        uint32_t filesize;
}__attribute__((packed));


struct fat_directory{
        struct fat_directory_item *item;
        uint32_t total;
        uint32_t sector_pos;
        uint32_t ending_sector_pos;
};

struct fat_item{
        union{
                struct fat_directory_item *item;
                struct fat_directory *directory;
        };


        uint32_t type;
};


struct fat_item_descriptor{
        struct fat_item *item;
        uint32_t pos;
};


struct fat_private{
        struct fat_h header;
        struct fat_directory root_dir;
        //stream things
        struct disk_stream *cluster_read_stream;
        struct disk_stream *fat_read_stream;
        struct disk_stream *directory_stream;
};

int fat16_resolve(struct disk *disk);
void *fat16_open(struct disk *disk, struct path_part *path, uint32_t mode);

struct filesystem fat16_fs = {
        .resolve = fat16_resolve,
        .open = fat16_open
};


struct filesystem *fat16_init(){
        strcpy(fat16_fs.name, "FAT16");
        return &fat16_fs;
}

static void fat16_init_private(struct disk *disk, struct fat_private *private){
        memset(private, 0, sizeof(struct fat_private));
        private->cluster_read_stream = disk_streamer_new(disk->id); 
        private->fat_read_stream = disk_streamer_new(disk->id);
        private->directory_stream = disk_streamer_new(disk->id);
}


int fat16_sector_to_absolute(struct disk *disk, uint32_t sector){
        return sector * disk->sector_size;
}

int fat16_get_total_items_for_directory(struct disk *disk, uint32_t directory_start_sector){
        struct fat_directory_item item;
        struct fat_directory_item empty_item;

        memset(&empty_item, 0, sizeof(struct fat_directory_item));

        struct fat_private *private = disk->fs_private;
        uint32_t i = 0;
        int directory_start_pos = directory_start_sector * disk->sector_size;
        struct disk_stream *stream = private->directory_stream;
        if(disk_stream_seek(stream, directory_start_pos) != 0)
                return -EIO;
        while(1){
                if (disk_stream_read(stream, &item, sizeof(item)) != 0)
                        return -EIO;
                if (item.filename[0] == 0)
                        return i;
                if (item.filename[0] == 0xE5)
                        continue;
                ++i;
        }
}

int fat_16_get_root_directory(struct disk *disk, struct fat_private *private, struct fat_directory *dir){
        struct fat_header *header = &private->header.primary_header;
        int root_dir_sector_pos = (header->fat_copies * header->sectors_per_fat) + header->reserved_sectors;
        int root_dir_entries = private->header.primary_header.rood_dir_entries;
        int root_dir_size = root_dir_entries * sizeof(struct fat_directory_item);
        int total_sectors = root_dir_size / disk->sector_size;
        if (root_dir_size % disk->sector_size)
                total_sectors += 1;
        int total_items = fat16_get_total_items_for_directory(disk, root_dir_sector_pos);
        struct fat_directory_item *dir_items = kzalloc(root_dir_size);
        if (!dir_items)
                return -ENOMEM;
        struct disk_stream *stream = private->directory_stream;
        if (disk_stream_seek(stream, fat16_sector_to_absolute(disk, root_dir_sector_pos)) != 0)
                return -EIO;

        if (disk_stream_read(stream, dir_items, root_dir_size) != 0)
                return -EIO;

        dir->item = dir_items;
        dir->total = total_items;
        dir->sector_pos = root_dir_sector_pos;
        dir->ending_sector_pos = root_dir_sector_pos + root_dir_size / disk->sector_size;
        return 0;
}

int fat16_resolve(struct disk *disk){
        int res = 0;
        struct fat_private *fat_private = kzalloc(sizeof(struct fat_private));
        fat16_init_private(disk, fat_private);
        disk->fs_private = fat_private;
        disk->fs = &fat16_fs;
        struct disk_stream *stream = disk_streamer_new(disk->id);
        if (!stream){
                res = -ENOMEM;
                goto out;
        }

        if (disk_stream_read(stream, &fat_private->header, sizeof(fat_private->header)) != 0){
                res = -EIO;
                goto out;
        }

        if (fat_private->header.shared.ext_header.sig != FAT16_SIG){
                res = -EFSNOTUS;
                goto out;
        }

        if (fat_16_get_root_directory(disk, fat_private, &fat_private->root_dir) != 0){
                res = -EIO;
                goto out;
        }
        
out:
        if (stream)
                disk_stream_close(stream);
        if (res < 0){
                kfree(fat_private);
                disk->fs_private = 0;
        }
        return res; 
}

void *fat16_open(struct disk *disk, struct path_part *path, uint32_t mode){
       return 0; 
}
