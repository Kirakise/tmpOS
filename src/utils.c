#include "utils.h"
#include "kernel.h"
#include <stddef.h>

size_t strlen(const char *str){
        size_t count = 0;

        while (*str != 0)
                count++;
        return count;
}


void inline terminal_putchar(uint8_t x, uint8_t y, char c, char fore_color, char back_color)
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
