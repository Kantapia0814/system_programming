CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address,undefined

all: rpsd3 rpsd2 xmit echoserv1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
echoserv1: echoserv1.o server.o
	$(CC) $(CFLAGS) -o $@ $^
xmit: xmit.o server.o
	$(CC) $(CFLAGS) -o $@ $^
rpsd2: rpsd2.o server.o
	$(CC) $(CFLAGS) -o $@ $^
rpsd3: rpsd3.o server.o
	$(CC) $(CFLAGS) -o $@ $^
