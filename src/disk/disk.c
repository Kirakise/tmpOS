#include "../io/io.h"
#include "../memory/status.h"
#include "../utils.h"
#include "disk.h"

struct disk disk;

int disk_read_sector(int lba, int total, void *buf){
        outb(0x1F6, (lba >> 24) | 0xE0);
        outb(0x1F2, total);
        outb(0x1F3, (uint8_t)(lba & 0xff));
        outb(0x1F4, (uint8_t)(lba >> 8));
        outb(0x1F5, (uint8_t)(lba >> 16));
        outb(0x1F7, 0x20);

        uint16_t *ptr = (uint16_t *)buf;
        for (int i = 0; i < total; i++){

                char c = insb(0x1F7);
                while (!(c & 0x08))
                        c = insb(0x1F7);

                for (int j = 0; j < 256; j++)
                        *ptr++ = insw(0x1F0); 
        }

        return 0;
}

int disk_write_sector(int lba, int total, void *buf){
        outb(0x1F6, (lba >> 24) | 0xE0);
        outb(0x1F2, total);
        outb(0x1F3, (uint8_t)(lba & 0xff));
        outb(0x1F4, (uint8_t)(lba >> 8));
        outb(0x1F5, (uint8_t)(lba >> 16));
        outb(0x1F7, 0x30);

        uint16_t *ptr = buf;
        for (int i = 0; i < total; i++){
          char c = insb(0x1F7);
          while (!(c & 0x08))
            c = insb(0x1F7);
          for (int j = 0; j < 256; j++)
            outw(0x1F0, *ptr++);
        }

        return 0;
}


void disk_search_and_init(){

        memset(&disk, 0, sizeof(disk));
        disk.type = DISK_TYPE_REAL;
        disk.sector_size = 512;
        disk.fs = fs_resolve(&disk);
}


struct disk *disk_get(int index){
        if (index != 0)
                return 0;
        return &disk;
}


int disk_read_block(struct disk *idisk, int lba, int total, void *buf){
        if (idisk != &disk)
                return -EIO;

        return disk_read_sector(lba, total, buf);
}
