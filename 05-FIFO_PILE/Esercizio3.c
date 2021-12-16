#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

#define NUM_KIDS 50
#define NUM_MAX 10
#define MAX_VALUE 1000

int end=0;

void signalHaldler(int sig) {
  switch (sig) {
    case SIGINT:
      printf("CHILD %d - Received: SIGINT\n",getpid());
      end=1;
      break;
    case SIGUSR1:
      printf("CHILD %d - Received: SIGUSR1\n",getpid());
      break;
    case SIGUSR2:
      printf("CHILD %d - Received: SIGUSR2\n",getpid());
      break;
  }
  printf("CHILD %d - ok\n",getpid());
}


int main() {
    printf("Padre[%d]\n", getpid());
    int fdes[2];
    int i;
    int cpid;
    int pidt[NUM_KIDS + 1];
    int chld[NUM_KIDS + 1] = {};
    int value[2];
    int n_bytes;
    memset(pidt, 0, sizeof(pidt));
    memset(chld, 0, sizeof(chld));
    switch(pipe(fdes)) {
        case -1:
            perror("pipe");
            exit(EXIT_FAILURE);
        break;
        default:
            for(i = 0; i < NUM_KIDS; i++) {
                switch(cpid = fork()) {
                    case -1:
                        perror("fork");
                        exit(EXIT_FAILURE);
                    break;
                    case 0:
                        // sono filgio
                        printf("Figlio[%d] %d-simo\n", getpid(), i);
                        close(fdes[0]);
                        srand(getpid());
                        value[0] = i;
                        // value[1] = rand()%MAX_VALUE + 1;
                        struct sigaction sa;
                        sigset_t mask;
                        sigemptyset(&mask);
                        sigaddset(&mask, SIGINT);
                        sigaddset(&mask, SIGUSR1);
                        sigset_t mask_empty;
                        sigemptyset(&mask_empty);
                        sa.sa_mask = mask_empty;
                        sa.sa_handler = signalHaldler;
                        sa.sa_flags = 0;
                        sigaction(SIGINT, &sa, NULL);
                        sigaction(SIGUSR1, &sa, NULL);
                        // printf("Ho generato %d\n", value[1]);
                        sigprocmask(SIG_SETMASK, &mask, NULL);
                        while(!end) {
                            value[1] = rand()%MAX_VALUE + 1;
                            n_bytes = write(fdes[1], &value, sizeof(value));
                            if(n_bytes < 0) {
                                fprintf(stderr, "CHILD %d  - %s: %d. Error in writing pipe #%03d: %s\n", getpid(), __FILE__, __LINE__, errno, strerror(errno));
                            }
                            sigsuspend(&mask_empty);
                        }
                        close(fdes[1]);
                        exit(EXIT_SUCCESS);
                    break;
                    default:
                        // no op
                        pidt[i] = cpid;
                    break;
                }
            }
            close(fdes[1]);
            // sleep(1);
            // for(int j = 0; j < NUM_KIDS; j++) {
            //     printf("%d\t", pidt[j]);
            // }
            printf("\n");
            int num_kids = NUM_KIDS;
            int min_value = MAX_VALUE;
            int min_value_proc_pid = -1;
            while(num_kids) {
                min_value = MAX_VALUE;
                for(int j = 0; j < num_kids; j++) {
                    n_bytes = read(fdes[0], &value, sizeof(value));
                    if(n_bytes < 0) {
                        fprintf(stderr, "%s: %d. Error in reading pipe #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
                    }

                    chld[j] = pidt[value[0]];

                    if(value[1] < min_value) {
                        min_value = value[1];
                        min_value_proc_pid = pidt[value[0]];
                    }
                }

                for(int k = 0; k < num_kids; k++) {
                    if(chld[k] == min_value_proc_pid) {
                        kill(chld[k], SIGINT);
                        printf("PARENT - Sent SIGINT to %d\n",chld[k]);
                    }
                    else {
                        kill(chld[k], SIGUSR1);
                        printf("PARENT - Sent SIGUSR1 to %d\n",chld[k]);
                    }
                }
                // printf("Ricevuto da [%d] %d bytes : %d\n", pidt[value[0]],n_bytes, value[1]);
                num_kids--;
            }
            printf("Valore minimo ricevuto: %d\n", min_value);
            close(fdes[0]);
        break;
    }
    return 0;
}