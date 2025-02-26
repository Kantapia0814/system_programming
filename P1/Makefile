CC = gcc
CFLAGS = -std=c99 -g -Wall -fsanitize=address,undefined

SRC = memgrind.c mymalloc.c
OBJ = $(SRC:.c=.o)

all: memgrind

memgrind: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c mymalloc.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
