# Compiler settings
CC = gcc
CFLAGS = -Wall -Wextra -g

# Executable and source files
EXEC = memgrind
SRC = memgrind.c mymalloc.c
OBJ = $(SRC:.c=.o)

# Default rule: Build the executable
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile C files into object files
%.o: %.c mymalloc.h
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f $(OBJ) $(EXEC)
