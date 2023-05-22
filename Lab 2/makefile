all: myshell mypipe looper

myshell: myshell.o LineParser.o
	gcc -m32 -Wall -g myshell.o LineParser.o -o myshell

mypipe: mypipe.o
	gcc -m32 -Wall -g mypipe.o -o mypipe

looper: looper.o
	gcc -m32 -Wall -g looper.o -o looper

myshell.o: myshell.c LineParser.h
	gcc -m32 -Wall -g -c myshell.c

mypipe.o: mypipe.c
	gcc -m32 -Wall -g -c mypipe.c

looper.o: looper.c
	gcc -m32 -Wall -g -c looper.c

LineParser.o: LineParser.c LineParser.h
	gcc -m32 -Wall -g -c LineParser.c

clean:
	rm -f myshell.o LineParser.o mypipe.o looper.o myshell mypipe looper