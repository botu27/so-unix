#define _GNU_SOURCE
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include "sem_utils.h"
#include "bin_sem_head.h"

int main()
{
    int s_id = semget(DEF_SEM_ID, 1, IPC_CREAT | 0600);
    SEM_CHECK(s_id);
    int init_response = initSemAvailable(s_id, 0);
    if (init_response != -1)
    {
        printf("%d tenta di reservare il semaforo!\n", getpid());
        int reserve_response = reserveSem(s_id, 0);
        if (reserve_response != -1)
        {
            sleep(5);
            printf("%d tenta di rilasciare il semaforo!\n", getpid());
            int release_response = releaseSem(s_id, 0);
            if (release_response != -1)
            {
                printf("%d termina\n", getpid());
            }
            else
            {
                perror("releaseSem");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            perror("reserveSem");
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