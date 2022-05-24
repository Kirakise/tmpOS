SRCS = ./src/stdlib.c ./src/os.c ./src/print.c
INCS = ./src
OBJS = $(SRCS:.c=.o)
ASMSRCS = start.asm ./src/os.asm
ASMOBJS = $(ASMSRCS:.asm=.asm.o)
CC = $(HOME)/opt/cross/bin/i686-elf-gcc
#FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -m32 -std=gnu99 -Wall -Wextra
FLAGS = -fno-builtin -fno-exceptions -fno-stack-protector -nostdlib -Wall -Wextra -m32 -std=gnu99 -g -nodefaultlibs -ffreestanding

bash:
	bash build.sh

all: $(ASMOBJS) $(OBJS)
	nasm -f elf ./start.asm -o ./start.o
	i686-elf-ld -m elf_i386 -relocatable ${ASMOBJS} $(OBJS) -o ./stdlib.elf

clean:
	rm -f $(ASMOBJS) $(OBJS)
	rm -f stdlib.elf

re: clean bash

%.asm.o: %.asm
	nasm -f elf -g $< -o $@

%.o: %.c
	$(CC) $(FLAGS) $(INCS) -c $< -o $@

