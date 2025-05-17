#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <signal.h>
#include "network.h"


#define BUFLEN 256

int main(int argc, char **argv)
{
    int sock, bytes, sent;
    char buf[BUFLEN];

    //signal(SIGPIPE, SIG_IGN);

    if (argc != 3) {
	printf("Specify host and service\n");
	exit(EXIT_FAILURE);
    }

    sock = connect_inet(argv[1], argv[2]);
    if (sock < 0) exit(EXIT_FAILURE);

    while ((bytes = read(STDIN_FILENO, buf, BUFLEN)) > 0) {
	printf("%d bytes read\n", bytes);

	sent = write(sock, buf, bytes);
	printf("%d bytes sent\n", sent);
	if (sent == -1) {
	    perror("write");
	    break;
	} else if (sent < bytes) {
	    printf("Only %d bytes sent\n", sent);
	}
    }

    close(sock);

    return EXIT_SUCCESS;
}
