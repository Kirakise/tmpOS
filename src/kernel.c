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
#include <stdarg.h>
#include "task/task.h"
#include "task/process.h"
#include "print/print.h"

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

struct tss tss_ker, tss_usr;
//struct gdt gdt_real[TOTAL_SEGMENTS];
struct gdt *gdt_real = (struct gdt *)0x800;
struct gdt_structured gdt_structured[TOTAL_SEGMENTS] = {
        {.base = 0x00, .limit = 0x00, .type = 0x00}, //null
        {.base = 0x00, .limit = 0xffffffff, .type = 0x9A}, //KerCode
        {.base = 0x00, .limit = 0xffffffff, .type = 0x92}, //KerData
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf8}, //UserCode
        {.base = 0x00, .limit = 0xffffffff, .type = 0xf2}, //UserData
        {.base = (uintptr_t)&tss_ker, .limit = sizeof(tss_ker), .type = 0x89}, //TSS_KER
        {.base = (uintptr_t)&tss_usr, .limit = sizeof(tss_usr), .type = 0xE9} //TSS_USR
};

void kernel_start()
{ 
        //Clear termianl window
        clear_term();
        //Load GDT
        memset(gdt_real, 0, sizeof(struct gdt));
        gdt_structured_to_gdt(gdt_real, gdt_structured, TOTAL_SEGMENTS);
        gdt_load(gdt_real, sizeof(struct gdt) * TOTAL_SEGMENTS);
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
        memset(&tss_ker, 0, sizeof(tss_ker));
        tss_ker.esp0 = 0x200000;
        tss_ker.ss0 = 0x10; //0x10 offset for kernel data
        tss_load(0x28); //0x28 is offset for gdt_real
        
        memset(&tss_usr, 0, sizeof(tss_usr));
        tss_usr.esp0 = 0x600000;
        tss_usr.ss0 = 0x20;
        tss_load(0x30);


        struct process *process = 0;
        if(process_load("0:/blank.bin", &process))
                panic("PROBLEM WITH PROCESS\n", 1);
        printk("%i", process->task->registers.ss);
        task_run_first_task();
 
        print("Everything is OK\n");
}
