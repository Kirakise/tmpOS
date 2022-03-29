#pragma once
#include <stdint.h>

struct path_root{
        uint8_t drive_num;
        struct path_part *first;
};

struct path_part{
        const char *path;
        struct path_part *next;
};

struct path_root *parser_parse(const char *path, const char *current_directory_path);
void path_free(struct path_root * root);
