#pragma once

#include <stdint.h>
#include <stddef.h>
#include "elf.h"
#include "../../memory/status.h"
#include "../../kernel.h"

struct elf_file{
  char filename[FILE_MAX_PATH];
  int in_memory_size;
  void *elf_memory; //The physical addres of the elf file
  void *virtual_base_address; //Address of the binary
  void *virtual_end_address; //End of the binary
  void *physical_base_address; //Same but physical
  void *physical_end_address; //Same but physical
};

int elf_load(const char *filename, struct elf_file **file_out);
void elf_close(struct elf_file *file);
void *elf_virt_base(struct elf_file *file);
void *elf_virt_end(struct elf_file *file);
void *elf_phys_end(struct elf_file *file);
void *elf_phys_base(struct elf_file *file);
void *elf_memory(struct elf_file *file);
struct elf_header *elf_header(struct elf_file *file);
struct elf32_shdr *elf_sheader(struct elf_header *header);
struct elf32_phdr *elf_pheader(struct elf_header *header);
struct elf32_phdr *elf_program_header(struct elf_header *header, int index);
struct elf32_shdr *elf_section(struct elf_header *header, int index);
char *efl_str_table(struct elf_header *header);
void *elf_phdr_phys_addr(struct elf_file *file, struct elf32_phdr *phdr);

