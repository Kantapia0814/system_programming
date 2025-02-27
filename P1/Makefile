CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

all: memgrind memtest

memgrind: memgrind.o mymalloc.o
	$(CC) $(CFLAGS) memgrind.o mymalloc.o -o memgrind

memtest: memtest.o mymalloc.o
	$(CC) $(CFLAGS) memtest.o mymalloc.o -o memtest

mymalloc.o: mymalloc.c mymalloc.h
	$(CC) $(CFLAGS) -c mymalloc.c -o mymalloc.o

memgrind.o: memgrind.c mymalloc.h
	$(CC) $(CFLAGS) -c memgrind.c -o memgrind.o

memtest.o: memtest.c mymalloc.h
	$(CC) $(CFLAGS) -c memtest.c -o memtest.o

clean:
	rm -f *.o memgrind memtest