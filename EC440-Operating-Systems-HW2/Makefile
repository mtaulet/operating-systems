CC=gcc -Wall -g -m32
all: threadlib main ptr_mangle
	$(CC) -o main main.o threads.o ptr_mangle.o

main: main.c
	$(CC) -c -o main.o main.c

threadlib: threads.c
	$(CC) -c -o threads.o threads.c

ptr_mangle: ptr_mangle.c
	$(CC) -c -o ptr_mangle.o ptr_mangle.c

clean:
	rm threads.o ptr_mangle.o main.o main
