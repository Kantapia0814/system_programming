CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

all: outlier

outlier: outlier.o
	$(CC) $(CFLAGS) outlier.o -o outlier

outlier.o: outlier.c
	$(CC) $(CFLAGS) -c outlier.c -o outlier.o

clean:
	rm -f *.o outlier