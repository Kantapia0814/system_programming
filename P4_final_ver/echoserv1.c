#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>
#include "network.h"

#define QUEUE_SIZE 8

volatile int active = 1;

void handler(int signum)
{
    active = 0;
}

void install_handlers(void)
{
    struct sigaction act;
    act.sa_handler = handler;
    act.sa_flags = 0;
    sigemptyset(&act.sa_mask);

    sigaction(SIGINT, &act, NULL);
    sigaction(SIGTERM, &act, NULL);
}


#define BUFSIZE 256
#define HOSTSIZE 100
#define PORTSIZE 10
void read_data(int sock, struct sockaddr *rem, socklen_t rem_len)
{
    char buf[BUFSIZE + 1], host[HOSTSIZE], port[PORTSIZE];
    int bytes, error;

    error = getnameinfo(rem, rem_len, host, HOSTSIZE, port, PORTSIZE, NI_NUMERICSERV);
    if (error) {
        fprintf(stderr, "getnameinfo: %s\n", gai_strerror(error));
        strcpy(host, "??");
        strcpy(port, "??");
    }

    printf("Connection from %s:%s\n", host, port);

    while (active && (bytes = read(sock, buf, BUFSIZE)) > 0) {
        buf[bytes] = '\0';
        printf("[%s:%s] read %d bytes |%s|\n", host, port, bytes, buf);
    }

	if (bytes == 0) {
		printf("[%s:%s] got EOF\n", host, port);
	} else if (bytes == -1) {
		printf("[%s:%s] terminating: %s\n", host, port, strerror(errno));
	} else {
		printf("[%s:%s] terminating\n", host, port);
	}

    close(sock);
}

int main(int argc, char **argv)
{
    struct sockaddr_storage remote_host;
    socklen_t remote_host_len;

    char *service = argc == 2 ? argv[1] : "15000";

	install_handlers();
	
    int listener = open_listener(service, QUEUE_SIZE);
    if (listener < 0) exit(EXIT_FAILURE);
    
    puts("Listening for incoming connections");

    while (active) {
        remote_host_len = sizeof(remote_host);
        int sock = accept(listener, 
            (struct sockaddr *)&remote_host,
            &remote_host_len);
        
        if (sock < 0) {
            perror("accept");
            continue;
        }

        read_data(sock, (struct sockaddr *)&remote_host, remote_host_len);
    }

    puts("Shutting down");
    close(listener);

    return EXIT_SUCCESS;
}
