CC = gcc
CFLAGS = -g -Wall -std=c99 -fsanitize=address,undefined

all: rpsd xmit echoserv1

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
echoserv1: echoserv1.o server.o
	$(CC) $(CFLAGS) -o $@ $^
xmit: xmit.o server.o
	$(CC) $(CFLAGS) -o $@ $^
rpsd: rpsd.o server.o
	$(CC) $(CFLAGS) -o $@ $^
clean:
	rm -f *.o rpsd xmit echoserv1