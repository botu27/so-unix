#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>

int main() {
    FILE* fd = fopen("./file.txt", "r+");
    int status;
    int child_pid;

    if (fd) {
        printf("File aperto!\n");
        
        switch(fork()) {
            case 0:
                printf("Sono processo-figlio [%d]\n", getpid());
                printf("Mio processo-padre [%d]\n", getppid());

                for(int i = 0; i < 10; i++) {
                    fprintf(fd, "Devo fare %d esercizi, e mi sembrano tanti!\n", i+1);
                }
                exit(EXIT_SUCCESS);
            break;
            case -1:
                // error handler
            break;
            default:
                printf("Sono processo-padre [%d]\n", getpid());
                child_pid = wait(&status);
                printf("Processo-figlio [%d] ha terminato\n", child_pid);
                
                char buf[2048] = {};

                printf("%d", (int)buf[0]);

                fseek(fd, 0, SEEK_SET);

                printf("Ho letto:\n");
                while(fread(buf, sizeof(char), sizeof(buf)/sizeof(char), fd)) {
                    printf("%s", buf);
                }
                fclose(fd);

                exit(EXIT_SUCCESS);
            break;
        }
    } else {
        printf("File non e' stato aperto!");
        exit(EXIT_SUCCESS);
    }

    return 0;
}