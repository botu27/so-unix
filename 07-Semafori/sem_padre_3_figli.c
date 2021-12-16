#define _GNU_SOURCE
#define NUM_KIDS 3

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>
#include "sem_utils.h"
#include "bin_sem_head.h"

int main()
{
    int sem_id = semget(DEF_SEM_ID, 1, IPC_CREAT | 0600);
    SEM_CHECK(sem_id);
    int init_response = initSemAvailable(sem_id, 0);
    int reserve_response;
    int release_response;
    size_t i = 0;
    if (init_response != -1)
    {
        // creo MAX_KIDS processi figli
        for (;i < NUM_KIDS; i++)
        {
            switch (fork())
            {
            case -1:
                perror("fork");
                exit(EXIT_FAILURE);
                break;
            case 0:
                srand(getpid());
                sleep(rand()%5+1);
                reserve_response = reserveSem(sem_id, 0);
                SEM_RESPONSE(reserve_response, "reserveSem");
                printf("Finalmente figlio %ld [%d] puo' scrivere a video!\n", i, getpid());
                sleep(3);
                release_response = releaseSem(sem_id, 0);
                SEM_RESPONSE(release_response, "releaseSem");
                exit(EXIT_SUCCESS);
                break;
            default:
                break;
            }
        }

        // attendo la terminazione di ciascun figlio
        int child_pid;
        int c_status;
        while ((child_pid = waitpid(-1, &c_status, 0)) != -1)
        {
            printf("%d ha terminato\n", child_pid);
            if (WIFEXITED(c_status))
            {
                printf("Child terminated normally with status %d\n", WEXITSTATUS(c_status));
            }
            else
            {
                printf("Unexpected status\n");
            }
        }

        if (errno == ECHILD)
        {
            printf("No child exist!\n");
        }
        else
        {
            perror("errno not ECHILD");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("semInitAvailable");
        exit(EXIT_FAILURE);
    }
    return 0;
}