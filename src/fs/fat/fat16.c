#include "fat16.h"
#include "../../memory/status.h"
#include "../../utils.h"
#include "../../disk/disk.h"
#include "../../memory/status.h"
#include "../../memory/kheap.h"
#include "../../disk/streamer.h"
#include "../../kernel.h"
#include "../../task/tss.h"
#include "../../print/print.h"


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


struct fat_file_descriptor{
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
int fat16_read(struct disk *disk, void *desc, uint32_t size, uint32_t nmemb, char *out_ptr);
int fat16_seek(void *private, uint32_t offset, uint32_t seek_mode);
int fat16_stat(struct disk *disk, void *private, struct file_stat *stat);
int fat16_close(void *private);

struct filesystem fat16_fs = {
        .resolve = fat16_resolve,
        .open = fat16_open,
        .read = fat16_read,
        .seek = fat16_seek,
        .stat = fat16_stat,
        .close = fat16_close
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


void fat16_to_proper_string(char **out, const char *in){
        while (*in != 0 && *in != 0x20){
                **out = *in;
                *out += 1;
                ++in;
        }

        if (*in == 0x20)
                **out = 0;
}

void fat16_get_full_relative_filename(struct fat_directory_item *item, char *out, uint32_t max_len){
        memset(out, 0, max_len);
        char *out_tmp = out;
        fat16_to_proper_string(&out_tmp, (const char *)item->filename);
        if (item->ext[0] != 0x0 && item->ext[0] != 0x20){
                *out_tmp++='.';
                fat16_to_proper_string(&out_tmp, (const char *) item->ext);
        }
}


struct fat_directory_item *fat16_clone_directory_item(struct fat_directory_item *item, uint32_t size){
        struct fat_directory_item *item_copy = 0;
        if (size < sizeof(struct fat_directory_item))
                return item_copy;

        item_copy = kzalloc(size);
        if (!item_copy)
                return 0;

