#include "parser.h"
#include "../utils.h"
#include "../kernel.h"
#include "../memory/status.h"
#include "../memory/kheap.h"


static int path_valid(const char *filename){
        int len = strnlen(filename, FILE_MAX_PATH);
        return len >= 3 && isdigit(filename[0]) && !memcmp(filename + 1, ":/", 2);
}

static int parser_get_drive_by_path(const char **path){
        if (!path_valid(*path))
                return -EBADPATH;
        return **path - 48;
}


static struct path_root *parser_create_root(int drive_num){
        struct path_root *path = kzalloc(sizeof(struct path_root));
        path->drive_num = drive_num;
        path->first = 0;
        return path;
}

static const char *parser_get_path_part(const char **path){
        char *res_path = kzalloc(FILE_MAX_PATH);
        int i = 0;
        while (*(*path + i) != '/' && *(*path + i) != 0){
                res_path[i] = *(*path + i);
                i++;
        }

        if (*(*path + i) == '/'){
                ++i;
        }

        *path += i;

        if (i == 0){
                kfree(res_path);
                return (0);
        }

        return res_path;
}


struct path_part *parse_path_part(struct path_part *last, const char **path){
        const char * path_part_str = parser_get_path_part(path);

        if (!path_part_str)
                return 0;


        struct path_part *part = kzalloc(sizeof(struct path_part));
        part->path = path_part_str;
        part->next = 0;

        if (last)
                last->next = part;

        return part;
}


void path_free(struct path_root * root){
        struct path_part *part = root->first;
        struct path_part *next = 0;
        while (part){
                next = part;
                part = part->next;
                kfree((char *)next->path);
                kfree(next);
        }
        kfree(root);
}


struct path_root *parser_parse(const char *path, const char *current_directory_path){
        int res = 0;

        const char *tmp_path = path;
        struct path_root *root= 0;
        if (strlen(path) > FILE_MAX_PATH)
                return 0;

        if((res = parser_get_drive_by_path(&tmp_path)) < 0)
                return 0;
        tmp_path += 3;

        if (!(root = parser_create_root(res)))
                return 0;

        struct path_part *first_part = parse_path_part(NULL, &tmp_path);
        if (!first_part){
                kfree(root);
                return 0;
        }

        root->first = first_part;
        struct path_part *part = parse_path_part(first_part, &tmp_path);
        while(part)
                part = parse_path_part(part, &tmp_path);
        return root;
}
