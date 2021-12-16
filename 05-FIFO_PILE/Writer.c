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
    printf("WRITER[%d]\n", getpid());
    int fd;
    fd = open("miafifa", O_WRONLY);
    while(fd == -1) {
        sleep(1);
        fd = open("miafifa", O_WRONLY);
    }
    printf("FIFO trovata!\n");
    char str[100];
    memset(str, '\0', sizeof(str));
    for(int i = 0; i < 3; i++) {
        printf("Mando\n");
        sprintf(str, "Questo e' il %d messaggio\n", i);
        write(fd, str, sizeof(str));
    }
    close(fd);
}