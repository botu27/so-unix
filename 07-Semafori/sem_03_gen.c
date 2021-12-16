#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
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
    int remove = semctl(s_id, 0, IPC_RMID, NULL);
    if (remove != -1)
    {
        printf("Semaforo[%d] eliminato\n", s_id);
    }
    else
    {
        perror("initSemInUse");
    }

    return 0;
}