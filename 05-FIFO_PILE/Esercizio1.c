#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

int main() {
    int filedes[2] = {};
    printf("Buffer Size : %d\n", BUFSIZ);
    if(pipe(filedes) == 0) {
        char buf[32] = {};
        int n_bytes;
        int value = 0;
        memset(buf, '\0', sizeof(buf));
        pid_t fork_res;
        fork_res = fork();
        switch(fork_res) {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
            break;
            case 0:
                printf("FILGIO[%d]\n", getpid());
                close(filedes[0]); // chiudo ramo di lettura
                // char str[32];
                // memset(str, '\0', sizeof(str));
                srand(getpid());
                for(int i = 0; i < 10; i++) {
                    // sprintf(str, "%d ", (rand()%1000 + 1));
                    value = rand()%1000 +1;
                    n_bytes = write(filedes[1], &value, sizeof(value));
                    // printf("Scrivo %d bytes\n", n_bytes);
                    // memset(str, '\0', sizeof(str));
                }
                close(filedes[1]);
            break;
            default:
                printf("PADRE[%d]\n", getpid());
                close(filedes[1]); // chiudo ramo di scrittura
                while(n_bytes = read(filedes[0], &value, sizeof(value))) {
                    printf("Ricevuto %d bytes : %d\n", n_bytes, value);
                    // memset(buf, '\0', sizeof(buf));
                }
                close(filedes[0]);
            break;
        }
    } else {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
}