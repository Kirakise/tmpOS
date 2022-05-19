ASM = ./build/kernel.asm.o ./build/idt/idt.asm.o ./build/io/io.asm.o ./build/memory/paging.asm.o
SRCS = ./src/kernel.c ./src/utils.c ./src/idt/idt.c ./src/memory/heap.c ./src/memory/kheap.c ./src/memory/paging.c ./src/disk/disk.c ./src/fs/parser.c ./src/disk/streamer.c ./src/fs/file.c ./src/fs/fat/fat16.c ./src/gdt/gdt.c  ./src/print/print.c ./src/task/task.c ./src/task/process.c ./src/isr80h/isr80h.c ./src/isr80h/misc.c ./src/isr80h/io.c ./src/keyboard/keyboard.c ./src/keyboard/PS2.c ./src/loader/formats/elf.c ./src/loader/formats/elfloader.c ./src/isr80h/heap.c
INCS = -I./src
OBJS = $(SRCS:.c=.o)
ASMSRCS = ./src/kernel.asm ./src/idt/idt.asm ./src/io/io.asm ./src/memory/paging.asm ./src/gdt/gdt.asm ./src/task/tss.asm ./src/task/task.asm
ASMOBJS = $(ASMSRCS:.asm=.asm.o)
CC = i686-elf-gcc
#FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -m32 -std=gnu99 -Wall -Wextra
FLAGS = -fno-builtin -fno-exceptions -fno-stack-protector -nostdlib -Wall -Wextra -m32 -std=gnu99 -g -nodefaultlibs -ffreestanding

bash:
	bash ./build.sh

all: ./bin/boot.bin ./bin/kernel.bin user_progs
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=1048576 count=16 >> ./bin/os.bin
	sudo mount -t vfat ./bin/os.bin /mnt
	sudo cp hello.txt /mnt
	sudo cp ./progs/blank/blank.elf /mnt
	sudo umount /mnt

./bin/kernel.bin: $(ASMOBJS) $(OBJS)
	i686-elf-ld -g -relocatable $(ASMOBJS) $(OBJS) -o ./src/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./linker.ld -o ./bin/kernel.bin -ffreestanding -nostdlib ./src/kernelfull.o

./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

%.asm.o: %.asm
	nasm -f elf -g $< -o $@

%.o: %.c
	$(CC) $(FLAGS) $(INCS) -c $< -o $@

clean: user_progs_clean
	rm -f ./bin/boot.bin ./bin/os.bin $(ASMOBJS) ./bin/kernel.bin $(OBJS) ./src/kernelfull.o

run:
	qemu-system-i386 -hda ./bin/os.bin


user_progs:
	cd ./progs/blank && $(MAKE) all
	cd ./progs/stdlib && $(MAKE) all

user_progs_clean:
	cd ./progs/blank && $(MAKE) clean
	cd ./progs/blank && $(MAKE) clean

re: clean user_progs_clean bash
