#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
    int fd = open("input.txt", O_RDONLY);
    char buf[256];

    if (fd < 0) {
        perror("open failed");
        return 1;
    }
    
    int bytes = read(fd, buf, 255);
    if (bytes < 0) {
        perror("read failed");
        close(fd);
        return 1;
    }

    if (bytes == 0) {
        printf("File is empty!\n");
        close(fd);
        return 1;
    }
    buf[bytes] = '\0';

    int fd2 = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (fd2 < 0) {
        perror("open failed");
        close(fd);
        return 1;
    }
    

    int bytes_written = write(fd2, buf, bytes);

    if (bytes_written < 0) {
        perror("write");
        close(fd);
        close(fd2);
        return 1;
    }

    close(fd);
    close(fd2);

    printf("File copied successfully!\n");
    return 0;
}