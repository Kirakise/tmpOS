#include "kernel.h"
#include "disk/streamer.h"
#include "disk/disk.h"
#include "utils.h"
#include "idt/idt.h"
#include "memory/kheap.h"
#include "memory/paging.h"
#include "fs/parser.h"

uint16_t *video_mem = (uint16_t *)(0xB8000);
uint8_t term_col;
uint8_t term_row;
struct paging_4gb_chunk *kernel_chunk;
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
        //Clear termianl window
        clear_term();
        //init kernel heap
        kheap_init();
         //Get paging
        kernel_chunk = paging_new_chunk(PAGING_IS_WRITABLE | PAGING_IS_PRESENT | PAGING_ACESS_FROM_ALL);
        //Switch kernel_chunk
        paging_switch(kernel_chunk->directory_entry);
        //enable_paging
        enable_paging();
        //inti filesystems
        fs_init();
        //With filesystems on search the disks
        disk_search_and_init();
        //Set interrupt table
        idt_init(); 
        //literally
        enable_interrupts();
        int fd = fopen("0:/hello.txt", "r");
        if (fd){
                print("hello.txt is open now\n");
        }
        print("Everything is OK\n");
}
