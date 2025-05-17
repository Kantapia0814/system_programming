#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <string.h>
#include <poll.h>
#include "network.h"

#ifndef BUFLEN
#define BUFLEN 256
#endif

#define NAMELEN 100
#define MOVELEN 50

int didTheyLeave(struct pollfd* pfds){
    int bytes, bytes2;
    char buf[BUFLEN], buf2[BUFLEN];
    int move1_ready = 0, move2_ready = 0;

    for (;;) {
        int ready = poll(pfds, 2, -1);
        if (ready == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (pfds[0].revents && !move1_ready) {
            bytes = read(pfds[0].fd, buf, BUFLEN);
            if (bytes <= 0) {
                printf("Player1 disconnected\n");
                break;
                }
                buf[bytes] = '\0';
            }
        if (pfds[1].revents && !move2_ready) {
                bytes2 = read(pfds[1].fd, buf2, BUFLEN);
                    if (bytes2 <= 0) {
                        printf("Player2 disconnected\n");
                        break;
                    }
                    buf2[bytes2] = '\0';
                }
            }
            if(strcmp(buf, "C") == 0 && strcmp(buf2, "C") == 0){
                return 1;
            }
            return 0;
        }

int main(int argc, char **argv)
{
    int sock, sock2;
    int bytes, bytes2;
    int ready1 = 0;
    int ready2 = 0;
    char buf[BUFLEN], buf2[BUFLEN];
    char player1[NAMELEN], player2[NAMELEN];

    char *service = argc == 2 ? argv[1] : "55555";

    int listen = open_listener(service, 2);
    printf("Server is running (port %s) \n", service);
    printf("Waiting for Player1...\n");
	sock = accept(listen, NULL, NULL);
    printf("Player1 is connected!\n");
    printf("Waiting for Player2...\n");
    sock2 = accept(listen, NULL, NULL);
    printf("Player2 is connected!\n");
	close(listen);

    if (sock < 0 || sock2 < 0) exit(EXIT_FAILURE);

    struct pollfd pfds[2];
    pfds[0].fd = sock;
    pfds[1].fd = sock2;
    pfds[0].events = POLLIN;
    pfds[1].events = POLLIN;

    for (;;) {
	int ready = poll(pfds, 2, -1);
	if (ready == -1) {
	    perror("poll");
	    exit(EXIT_FAILURE);
	}

	if (pfds[0].revents && ready1 == 0) {
        bytes = read(sock, buf, BUFLEN);
	        if (bytes <= 0) {
                printf("Connection closed remotely\n");
                break;
            }
            buf[bytes] = '\0';

            if (strncmp(buf, "P|", 2) == 0) {
                char *end = strstr(buf, "||");
                if (end != NULL) {
                    int name_length = end - (buf + 2);
                    strncpy(player1, buf + 2, name_length);
                    player1[name_length] = '\0';
                    printf("Player1's name: %s\n", player1);
                    write(sock, "W|1||", 5);
                    ready1 = 1;
                } else {
                    printf("Please enter your name again!\n");
                } 
            } else {
                printf("Please enter your name again!\n");
            }
                
	    struct iovec msg[] = {
            { "In: >>", 6 },
            { buf, bytes },
            { "<<\n", 3 }};
    
            writev(STDOUT_FILENO, msg, 3);
	}
	if (pfds[1].revents && ready2 == 0) {
        bytes2 = read(sock2, buf2, BUFLEN);
	        if (bytes2 < 1) {
		        printf("Connection closed remotely\n");
		        break;
	        }
            buf2[bytes2] = '\0';

            if (strncmp(buf2, "P|", 2) == 0) {
                char *end = strstr(buf2, "||");
                if (end != NULL) {
                    int name_length = end - (buf2 + 2);
                    strncpy(player2, buf2 + 2, name_length);
                    player2[name_length] = '\0';
                    printf("Player2's name: %s\n", player2);
                    write(sock2, "W|1||", 5);
                    ready2 = 1;
                } else {
                    printf("Please enter your name again!\n");
                }
            } else {
                printf("Please enter your name again!\n");
            }   
	    struct iovec msg[] = {
		{ "In: >>", 6 },
		{ buf2, bytes2 },
		{ "<<\n", 3 }};

	    writev(STDOUT_FILENO, msg, 3);
	}
    if (ready1 == 1 && ready2 == 1) {
        char message1[BUFLEN];
        snprintf(message1, BUFLEN, "B|%s||", player2);
        write(sock, message1, strlen(message1));
    
        char message2[BUFLEN];
        snprintf(message2, BUFLEN, "B|%s||", player1);
        write(sock2, message2, strlen(message2));
    
        printf("Both players are ready. Begin messages sent.\n");

        char move1[MOVELEN], move2[MOVELEN];
        int move1_ready = 0, move2_ready = 0;

        for (;;) {
            int ready = poll(pfds, 2, -1);
            if (ready == -1) {
                perror("poll");
                exit(EXIT_FAILURE);
            }
        
            if (pfds[0].revents && !move1_ready) {
                bytes = read(sock, buf, BUFLEN);
                    if (bytes <= 0) {
                        printf("Player1 disconnected\n");
                        break;
                    }
                    buf[bytes] = '\0';
        
                    if (strncmp(buf, "M|", 2) == 0) {
                        char *end = strstr(buf, "||");
                        if (end != NULL) {
                            int move_length = end - (buf + 2);
                            strncpy(move1, buf + 2, move_length);
                            move1[move_length] = '\0';
                            printf("Player1's move: %s\n", move1);
                            move1_ready = 1;
                        } else {
                            printf("Please enter a valid move again!\n");
                        } 
                    } else {
                        printf("Please enter a valid move again!\n");
                    }
                        
                struct iovec msg[] = {
                    { "In: >>", 6 },
                    { buf, bytes },
                    { "<<\n", 3 }};
            
                    writev(STDOUT_FILENO, msg, 3);
            }
            if (pfds[1].revents && !move2_ready) {
                bytes2 = read(sock2, buf2, BUFLEN);
                    if (bytes2 <= 0) {
                        printf("Player2 disconnected\n");
                        break;
                    }
                    buf2[bytes2] = '\0';
        
                    if (strncmp(buf2, "M|", 2) == 0) {
                        char *end = strstr(buf2, "||");
                        if (end != NULL) {
                            int move_length = end - (buf2 + 2);
                            strncpy(move2, buf2 + 2, move_length);
                            move2[move_length] = '\0';
                            printf("Player2's move: %s\n", move2);
                            move2_ready = 1;
                        } else {
                            printf("Please enter a valid move again!\n");
                        } 
                    } else {
                        printf("Please enter a valid move again!\n");
                    }
                        
                struct iovec msg[] = {
                    { "In: >>", 6 },
                    { buf2, bytes2 },
                    { "<<\n", 3 }};
            
                    writev(STDOUT_FILENO, msg, 3);

                if (move1_ready && move2_ready) {
                    if (move1_ready && move2_ready) {
                        char game_result1[BUFLEN];
                        char game_result2[BUFLEN];

                        if (strcmp(move1, move2) == 0 ) {
                            snprintf(game_result1, BUFLEN, "R|D|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|D|%s||\n", move1);
                        }
                        else if (strcmp(move1, "ROCK") == 0 && strcmp(move2, "SCISSORS") == 0) {
                            snprintf(game_result1, BUFLEN, "R|W|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|L|%s||\n", move1);
                        }
                        else if (strcmp(move1, "ROCK") == 0 && strcmp(move2, "PAPER") == 0) {
                            snprintf(game_result1, BUFLEN, "R|L|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|W|%s||\n", move1);
                        }
                        else if (strcmp(move1, "PAPER") == 0 && strcmp(move2, "ROCK") == 0) {
                            snprintf(game_result1, BUFLEN, "R|W|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|L|%s||\n", move1);
                        }
                        else if (strcmp(move1, "PAPER") == 0 && strcmp(move2, "SCISSORS") == 0) {
                            snprintf(game_result1, BUFLEN, "R|L|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|W|%s||\n", move1);
                        }
                        else if (strcmp(move1, "SCISSORS") == 0 && strcmp(move2, "PAPER") == 0) {
                            snprintf(game_result1, BUFLEN, "R|W|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|L|%s||\n", move1);
                        }
                        else if (strcmp(move1, "SCISSORS") ==0 && strcmp(move2, "ROCK") == 0) {
                            snprintf(game_result1, BUFLEN, "R|L|%s||\n", move2);
                            snprintf(game_result2, BUFLEN, "R|W|%s||\n", move1);
                        }

                        write(pfds[0].fd, game_result1, strlen(game_result1));
                        write(pfds[1].fd, game_result2, strlen(game_result2));
                    }
                }
            }
        }
    }


    }
    puts("Closing\n");
    close(sock);
    close(sock2);

    return EXIT_SUCCESS;
}