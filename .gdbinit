target remote | qemu-system-i386 -hda ./bin/os.bin -gdb stdio -S
add-symbol-file src/kernelfull.o 0x100000
