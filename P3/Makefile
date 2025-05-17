CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

all: mysh

mysh: mysh.o
	$(CC) $(CFLAGS) mysh.o -o mysh

mysh.o: mysh.c
	$(CC) $(CFLAGS) -c mysh.c -o mysh.o

clean:
	rm -f *.o mysh