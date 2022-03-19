ASM = ./build/kernel.asm.o ./build/idt/idt.asm.o ./build/io/io.asm.o
SRCS = ./src/kernel.c ./src/utils.c ./src/idt/idt.c ./src/memory/heap.c ./src/memory/kheap.c
INCS = -I./src
OBJS = $(SRCS:.c=.o)
CC = i686-elf-gcc
FLAGS = -g -ffreestanding -falign-jumps -falign-functions -falign-loops -fstrength-reduce -fomit-frame-pointer -finline-functions -Wno-unused-function -fno-builtin -Werror -Wno-unused-label -Wno-cpp -Wno-unused-parameter -nostdlib -nostartfiles -nodefaultlibs -Wall -O0 -Iinc

bash:
	bash ./build.sh

all: ./bin/boot.bin ./bin/kernel.bin
	rm -f ./bin/os.bin
	dd if=./bin/boot.bin >> ./bin/os.bin
	dd if=./bin/kernel.bin >> ./bin/os.bin
	dd if=/dev/zero bs=512 count=100 >> ./bin/os.bin


./bin/kernel.bin: $(ASM) $(OBJS)
	i686-elf-ld -g -relocatable $(ASM) $(OBJS) -o ./build/kernelfull.o
	i686-elf-gcc $(FLAGS) -T ./linker.ld -o ./bin/kernel.bin -ffreestanding -O0 -nostdlib ./build/kernelfull.o

./build/idt/idt.asm.o:
	nasm -f elf -g ./src/idt/idt.asm -o ./build/idt/idt.asm.o


./bin/boot.bin: ./src/boot/boot.asm
	nasm -f bin ./src/boot/boot.asm -o ./bin/boot.bin

./build/kernel.asm.o: ./src/kernel.asm
	nasm -f elf -g ./src/kernel.asm -o ./build/kernel.asm.o

./build/io/io.asm.o:
	nasm -f elf -g ./src/io/io.asm -o ./build/io/io.asm.o

%.o: %.c
	$(CC) $(FLAGS) $(INCS) -c $< -o $@

clean:
	rm -f ./bin/boot.bin ./bin/os.bin $(ASM) ./bin/kernel.bin $(OBJS) ./build/kernelfull.o


run:
	qemu-system-x86_64 -hda ./bin/os.bin

re: clean bash
