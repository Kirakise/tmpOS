all:
	nasm -f elf ./blank.asm -o ./blank.o
	i686-elf-gcc -g -T ./linker.ld -o ./blank.bin -ffreestanding -nostdlib -fpic -g ./blank.o

clean:
	rm -f ./blank.o
