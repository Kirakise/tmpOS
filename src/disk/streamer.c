#include "streamer.h"
#include "../memory/status.h"
#include "../memory/kheap.h"
#include <stdbool.h>

struct disk_stream * disk_streamer_new(int disk_id){
        struct disk *disk = disk_get(disk_id);
        if (!disk)
                return 0;
        struct disk_stream *streamer = kzalloc(sizeof(struct disk_stream));
        streamer->pos = 0;
        streamer->disk = disk;
        return streamer;
}


int disk_stream_seek(struct disk_stream *stream, int pos){
        stream->pos = pos;
        return 0;
}

int disk_stream_read(struct disk_stream *stream, void *out, int total){
        int sector = stream->pos / SECTOR_SIZE;
        int offset = stream->pos % SECTOR_SIZE;
        char buf[SECTOR_SIZE];
        int total_to_read = total > SECTOR_SIZE ? SECTOR_SIZE : total;
        bool overflow = (offset + total_to_read) >= SECTOR_SIZE;

        if (overflow)
          total_to_read -= (offset + total_to_read) - SECTOR_SIZE;

        int res = disk_read_block(stream->disk, sector, 1, buf);
        if (res < 0)
                return -EIO;

        for (int i = 0; i < total_to_read; i++){
                *(char *)out++ = buf[offset + i];
        }

        stream->pos += total_to_read;
        if (total > SECTOR_SIZE)
                res = disk_stream_read(stream, out, total - SECTOR_SIZE);
        return res;
}

int disk_stream_write(struct disk_stream *stream, void *out, int total){
        int sector = stream->pos / SECTOR_SIZE;
        int offset = stream->pos % SECTOR_SIZE;
        char buf[SECTOR_SIZE];
        int total_to_read = total > SECTOR_SIZE ? SECTOR_SIZE : total;
        bool overflow = (offset + total_to_read) >= SECTOR_SIZE;

        if (overflow)
          total_to_read -= (offset + total_to_read) - SECTOR_SIZE;

        int res = disk_read_block(stream->disk, sector, 1, buf);
        if (res < 0)
                return -EIO;

        for (int i = 0; i < total_to_read; i++){
                *(char *)out++ = buf[offset + i];
        }

        stream->pos += total_to_read;
        if (total > SECTOR_SIZE)
                res = disk_stream_read(stream, out, total - SECTOR_SIZE);
        return res;
}


void disk_stream_close(struct disk_stream *stream){
        kfree(stream);
}
