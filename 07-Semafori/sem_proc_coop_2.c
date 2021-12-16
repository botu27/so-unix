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
    printf("%d tenta di reservare il semaforo!\n", getpid());
    int reserve_response = reserveSem(s_id, 0);
    if(reserve_response != -1) {
        sleep(1);
        printf("%d tenta di rilasciare il semaforo!\n", getpid());
        int release_response = releaseSem(s_id, 0);
        if(release_response != -1)
        {
            printf("%d termina\n", getpid());
        }
        else
        {
            perror("releaseSem");
            exit(EXIT_FAILURE);
        }
    }
    else {
        perror("reserveSem");
        exit(EXIT_FAILURE);
    }
    int remove_respond = semctl(s_id, 0, IPC_RMID, NULL);
    if(remove_respond != -1)
    {
        printf("Semaforo[%d] deallocato\n", s_id);
    }
    else {
        perror("semctl(IPC_RMID)");
        exit(EXIT_FAILURE);
    }
    return 0;
}