        memcpy(item_copy, item, size);
        return item_copy;
}

static uint32_t fat16_get_first_cluster(struct fat_directory_item *item){
        return (item->high_16_bits_first_cluster | item->low_16_bits_first_cluster);
}

static int fat16_cluster_to_sector(struct fat_private *private, int cluster){
        return private->root_dir.ending_sector_pos + ((cluster - 2) * private->header.primary_header.sectors_per_cluser);
}


static uint32_t fat16_get_first_fat_sector(struct fat_private *private){
        return private->header.primary_header.reserved_sectors;
}

static int fat16_get_fat_entry(struct disk *disk, int cluster){
        int res = -1;
        struct fat_private *private = disk->fs_private;
        struct disk_stream *stream = private->fat_read_stream;
        if (!stream)
                return res;
        uint32_t fat_table_pos = fat16_get_first_fat_sector(private) * disk->sector_size;
        res = disk_stream_seek(stream, fat_table_pos * (cluster * FAT16_FAT_ENTRY_SIZE));
        if (res < 0)
                return res;

        uint16_t result = 0;
        res = disk_stream_read(stream, &result, sizeof(result));
        if (res < 0)
                return res;
        res = result;
        return res;
}

//Get the correct cluster to use based on the starting cluster and offset
static int fat16_get_cluster_for_offset(struct disk *disk, int starting_cluster, int offset){
        int res = 0;


        struct fat_private *private = disk->fs_private;
        int size_of_cluster = private->header.primary_header.sectors_per_cluser * disk->sector_size;
        int cluster_to_use = starting_cluster;
        int cluster_ahead = offset / size_of_cluster;
        for (int i = 0; i < cluster_ahead; i++){
                int entry = fat16_get_fat_entry(disk, cluster_to_use);
                if (entry == 0xFF8 || entry == 0xFFF)
                        return -EIO;
                if (entry == FAT16_BAD_SECTOR)
                        return -EIO;
                if (entry == 0xFF0 || entry == 0xFF6)
                        return -EIO; 
                if (entry == 0)
                        return -EIO;
                cluster_to_use = entry;
        }

        res = cluster_to_use;

        return res;

}

static int fat16_read_internal_from_stream(struct disk *disk, struct disk_stream *stream, int cluster, int offset, int total, void *out){
        int res = 0;
        struct fat_private *private = disk->fs_private;
        int size_of_cluster = private->header.primary_header.sectors_per_cluser * disk->sector_size;
        int cluster_to_use = fat16_get_cluster_for_offset(disk, cluster, offset);
        if (cluster_to_use < 0){
                return cluster_to_use;
        }

        int offset_from_cluster = offset % size_of_cluster;

        int starting_sector = fat16_cluster_to_sector(private, cluster_to_use);
        int starting_pos = (starting_sector * disk->sector_size) + offset_from_cluster;
        int total_to_read = total > size_of_cluster ? size_of_cluster : total;
        res = disk_stream_seek(stream, starting_pos);
        if (res != 0)
                return res;
        res = disk_stream_read(stream, out, total_to_read);
        if (res != 0)
                return res;
        total -= total_to_read;
        if (total > 0)
                res = fat16_read_internal_from_stream(disk, stream, cluster, offset + total_to_read, total, out + total_to_read);
        return res;
}


static int fat16_read_internal(struct disk *disk, int starting_cluster, int offset, int total, void *out){
        struct fat_private *fs_private;
        fs_private = disk->fs_private;
        struct disk_stream *stream = fs_private->cluster_read_stream;
        return fat16_read_internal_from_stream(disk, stream, starting_cluster, offset, total, out);
}

void fat16_free_directory(struct fat_directory *dir){
        if (!dir)
                return ;
        if (dir->item)
                kfree(dir->item);
        kfree(dir);
}


void fat16_item_free(struct fat_item *item){
        if (item->type == FAT_ITEM_TYPE_DIRECTORY)
                fat16_free_directory(item->directory);
        if (item->type == FAT_ITEM_TYPE_FILE)
                kfree(item->item);

        kfree(item);
}

struct fat_directory *fat16_load_fat_directory(struct disk *disk, struct fat_directory_item *item){
        int res = 0;
        struct fat_directory *directory = 0;
        struct fat_private *fat_private = disk->fs_private;
        if (!(item->attr & FAT_FILE_SUBDIRECTORY))
                return directory;

        directory = kzalloc(sizeof(struct fat_directory));
        if (!directory){
                res = -ENOMEM;
                goto out;
        }

        int cluster = fat16_get_first_cluster(item);
        int cluster_sector = fat16_cluster_to_sector(fat_private, cluster);
        int total_items = fat16_get_total_items_for_directory(disk, cluster_sector);
        directory->total = total_items;
        int directory_size = directory->total * sizeof(struct fat_directory_item);
        directory->item = kzalloc(directory_size);
        if (!directory->item){
                res = -ENOMEM;
                goto out;
        }

        res = fat16_read_internal(disk, cluster, 0, directory_size, directory->item);
        if (res != 0)
                goto out;

out:
        if (res != 0){
                fat16_free_directory(directory);
        }
        return directory; 
}

struct fat_item *fat16_new_fat_item_for_directory_item(struct disk *disk, struct fat_directory_item *item){
        struct fat_item *f_item = kzalloc(sizeof(struct fat_item));
        if (!f_item)
                return 0;

        if (item->attr & FAT_FILE_SUBDIRECTORY){
                f_item->directory = fat16_load_fat_directory(disk, item);
                f_item->type = FAT_ITEM_TYPE_DIRECTORY;
        }

