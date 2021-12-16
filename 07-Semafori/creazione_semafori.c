#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <time.h>
#include "sem_utils.h"

#ifndef USING_SEM_BIN
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
#endif

/* int semget(key_t key, int nsems, int semflg);
    Returns semaphore set identifier on success,
    or -1 on error
    ***
    int semctl(int semid, int semnum, int cmd, ... (union semun arg) );
    Returns nonnegative integer on success; returns -1 on error
    ***
    int semop(int semid, struct sembuf *sops, unsigned int nsops);
    Returns 0 on success, or -1 on error
*/
int main()
{
    // CREAZIONE
    int n_sems = 1;
    key_t s_key = IPC_PRIVATE; // key, also can be used ftok(path, int) function
    /* 
    IPC_CREAT. Se non esiste un set di semafori con la chiave specificata, crea un nuovo set.
    IPC_EXCL. Se è stato specificato anche IPC_CREAT, e un set di semafori con la
    chiave specificata esiste già, restituisci un fallimento, con errore EEXIST 
    */
    int s_id = semget(s_key, n_sems, IPC_CREAT | 0600);
    SEM_CHECK(s_id);
    printf("Chiave di generazione utilizzata e' %d\n", s_key);
    printf("*** Set di semafori con id[%d] contiene [%d] semafori!***\n", s_id, n_sems);
    // INIZIALIZZAZIONE DEL SEMAFORO - opzione con array
    union semun s_semun;
    unsigned short semun_array[1] = {1};
    s_semun.array = semun_array;
    int sem_setall = semctl(s_id, 0, SETALL, s_semun);
    if (sem_setall == -1)
    {
        perror("semctl(SETALL)");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Valore di ritorno sem_setall[%d]\n", sem_setall);
    }
    // INIZIALIZZAZIONE DEL SEMAFORO - opzione con val
    int sem_getval = semctl(s_id, 0, GETVAL, NULL);
    if (sem_getval != -1)
    {
        printf("Valore corrente di sem_getval[%d]\n", sem_getval);
    }
    else
    {
        perror("semctl(GETVAL)");
        exit(EXIT_FAILURE);
    }
    int val = 0;
    s_semun.val = val;
    int sem_setval = semctl(s_id, 0, SETVAL, s_semun);
    if (sem_setval == -1)
    {
        perror("semctl(SETVAL)");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Valore di ritorno sem_setval[%d]\n", sem_setval);
    }
    sem_getval = semctl(s_id, 0, GETVAL, NULL);
    if (sem_getval != -1)
    {
        printf("Valore corrente di sem_getval[%d]\n", sem_getval);
    }
    else
    {
        perror("semctl(GETVAL)");
        exit(EXIT_FAILURE);
    }
    // STATISTICHE
    int cmd_stat = IPC_STAT;
    struct semid_ds buffer; // must be linked to union semid_ds &buf
    s_semun.buf = &buffer;
    int sem_stat = semctl(s_id, 0, cmd_stat, s_semun);
    if (sem_stat != -1)
    {
        printf("Valore sem_stat[%d]\n", sem_stat);
        printf("Statistiche IPC_STAT\n");
        printf("# semaphores in set: %ld\n", s_semun.buf->sem_nsems);
        printf("Last semop time: %ld\n", s_semun.buf->sem_otime);
        printf("Last change time: %ld\n", s_semun.buf->sem_ctime);
    }
    else
    {
        perror("semctl(IPC_STAT)");
        exit(EXIT_FAILURE);
    }
    printf("PERMISSIONS: %d\n", s_semun.buf->sem_perm.mode);
    // INFORMAZIONI SUL SEMAFORO
    /*
    Restituisce il PID dell'ultimo processo che ha eseguito una
    semop() su questo semaforo; questo è riferito come il valore sempid.
    Se nessun processo ha ancora eseguito una semop() su questo
    semaforo, restituisce 0.
    */
    int getpid = semctl(s_id, 0, GETPID, NULL);
    /*
    Restituisce il numero di processi attualmente in attesa di un
    incremento del valore del semaforo; questo è riferito come il valore
    semncnt.
    */
    int getcnt = semctl(s_id, 0, GETNCNT, NULL);
    /*
     Restituisce il numero di processi attualmente in attesa che il
    valore del semaforo divenga 0; questo è riferito come valore semzcnt.
    */
    int getzcnt = semctl(s_id, 0, GETZCNT, NULL);
    printf("PID of last semop[%d]\n", getpid);
    printf("# of process waiting to sem inc[%d]\n", getcnt);
    printf("# of process waiting to sem become 0[%d]\n", getzcnt);
    // MODIFICA DEI PERMESSI
    /*
    // definisco la mia ipc_perm, 
    cosi' prima copia i valori statistici del semaforo, 
    poi cambio il campo desiderato(avendo tutti i permessi necessari), 
    modifico il camposemun.buf->sem_perm con la mia struttura e 
    infine effettuo la semctl con FLAG IPC_SET
    */
    struct ipc_perm my_ipc_perm;
    my_ipc_perm = s_semun.buf->sem_perm;
    my_ipc_perm.mode = 0666;
    s_semun.buf->sem_perm = my_ipc_perm;
    int sem_ipcset = semctl(s_id, 0, IPC_SET, s_semun);
    sem_stat = semctl(s_id, 0, cmd_stat, s_semun);
    if (sem_stat != -1)
    {
        printf("Valore sem_stat[%d]\n", sem_stat);
        printf("Statistiche IPC_STAT\n");
        printf("# semaphores in set: %ld\n", s_semun.buf->sem_nsems);
        printf("Last semop time: %ld\n", s_semun.buf->sem_otime);
        printf("Last change time: %ld\n", s_semun.buf->sem_ctime);
    }
    else
    {
        perror("semctl(IPC_STAT)");
        exit(EXIT_FAILURE);
    }
    printf("PERMISSIONS: %d\n", s_semun.buf->sem_perm.mode);
    // OPERAZIONI SUI SEMAFORI
    /*
    La system call semop() esegue una o più operazioni sui
    semafori nel set identificato da semid.
    */
    struct sembuf sops[1];
    sops[0].sem_num = 0; // prendo in visione primo semaforo nel set
    sops[0].sem_flg = 0;
    /*
    maggiore di 0 -> incrementa il valore del semaforo, 
    == 0 si effettua il test se sem == 0, 
    se si' si ritorna, altrimenti si blocca finche' non diveti 0,
    <0 semaforo viene decrementato
    */
    sops[0].sem_op = 1;
    int sem_semop = semop(s_id, sops, 1);
    if (sem_semop != -1)
    {
        printf("Semop avvenuta con successo!\n");
    }
    else
    {
        perror("semop");
        exit(EXIT_FAILURE);
    }
    // TIMED SEMOP
    struct timespec s_semtimedop;
    s_semtimedop.tv_nsec = 0;
    s_semtimedop.tv_sec = 1;
    int sem_semtimedop = semtimedop(s_id, sops, 1, &s_semtimedop);
    if (sem_semtimedop != -1)
    {
        printf("Semtimedop avvenuta con successo!\n");
    }
    else
    {
        perror("semop");
        exit(EXIT_FAILURE);
    }
    // ELIMINAZIONE
    int cmd_rm = IPC_RMID; // specifica l'operazione,
    /*
    Rimuove immediatamente il set di semafori e
    l'associata struttura semid_ds. Qualsiasi processo bloccato in
    chiamate semop() in attesa su semafori è immediatamente
    svegliato, e semop() riporta l'errore EIDRM. semnum viene ignorato 
    */
    /*
    Quando un processo è bloccato in una semop() e riceve
    un segnale non mascherato:
    -> Il corrispondente handler è eseguito
    -> La semop() si sblocca, ritornando -1 e errno è settato a EINTR
   */
    int sem_rm = semctl(s_id, 0, cmd_rm, NULL);
    if (sem_rm == -1)
    {
        perror("sem_rm(IPC_RMID)");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Sem-set[%d] eliminato con successo!\n", s_id);
    }
    return 0;
}