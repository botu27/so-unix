#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int filedes[2] = {};
    char buffer[100];
    int n_bytes = -1;
    memset(buffer, '\0', sizeof(buffer));
    printf("Buffer Size : %d\n", BUFSIZ);
    if(pipe(filedes) == 0) {
        printf("PADRE[%d]\n", getpid());
        switch(fork()) {
            case -1:
                perror(strerror(errno));
                exit(EXIT_FAILURE);
            break;
            case 0:
                printf("FIGLIO[%d]\n", getpid());
                close(filedes[0]);
                char str[] = "Ciao, Fratello, come stai!";
                n_bytes = write(filedes[1], str, sizeof(str));
                printf("Invio %d bytes", n_bytes);
                close(filedes[1]);
            break;
            default:
                    switch(fork()) {
                        case -1:
                            perror(strerror(errno));
                            exit(EXIT_FAILURE);
                        break;
                        case 0:
                            printf("FIGLIO[%d]\n", getpid());
                            close(filedes[1]);
                            n_bytes = read(filedes[0], buffer, sizeof(buffer));
                            printf("Ricevuto %d bytes: %s\n", n_bytes, buffer);
                            close(filedes[0]);
                        break;
                        default:
                            close(filedes[0]);
                            close(filedes[1]);
                        break;
                }
            break;
        }
    } else {
        perror(strerror(errno));
        exit(EXIT_FAILURE);
    }
}