        f_item->type = FAT_ITEM_TYPE_FILE;
        f_item->item = fat16_clone_directory_item(item, sizeof(struct fat_directory_item));
        return f_item;
}

struct fat_item *fat16_find_item_in_directory(struct disk *disk, struct fat_directory *dir, const char *name){
        struct fat_item *f_item = 0;
        char tmp_filename[FILE_MAX_PATH];
        for (int i = 0; i < dir->total; ++i)
        {
                fat16_get_full_relative_filename(&dir->item[i], tmp_filename, FILE_MAX_PATH);
                if (istrncmp(tmp_filename, name, FILE_MAX_PATH) == 0){
                        f_item = fat16_new_fat_item_for_directory_item(disk, &dir->item[i]);
                }
        }
        return f_item;
}


struct fat_item *fat16_get_directory_entry(struct disk *disk, struct path_part *path){
        struct fat_private *fat_private = disk->fs_private;
        struct fat_item *cur_item = 0;
        struct fat_item *root_item 
                = fat16_find_item_in_directory(disk, &fat_private->root_dir, path->path);

        if (!root_item)
                return 0;

        struct path_part *next_part = path->next;
        cur_item = root_item;
        while (next_part){
                if (cur_item->type != FAT_ITEM_TYPE_DIRECTORY){
                        cur_item = 0;
                        break;
                }

                struct fat_item *tmp = fat16_find_item_in_directory(disk, cur_item->directory, next_part->path);
                fat16_item_free(cur_item);
                cur_item = tmp;
                next_part = next_part->next;
        }
        return cur_item;
}

void *fat16_open(struct disk *disk, struct path_part *path, uint32_t mode){
        if (mode != FILE_READ)
                return ERROR(ERDONLY);
        struct fat_private *p = disk->fs_private;
        (void)p;

        struct fat_file_descriptor *desc = 0;
        desc = kzalloc(sizeof(struct fat_file_descriptor));
        if (!desc)
                return ERROR(-ENOMEM);

        desc->pos = 0;

        desc->item = fat16_get_directory_entry(disk, path);
        if (desc->item == 0)
                return ERROR(-EIO);
        return desc; 
}

int fat16_read(struct disk *disk, void *desc, uint32_t size, uint32_t nmemb, char *out_ptr){
        int res = 0;


        struct fat_file_descriptor *fd = desc;
        struct fat_directory_item *item = fd->item->item;
        int offset = fd->pos;
        for (uint32_t i = 0; i < nmemb; i++){
                res = fat16_read_internal(disk, fat16_get_first_cluster(item), offset, size, out_ptr);
                if (ISERR(res))
                        return ERROR_I(res);
                out_ptr += size;
                offset += size;
        }

        res = nmemb;
        return res;
}


int fat16_seek(void *private, uint32_t offset, uint32_t seek_mode){
        struct fat_file_descriptor *desc = private;
        struct fat_item *desc_item = desc->item;
        if (desc_item->type != FAT_ITEM_TYPE_FILE)
                return -EINVARG;

        struct fat_directory_item *ritem = desc_item->item;
        if (offset >- ritem->filesize)
                return -EIO;

        if (seek_mode == SEEK_SET)
                desc->pos = offset;
        else if (seek_mode == SEEK_END)
                return -EUNIMP;
        else if (seek_mode == SEEK_CUR)
                desc->pos += offset;
        else
                return -EINVARG;
        return 0;
}

int fat16_stat(struct disk *disk, void *private, struct file_stat *stat){
        struct fat_file_descriptor *desc = private;
        struct fat_item *desc_item = desc->item;
        if (desc_item->type != FAT_ITEM_TYPE_FILE)
                return -EINVARG;
        struct fat_directory_item *ritem = desc_item->item;
        stat->file_size = ritem->filesize;
        stat->flags = 0;
        if (ritem->attr & FAT_FILE_READ_ONLY){
                stat->flags |= FILE_STAT_READ_ONLY;
        }
        return 0;
}

static void fat16_free_file_descriptor(struct fat_file_descriptor *desc){
        fat16_item_free(desc->item);
        kfree(desc);
}

int fat16_close(void *private){
        fat16_free_file_descriptor((struct fat_file_descriptor *)private);
        return 0;
}
