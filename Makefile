ASM = ./build/kernel.asm.o ./build/idt/idt.asm.o ./build/io/io.asm.o ./build/memory/paging.asm.o
SRCS = ./src/kernel.c ./src/utils.c ./src/idt/idt.c ./src/memory/heap.c ./src/memory/kheap.c ./src/memory/paging.c ./src/disk/disk.c ./src/fs/parser.c ./src/disk/streamer.c ./src/fs/file.c ./src/fs/fat/fat16.c
INCS = -I./src
OBJS = $(SRCS:.c=.o)
ASMSRCS = ./src/kernel.asm ./src/idt/idt.asm ./src/io/io.asm ./src/memory/paging.asm
ASMOBJS = $(ASMSRCS:.asm=.asm.o)
CC = i686-elf-gcc
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

bash:
	bash ./build.sh

all: ./bin/boot.bin ./bin/kernel.bin
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt/d
	sudo cp hello.txt /mnt/d/
	sudo umount /mnt/d

./bin/kernel.bin: $(ASMOBJS) $(OBJS)
	i686-elf-ld -g -relocatable $(ASMOBJS) $(OBJS) -o ./src/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./src/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

%.asm.o: %.asm
	nasm -f elf -g $< -o $@

%.o: %.c
	$(CC) $(FLAGS) $(INCS) -c $< -o $@

clean:
	rm -f ./bin/boot.bin ./bin/os.bin $(ASMOBJS) ./bin/kernel.bin $(OBJS) ./src/kernelfull.o

run:
	qemu-system-i386 -hda ./bin/os.bin

re: clean bash
