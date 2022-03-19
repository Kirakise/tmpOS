#pragma once
#include <stddef.h>

void print(const char *str);
void terminal_writechar(const char c, char fore_color, char back_color);
size_t strlen(const char *str);
void *memset(void *ptr, char c, int size);

