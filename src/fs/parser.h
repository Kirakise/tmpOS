#pragma once
#include <stdint.h>

struct path_root{
        uint8_t drive_num;
        struct path_part *frist;
};

struct path_part{
        const char *path;
        struct path_part *next;
};
