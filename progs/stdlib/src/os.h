#pragma once
#include <stddef.h>
void print(const char *str);
char getkey(void);
void *os_malloc(size_t size);
void os_free(void *ptr);
void os_putchar(char c);
