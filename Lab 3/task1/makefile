all: clean start.o util.o
	ld -m elf_i386 start.o util.o -o task1
start.o:
	nasm -f elf32 start.s -o start.o 
util.o:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
clean:
	rm -f start.o util.o task1
