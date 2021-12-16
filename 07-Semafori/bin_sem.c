#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include "bin_sem_head.h"

union semun
{
    int val;               // value for SEMVAL
    struct semid_ds *buf;  // buffer for IPC_STAT, IPC_SET
    unsigned short *array; // array for GETALL, SETALL
// Linux specific part
#if defined(__linux__)
    struct seminfo *__buf; // buffer for IPC_INFO
#endif
};

/*
Initialize semaphore to 1 (i.e., "available")
*/
int initSemAvailable(int semId, int semNum)
{
    union semun arg;
    arg.val = 1;
    return semctl(semId, semNum, SETVAL, arg);
}

/*
Initialize semaphore set to 1 (i.e., "available")
*/
int initSemsetAvailable(int semId, int size)
{
    union semun arg;
    unsigned short *arr = calloc(size, sizeof(short));
    for (int i = 0; i < size; i++)
    {
        arr[i] = 1;
    }
    arg.array = arr;
    return semctl(semId, 0, SETALL, arg);
}

/*
Initialize semaphore to 0 (i.e., "in use")
*/
int initSemInUse(int semId, int semNum)
{
    union semun arg;
    arg.val = 0;
    return semctl(semId, semNum, SETVAL, arg);
}

/*
Initialize semaphore set to 1 (i.e., "available")
*/
int initSemsetInUse(int semId, int size)
{
    union semun arg;
    unsigned short *arr = calloc(size, sizeof(short));
    for (int i = 0; i < size; i++)
    {
        arr[i] = 0;
    }
    arg.array = arr;
    return semctl(semId, 0, SETALL, arg);
}

/*
Reserve semaphore - decrement it by 1
*/
int reserveSem(int semId, int semNum)
{
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = -1;
    sops.sem_flg = 0;
    return semop(semId, &sops, 1);
}

/*
Release semaphore - increment it by 1
*/
int releaseSem(int semId, int semNum)
{
    struct sembuf sops;
    sops.sem_num = semNum;
    sops.sem_op = 1;
    sops.sem_flg = 0;
    return semop(semId, &sops, 1);
}