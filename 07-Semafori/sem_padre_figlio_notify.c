#define _GNU_SOURCE

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
    int init_response = initSemInUse(sem_id, 0);
    SEM_RESPONSE(init_response, "initSemInUse");
    int reserve_response;
    int release_response;
    switch (fork())
    {
    case 0:
        printf("Attendo la notifica dal processo padre!\n");
        reserve_response = reserveSem(sem_id, 0);
        SEM_RESPONSE(reserve_response, "reserveSem");
        printf("Sezione critica\n");
        release_response = releaseSem(sem_id, 0);
        SEM_RESPONSE(release_response, "releaseSem");
        printf("Risorsa rilasciata\n");
        exit(EXIT_SUCCESS);
        break;
    case -1:
        perror("fork");
        exit(EXIT_FAILURE);
        break;
    default:
        sleep(3);
        release_response = releaseSem(sem_id, 0);
        SEM_RESPONSE(release_response, "releaseSem");
        printf("Figlio notificato!\n");
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

        int remove = semctl(sem_id, 0, IPC_RMID, NULL);
        if (remove != -1)
        {
            printf("Semaforo[%d] eliminato\n", sem_id);
        }
        else
        {
            perror("initSemInUse");
        }
        break;
    }
    return 0;
}