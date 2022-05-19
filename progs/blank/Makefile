all:
	$(HOME)/opt/cross/bin/i686-elf-gcc -g -std=gnu99 -c ./blank.c -o ./blank.o
	$(HOME)/opt/cross/bin/i686-elf-gcc -g -T ./linker.ld -o ./blank.elf -ffreestanding -nostdlib -fpic -g ./blank.o ../stdlib/stdlib.elf

clean:
	rm -f ./blank.o
	rm -f ./blank.elf
