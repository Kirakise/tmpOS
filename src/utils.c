#include "utils.h"
#include "kernel.h"
#include <stddef.h>
#include <stdbool.h>
#include "memory/kheap.h"
#include "print/print.h"

size_t strlen(const char *str){
        size_t count = 0;

        while (*str++ != 0)
                count++;
        return count;
}


void terminal_putchar(uint8_t x, uint8_t y, char c, char fore_color, char back_color)
{
        video_mem[y * VGA_WIDTH + x] = get_char(c, fore_color, back_color);
}


static void term_new_line()
{
        term_col = 0;
        if (term_row == 19){
                for (int i = 0; i < VGA_HEIGHT - 1; i++)
                        for (int j = 0; j < VGA_WIDTH; j++)
                                video_mem[i * VGA_WIDTH + j] = video_mem[(i + 1) * VGA_WIDTH + j];
        }
        else
                ++term_row;
}


void terminal_writechar(const char c, char fore_color, char back_color)
{
        if (c == '\n'){
                term_new_line();
                return ;
        }
        video_mem[term_row * VGA_WIDTH + term_col++] = get_char(c, fore_color, back_color);
        if (term_col == VGA_WIDTH)
                term_new_line();
}


void print(const char *str){
        while (*str)
                terminal_writechar(*str++, RED, BLACK);
}


void *memset(void *ptr, char c, int size){
        char *c_ptr = ptr;
        for (int i = 0; i < size; i++)
                c_ptr[i] = c;
        return ptr;
}


bool isdigit(char c){
        return c >= 48 && c <= 57;
}


int strnlen(const char *str, uint32_t max){
        uint32_t count = 0;

        while (*str++ != 0 || count > max)
                count++;
        return count > max ? -1 : count;
}

int memcmp(const void *ptr1, const void *ptr2, uint32_t n){
        for (int i = 0; i < n; i++)
                if (((char *)ptr1)[i] != ((char *)ptr2)[i])
                        return  ((char *)ptr1)[i] - ((char *)ptr2)[i];
        return  ((char *)ptr1)[n] - ((char *)ptr2)[n];
}


char *strcpy(char *dest, char *src){
        char *tmp = dest;
        while (*src)
                *dest++ = *src++;
        *dest = 0;
        return tmp;
}


int strcmp(char *s1, char *s2){
        while (*s1 == *s2 && *s1){
                ++s1;
                ++s2;
        }
        return *s1 - *s2;
}


int strncmp(char *s1, char *s2, uint32_t n){
        for (uint32_t i = 0; i < n; i++){
                if (s1[i] != s2[i] || !s1[i])
                        return s1[i] - s2[i];
        }
        return 0;
}

uint32_t strnlen_terminator(const char *str, uint32_t len, char c){
        for (uint32_t i = 0; i < len; i++){
                if (str[i] == '\0' || str[i] == c)
                        return i;
        }
        return 0;
}

uint8_t tolower(char c){
        if (c >= 'A' && c <= 'Z')
                return c + 32;
        else
                return c;
}

int istrncmp(const char *s1, const char *s2, uint32_t n){
        while(n-- > 0){
                if(tolower(*s1++) != tolower(*s2++))
                        return (tolower(*s1) - tolower(*s2));
        }
        return 0;
}


void *memcpy(void *dest, void *src, uint32_t n){
        for (uint32_t i = 0; i < n; ++i){
                ((char *)dest)[i] = ((char *)src)[i];
        }
        return dest;
}

int kwrite(int fd, char *str, int size){
        int i = 0;
        for (i = 0; i < size; i++)
                terminal_writechar(str[i], RED, BLACK);
        return i;
}

char *strdup(const char *s){
        uint32_t len = strlen(s);
        char *ret = kmalloc(len);
        for (uint32_t i = 0; i < len; i++){
                ret[i] = s[i];
        }
        ret[len] = 0;
        return ret;
}

char *strncpy(char *dest, const char *src, uint32_t n){
        if (!dest || !src || dest == src)
                return dest;
        for (int i = 0; i < n && src[i]; i++){
                dest[i] = src[i];
        }
        return dest;
}

void hexdump(void *addr, uint32_t count){
        uint8_t *c = addr;
        uint32_t offset = 0;
        char tmp[5];
        tmp[4] = 0;
        if (count > 100)
                return ;
        for (uint32_t i = 0; i < (count - count % 4); i += 4){

                printk("%p ", offset);
                printk("%X %X %X %X ", *c, *(c + 1), *(c + 2), *(c + 3));
                memcpy(tmp, c, 4);
                printk ("%s\n", tmp);
                offset += 4;
                c += 4;
        }
        if (count % 4){
                printk("%p ", offset);
                for (uint32_t j = 0; j < count % 4; j++)
                        printk("%X ", *(c + j));
                memcpy(tmp, c, offset % 4);
                tmp[offset % 4] = 0;
                printk("%s\n", tmp);
        }
}


void memwrite(void *addr, const char *str){
        char *s = addr;
        for (int i = 0; i < strlen(str); i++)
                s[i] = str[i]; 
}
