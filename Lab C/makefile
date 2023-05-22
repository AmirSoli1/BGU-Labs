all: myshell looper mypipeline

myshell: myshell.o LineParser.o
	gcc -Wall -g myshell.o LineParser.o -o myshell

looper: looper.o
	gcc -Wall -g looper.o -o looper

mypipeline: mypipeline.o
	gcc -Wall -g mypipeline.o -o mypipeline	

myshell.o: myshell.c LineParser.h
	gcc -Wall -g -c myshell.c

mypipe.o: mypipe.c
	gcc -Wall -g -c mypipe.c

looper.o: looper.c
	gcc -Wall -g -c looper.c

mypipeline.o: mypipeline.c
	gcc -Wall -g -c mypipeline.c

LineParser.o: LineParser.c LineParser.h
	gcc -Wall -g -c LineParser.c

clean:
	rm -f myshell.o LineParser.o looper.o mypipeline.o myshell looper mypipeline
