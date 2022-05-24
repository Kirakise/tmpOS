#pragma once
#include <stddef.h>
#include <stdbool.h>
void print(const char *str);
char os_getkey(void);
void *os_malloc(size_t size);
void os_free(void *ptr);
void os_putchar(char c);



int getkey_block();
void terminal_readline(char *out, int max, bool out_while_type);
