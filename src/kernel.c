#include "kernel.h"
#include "disk/streamer.h"
#include "disk/disk.h"
#include "utils.h"
#include "idt/idt.h"
#include "memory/kheap.h"
#include "memory/paging.h"
#include "fs/parser.h"
#include "gdt/gdt.h"
#include "task/tss.h"

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


void panic(const char *msg, uint8_t mode){
        print(msg);
        while (mode) {}
}

struct tss tss;
struct gdt gdt_real[TOTAL_SEGMENTS];
struct gdt_structured gdt_structured[TOTAL_SEGMENTS] = {
        {.base = 0x00, .limit = 0x00, .type = 0x00}, //null
        {.base = 0x00, .limit = 0xffffffff, .type = 0x9A}, //KerCode
        {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, //KerData
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf8}, //UserCode
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, //UserData
        {.base = (uint32_t)&tss, .limit = sizeof(tss), .type = 0xE9} //TSS
};

void kernel_start()
{ 
        //Clear termianl window
        clear_term();
        //Load GDT
        memset(gdt_real, 0, sizeof(gdt_real));
        gdt_structured_to_gdt(gdt_real, gdt_structured, TOTAL_SEGMENTS);
        gdt_load(gdt_real, sizeof(gdt_real));
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
        //Set TSS
        memset(&tss, 0, sizeof(tss));
        tss.esp0 = 0x600000;
        tss.ss0 = 0x10; //0x10 offset for kernel data
        tss_load(0x28); //0x28 is offset for gdt_real
        //literally
        enable_interrupts();
        int fd = fopen("0:/hello.txt", "r");
        if (fd){
                print("hello2.txt is open now\n");
                char buf[20];
                fseek(fd, 2, SEEK_SET);
                fread(buf, 20, 1, fd);
                struct file_stat s;
                fstat(fd, &s);
                fclose(fd);
                print("123");
                print(buf);
        }
        print("Everything is OK\n");
}
