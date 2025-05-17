#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include "network.h"

#ifndef BUFLEN
#define BUFLEN 256
#endif

#define NAMELEN 256
#define MOVELEN 50

char *names[100] = {NULL};

int is_duplicate(const char *name) {
    if (!name || !*name) return 0;
    for (int i = 0; names[i] != NULL; ++i)
        if (strcmp(names[i], name) == 0)
            return 1;
    return 0;
}

void add_name(const char *name) {
    for (int i = 0; i < 100; ++i) {
        if (names[i] == NULL) {
            names[i] = strdup(name);
            break;
        }
    }
}

void remove_name(const char *name) {
    for (int i = 0; names[i] != NULL; ++i) {
        if (strcmp(names[i], name) == 0) {
            free(names[i]);
            for (int j = i; names[j] != NULL; ++j)
                names[j] = names[j + 1];
            break;
        }
    }
}

void readNames(struct pollfd * pfds, char *player1, char *player2) {
    int ready1 = 0;
    int ready2 = 0;
    int bytes, bytes2;
    char buf[BUFLEN], buf2[BUFLEN];
    for (;;) {
	    int ready = poll(pfds, 2, -1);
	    if (ready == -1) {
	        perror("poll");
	        exit(EXIT_FAILURE);
	    }

	    if (pfds[0].revents && ready1 == 0) {
            bytes = read(pfds[0].fd, buf, BUFLEN);
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
                    write(pfds[0].fd, "W|1||", 5);
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
            bytes2 = read(pfds[1].fd, buf2, BUFLEN);
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
                    write(pfds[1].fd, "W|1||", 5);
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

        if (ready1 && ready2) {
            break;
        }
    }
}

void processMoves(struct pollfd * pfds, char *player1, char *player2) {
    char message1[BUFLEN];
    snprintf(message1, BUFLEN, "B|%s||", player2);
    write(pfds[0].fd, message1, strlen(message1));
        
    char message2[BUFLEN];
    snprintf(message2, BUFLEN, "B|%s||", player1);
    write(pfds[1].fd, message2, strlen(message2));
        
    printf("Both players are ready. Begin messages sent.\n");

    int bytes, bytes2;
    char buf[BUFLEN], buf2[BUFLEN];
    char move1[MOVELEN], move2[MOVELEN];
    int move1_ready = 0, move2_ready = 0;

    for (;;) {
        int ready = poll(pfds, 2, -1);
        if (ready == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        // Player1 move
        if (pfds[0].revents && !move1_ready) {
            bytes = read(pfds[0].fd, buf, BUFLEN-1);
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
            bytes2 = read(pfds[1].fd, buf2, BUFLEN);
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
        }
        if (move1_ready && move2_ready) {
            char game_result1[BUFLEN];
            char game_result2[BUFLEN];

            if (strcmp(move1, move2) == 0 ) {
                snprintf(game_result1, BUFLEN, "R|D|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|D|%s||", move1);
            }
            else if (strcmp(move1, "ROCK") == 0 && strcmp(move2, "SCISSORS") == 0) {
                snprintf(game_result1, BUFLEN, "R|W|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|L|%s||", move1);
            }
            else if (strcmp(move1, "ROCK") == 0 && strcmp(move2, "PAPER") == 0) {
                snprintf(game_result1, BUFLEN, "R|L|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|W|%s||", move1);
            }
            else if (strcmp(move1, "PAPER") == 0 && strcmp(move2, "ROCK") == 0) {
                snprintf(game_result1, BUFLEN, "R|W|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|L|%s||", move1);
            }
            else if (strcmp(move1, "PAPER") == 0 && strcmp(move2, "SCISSORS") == 0) {
                snprintf(game_result1, BUFLEN, "R|L|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|W|%s||", move1);
            }
            else if (strcmp(move1, "SCISSORS") == 0 && strcmp(move2, "PAPER") == 0) {
                snprintf(game_result1, BUFLEN, "R|W|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|L|%s||", move1);
            }
            else if (strcmp(move1, "SCISSORS") ==0 && strcmp(move2, "ROCK") == 0) {
                snprintf(game_result1, BUFLEN, "R|L|%s||", move2);
                snprintf(game_result2, BUFLEN, "R|W|%s||", move1);
            }

            write(pfds[0].fd, game_result1, strlen(game_result1));
            write(pfds[1].fd, game_result2, strlen(game_result2));                
            break;
        }                
    }
}

int didTheyLeave(struct pollfd* pfds) {
    int bytes, bytes2;
    char buf[BUFLEN] = "", buf2[BUFLEN] = "";
    int move1_ready = 0, move2_ready = 0;

    for (;;) {
        int ready = poll(pfds, 2, -1);
        if (ready == -1) {
            perror("poll");
            exit(EXIT_FAILURE);
        }
        if (pfds[0].revents && !move1_ready) {
            bytes = read(pfds[0].fd, buf, BUFLEN-1);
            if (bytes <= 0) {
                printf("Player1 disconnected\n");
                return 0;
            }
            buf[bytes] = '\0';
            move1_ready = 1;
        }
        if (pfds[1].revents && !move2_ready) {
            bytes2 = read(pfds[1].fd, buf2, BUFLEN-1);
            if (bytes2 <= 0) {
                printf("Player2 disconnected\n");
                return 0;
            }
            buf2[bytes2] = '\0';
            move2_ready = 1;
        }
        if (move1_ready && move2_ready) {
            if(strcmp(buf, "C") == 0 && strcmp(buf2, "C") == 0) {
                return 1;
            } else {
                return 0;
            }
        }
    }
}

int main(int argc, char **argv)
{
    int waiting_room[2];
    int count = 0;
    char *service = argc == 2 ? argv[1] : "55555";

    int listen = open_listener(service, 100);
    printf("Server is running (port %s) \n", service);

    for (;;) {
        int player = accept(listen, NULL, NULL);
        if (player < 0) {
            perror("accept");
            continue;
        }
        printf("Player connected!\n");

        waiting_room[count++] = player;

        if (count == 2) {
            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                exit(EXIT_FAILURE);
            }
            if (pid == 0) {
                close(listen);

                struct pollfd pfds[2];
                pfds[0].fd = waiting_room[0];
                pfds[1].fd = waiting_room[1];
                pfds[0].events = POLLIN;
                pfds[1].events = POLLIN;

                char player1[NAMELEN], player2[NAMELEN];
                readNames(pfds, player1, player2);

                if (strcmp(player1, player2) == 0 || is_duplicate(player1) || is_duplicate(player2)) {
                    write(pfds[0].fd, "R|F|Logged in||", strlen("R|F|Logged in||"));
                    write(pfds[1].fd, "R|F|Logged in||", strlen("R|F|Logged in||"));
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    exit(EXIT_SUCCESS);
                }

                add_name(player1);
                add_name(player2);

                processMoves(pfds, player1, player2);
                while (didTheyLeave(pfds))
                    processMoves(pfds, player1, player2);

                remove_name(player1);
                remove_name(player2);

                puts("Closing game");
                close(pfds[0].fd);
                close(pfds[1].fd);
                exit(EXIT_SUCCESS);
            } else {
                close(waiting_room[0]);
                close(waiting_room[1]);
                count = 0;
            }
        }

        while (waitpid(-1, NULL, WNOHANG) > 0);
    }

    close(listen);
    return 0;
}
