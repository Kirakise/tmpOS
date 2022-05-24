#pragma once
#include <stddef.h>
#include <stdarg.h>
void *malloc(size_t size);
void free(void *ptr);
int printf(const char *form, ...);
