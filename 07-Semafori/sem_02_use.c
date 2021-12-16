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
    int reserve = reserveSem(s_id, 0);
    if (reserve != -1)
    {
        printf("reserveSem OK responded with status[%d]\n", reserve);
    }
    else
    {
        perror("(releaseSem)");
    }
    return 0;
}