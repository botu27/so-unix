#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() {
    printf("READER[%d]\n", getpid());
    int fd;
    char buffer[100];
    int n_bytes;
    switch(mkfifo("miafifa", 0666)) {
        case -1:
            perror("mkfifo");
            exit(EXIT_FAILURE);
        break;
        default:
            fd = open("miafifa", O_RDONLY);
            switch(fd) {
                case -1:
                    perror("open");
                    exit(EXIT_FAILURE);
                break;
                default:
                    memset(buffer, '\0', sizeof(buffer));
                    while(n_bytes = read(fd, buffer, sizeof(buffer))) {
                        printf("Ricevuto %d bytes: %s", n_bytes, buffer);
                        memset(buffer, '\0', sizeof(buffer));
                    }
                    close(fd);
                break;
            }
            
        break;
    }
}