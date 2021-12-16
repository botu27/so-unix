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
    int init = initSemInUse(s_id, 0);

    switch (fork())
    {
    case 0:
        sleep(5);
        int release = releaseSem(s_id, 0);
        printf("%d ha effettuato release!\n", getpid());
        exit(EXIT_SUCCESS);
        break;
    default:
        printf("%d tenta di reservare il semaforo\n", getpid());
        int reserve = reserveSem(s_id, 0);
        printf("%d ha effettuato reserve!\n", getpid());
        exit(EXIT_SUCCESS);
        break;
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
    }

    return 0;
}