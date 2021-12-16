#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

/*
    ***
    int shmget(key, size in byte, flags) - per creare
    un nuovo segmento di SM o per ottenere l'ID
    del segmento gia' esistente
    ***
    void* shmat(id, puntatore all'indirizzo, flags) -
    - per attaccare il segmento di SM; cioè, per rendere il
    segmento in questione parte della memoria virtuale del 
    processo chiamante
    ***
    int shmdt(void *addr) - per staccare il segmento di SM. 
    Dopo tale chiamata, il processo non può più fare riferimento alla SM.
    Occorre automaticamente alla terminazione del
    processo.
    ATTENZIONE con *addr, uso non raccomandabile.
    se *addr punta alla cella di memoria in utilizzo - chiamata fallisce
    ***
    int shmctl(id, comando, semid_ds *buf) - per cancellare il segmento di SM.
    Il segmento sarà effettivamente distrutto solo dopo che tutti i processi
    correntemente attaccati lo avranno staccato. UN SOLO processo effettua 
    la cancellazione.
*/

int main(int argc, char const *argv[])
{
    int shmemId;
    int *ptr;
    int shmdtResponse;
    int shmctlResponse;
    char buf[80];
    struct tm timestamp;
    struct shmid_ds shmemData;

    shmemId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0600);
    if(shmemId == -1)
    {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    ptr = (int *)shmat(shmemId, NULL, 0);
    if(*ptr == -1)
    {
        perror("shmat");
        exit(EXIT_FAILURE);
    }
    *ptr = 4;
    printf("Valore puntato da intPtr e' %d\n", *ptr);
    
    shmctlResponse = shmctl(shmemId, IPC_STAT, &shmemData);
    if(shmctlResponse == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    printf("DATA:\n");
    printf("Size of segment in BYTES: %ld\n", shmemData.shm_segsz);
    timestamp = *localtime(&shmemData.shm_atime);
    strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &timestamp);
    printf("Time of last attachemnt: %s\n", buf);
    printf("# Currently attached processes: %ld\n", shmemData.shm_nattch);

    shmdtResponse = shmdt(ptr);
    if(shmdtResponse == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    printf("Segmento della memoria staccato con successo!\n");

    shmctlResponse = shmctl(shmemId, IPC_RMID, NULL);
    if(shmctlResponse == -1)
    {
        perror("shmdt");
        exit(EXIT_FAILURE);
    }
    printf("Segmento della memoria condiviso eliminato correttamente!\n");
    return 0;
}
