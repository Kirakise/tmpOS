#pragma once
#include <stdint.h>

#define TOTAL_IDESCS 512

struct idt_desc
{
        uint16_t offset_1; //0-15
        uint16_t selector;
        uint8_t zero;
        uint8_t type_attr;
        uint16_t offset_2; //16-31
} __attribute__((packed)); 


struct idtr_desc
{
        uint16_t limit;
        uint32_t base;
} __attribute__((packed));

void idt_init();
