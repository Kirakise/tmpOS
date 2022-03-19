#include "kernel.h"
#include "utils.h"
#include "idt/idt.h"
#include "memory/kheap.h"

uint16_t *video_mem = (uint16_t *)(0xB8000);
uint8_t term_col;
uint8_t term_row;
extern void _problem();


void clear_term()
{
        term_col = 0;
        term_row = 0;
        for (int i = 0; i < VGA_HEIGHT; i++)
                for (int j = 0; j < VGA_WIDTH; j++)
                        video_mem[i * VGA_WIDTH + j] = get_char(' ', RED, BLACK);
}

uint16_t get_char(uint8_t ch, uint8_t fore_color, uint8_t back_color)
{
  uint16_t ax = 0;
  uint8_t ah = 0, al = 0;
  ah = back_color;
  ah <<= 4;
  ah |= fore_color;
  ax = ah;
  ax <<= 8;
  al = ch;
  ax |= al;

  return ax;
}

void kernel_start()
{
        idt_init();
        clear_term();
        kheap_init(); //init kernel heap
        void *ptr = kmalloc(50);
        void *ptr2 = kmalloc(5000);
        kfree(ptr);
        kfree(ptr2);
        ptr = kmalloc(50);
        if ((uint32_t)ptr != (uint32_t)HEAP_ADDRESS)
                print("HUI");
        print("23");
}
