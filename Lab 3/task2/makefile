all: clean start.o main.o util.o
	ld -m elf_i386 start.o main.o util.o -o task2 
start.o:
	nasm -f elf32 start.s -o start.o 
main.o:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector main.c -o main.o 
util.o:
	gcc -m32 -Wall -ansi -c -nostdlib -fno-stack-protector util.c -o util.o
clean:
	rm -f start.o main.o util.o task2
