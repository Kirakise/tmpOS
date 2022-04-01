#pragma once
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

void print(const char *str);
void terminal_writechar(const char c, char fore_color, char back_color);
size_t strlen(const char *str);
void *memset(void *ptr, char c, int size);
bool isdigit(char c);
int strnlen(const char *str, uint32_t max);
int memcmp(const void *ptr1, const void *ptr2, uint32_t n);
char *strcpy(char *dest, char *src);
int strncmp(char *s1, char *s2, uint32_t n);
int strcmp(char *s1, char *s2);
uint8_t tolower(char c);
int istrncmp(const char *s1, const char *s2, uint32_t n);
void *memcpy(void *dest, void *src, uint32_t n);

