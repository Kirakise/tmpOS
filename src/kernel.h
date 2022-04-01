#pragma once

//Вообще-то можно #include <stddef.h>, но раз уж все так красиво сделал
//#define uint16_t        unsigned short int
//#define uint32_t        unsigned int
//#define uint64_t        unsigned long long
//#define uint8_t         unsigned char
//#define int8_t          char
//#define int16_t         short int
//#define int32_t         int
//#define int64_t         long long
#include <stdint.h>


//Размер терминала
#define VGA_WIDTH 80
#define VGA_HEIGHT 20


#define FILE_MAX_PATH 100



//Цвета
#define BLACK 0
#define BLUE 1
#define GREEN 2
#define CYAN 3
#define RED 4
#define MAGNETA 5
#define BROWN 6
#define GREY 7
#define DARK_GREY 8
#define BRIGHT_BLUE 9
#define BRIGHT_GREEN 10
#define BRIGHT_CYAN 11
#define BRIGHT_RED 12
#define BRIGHT_MAGNETA 13
#define YELLOW 14
#define WHITE 15


extern uint16_t *video_mem;
extern uint8_t term_col;
extern uint8_t term_row;

#define ERROR(value) (void*)(value)
#define ERROR_I(value) (int)(value)
#define ISERR(value) ((int)value < 0)

void kernel_main();
uint16_t get_char(uint8_t ch, uint8_t fore_color, uint8_t back_color);



