#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#define SIZE 256
#define BUFFER_SIZE 256 // 한 번에 읽을 최대 바이트 수

/*int main() {
    int fd = open("input.txt", O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }
    
    char buf[SIZE];
    char line[1024];
    int bytes;
    int line_pos = 0;
    while ((bytes = read(fd, buf, SIZE)) > 0) {
        for (int i = 0; i < bytes; i++) {
            if (buf[i] == '\n') {
                line[line_pos] = '\0';
                printf("Line: %s\n", line);
                line_pos = 0;
            } else {
                line[line_pos++] = buf[i];
            }
        }
    }
}*/

int main() {
    int fd = open("input.txt", O_RDONLY);
    if (fd < 0) {
        perror("open failed");
        return 1;
    }

    char buf[BUFFER_SIZE];
    char *line = malloc(128);
    size_t line_size = 128;
    size_t line_pos = 0;
    int bytes;

    while ((bytes = read(fd, buf, BUFFER_SIZE - 1)) > 0) {
        for (int i = 0; i < bytes; i++) {
            if (line_pos + 1 >= line_size) {    // 남은 공간이 부족하면 크기를 증가
                line_size *= 2; // 기존 크기의 2배로 증가
                line = realloc(line, line_size);
            }

            if (buf[i] == '\n') {
                line[line_pos] = '\0';
                printf("Line: %s\n", line);
                line_pos = 0;
            } else {
                line[line_pos++] = buf[i];
            }
        }
    }

    free(line);
    close(fd);
    return 0;
}