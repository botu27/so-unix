#define _GNU_SOURCE
#define _POSIX_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>

#define TEST_ERROR                                 \
    if (errno)                                     \
    {                                              \
        fprintf(stderr,                            \
                "%s:%d: PID=%5d: Error %d (%s)\n", \
                __FILE__,                          \
                __LINE__,                          \
                getpid(),                          \
                errno,                             \
                strerror(errno));                  \
    }

/*MACRO PER POTER ABILITARE TEST MODE*/
#define ENABLE_TEST 1
/*MACRO PER GESTIRE TRANSACTION POOL*/
#define NOT_FULL 0
#define FULL 1
#define TRANSAZIONE_OK 1
#define TRANSAZIONE_KO 0

typedef struct transazione
{
    struct timespec timestamp;
    pid_t sender;
    pid_t receiver;
    int quantita;
    int reward;
} transazione;

/*La Transaction Pool e' gestita mediante la seguente struttura*/
typedef struct transazione_tp
{
    int state;
    transazione transazione;
} transazione_tp;

struct msgbuf
{
    long mtype;
    transazione body;
};

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
#if defined(__linux__)
    struct seminfo *__buf;
#endif
};

/*PER READ ALL PARAMETERS*/
int SO_USERS_NUM;
int SO_NODES_NUM;
int SO_REWARD;
long SO_MIN_TRANS_GEN_NSEC;
long SO_MAX_TRANS_GEN_NSEC;
int SO_RETRY;
int SO_TP_SIZE;
int SO_BLOCK_SIZE;
long SO_MIN_TRANS_PROC_NSEC;
long SO_MAX_TRANS_PROC_NSEC;
int SO_REGISTRY_SIZE;
int SO_BUDGET_INIT;
int SO_SIM_SEC;
int SO_FRIENDS_NUM;

/*Inizializza le variabili globali da un file di input TXT. */
int read_all_parameters();
void alarmHandler(int);
int getRandomUser(int, int, int *);
int getMyBilancio(int, transazione *, int *, int);
void initDummyTransactionPool(transazione_tp *, int);
int isFull(transazione_tp *, int);
int getNextIndex(transazione_tp *, int);
void attesaNonAttiva(struct timespec *, struct timespec *, long, long);
void marcaTransazioni(transazione_tp *, transazione *, int, int);

int main()
{
    /* SOLO PER TEST */
#if (ENABLE_TEST)
    if (read_all_parameters() == -1)
    {
        fprintf(stderr, "Parsing failed!");
    }
#endif

    /****ID DELLE MEMORIE CONDIVISE****/
    int shmIdMastro;
    int shmIdNodo;
    int shmIdUtente;
    int shmIdIndiceBlocco;
    /**********************************/
    /*PUNTATORI ALLE STRUTTURE CONTENUTE ALL'INTERNO DELLE MEMORIE CONDIVISE*/
    int *shmArrayNodeMsgQueuePtr;
    int *shmArrayUsersPidPtr;
    int *shmIndiceBloccoPtr;
    transazione *shmLibroMastroPtr;
    /************************************************************************/
    /*ID DELLE CODE DI MESSAGGI*/
    int msgQueueId;
    /***************************/
    /*ID DEI SEMAFORI*/
    int semSetMsgQueueId;
    int semSyncStartId;
    int semArrUserPidId;
    int semIndiceLibroMastro;
    /***************************/
    /*STRUTTURE DI SUPPORTO PER ESEGUIRE LE OPERAZIONI SUI SEMAFORI*/
    struct sembuf sops;
    union semun semMsgQueueUnion; /*union semctl*/
    /*ATTENZIONE: la riga seguente contiene del codice che non rispetta lo standard c89*/
    /*unsigned short semMsgQueueValArr[SO_NODES_NUM];  /*dichiaro array di union semctl*/
    /*ALTERNATIVA - allocazione dinamica con la calloc*/
    unsigned short *semMsgQueueValArr;
    /***************************************************************/
    /*STRUTTURE PER GESTIONE DEI SEGNALI*/
    struct sigaction sigactionForAlarmNew;
    struct sigaction sigactionForAlarmOld;
    sigset_t maskSetForAlarm;
    /************************************/
    /*STRUTTURE RELATIVE AI NODI*/
    transazione_tp *transactionPool;
    transazione *blocco;
    int nextTransactionPoolIndex; /*contiene il prossimo indice valido*/
    int nextBloccoIndex;          /*contiene il prossimo indice valido*/
    struct msgbuf msgbuf;         /*buffer dove salvare i dati ricevuti dalla coda di messaggi*/
    int bI;
    int isTransactionPoolFull;
    transazione transazioneReward; /*ultimo campo del blocco da salvare nel registro*/
    int rewardNodo;
    struct timespec timeToSleep;
    struct timespec remainedTimeToSleep;
    struct sigaction sigactionForNode;
    sigset_t maskSetForNode;
    /****************************/
    /*VARIABILI AUSILIARI*/
    int i;
    int childPid;
    int childPidWait;
    int childStatus;
    int *arrayChildrenNodePid; /*struttura dati locale al processo master, contiene tutti i PID dei processi NODO*/
    /*********************/

/*TEST SIGACTION*/
#if (ENABLE_TEST)
    sigactionForAlarmNew.sa_flags = 0; /*no flags*/
    sigactionForAlarmNew.sa_handler = &alarmHandler;
    sigemptyset(&maskSetForAlarm);
    sigactionForAlarmNew.sa_mask = maskSetForAlarm;
    sigaction(SIGALRM, &sigactionForAlarmNew, NULL);

    alarm(SO_SIM_SEC);
#endif

/*SOLO PER TEST*/
#if (ENABLE_TEST)
    union semun TEST_GETALL;
    /*unsigned short TEST_GETALL_ARRAY[SO_NODES_NUM];*/
    unsigned short *TEST_GETALL_ARRAY;
    TEST_GETALL_ARRAY = (unsigned short *)calloc(SO_NODES_NUM, sizeof(unsigned short));
#endif

    /*Inizializzazione semaforo accesso esclusivo all'indice*/
    semIndiceLibroMastro = semget(IPC_PRIVATE, 1, 0600 | IPC_CREAT);
    TEST_ERROR;
    /*Assegno valore 1 al semaforo di indice*/
    sops.sem_op = 1;
    sops.sem_flg = 0;
    sops.sem_num = 0;
    semop(semIndiceLibroMastro, &sops, 1);

    /*Inizializzazione memoria condivisa per libro mastro*/
    shmIdMastro = shmget(IPC_PRIVATE, SO_REGISTRY_SIZE * SO_BLOCK_SIZE * sizeof(transazione), 0600 | IPC_CREAT);

    /*Inizializzazione memoria condivisa per indice blocco libro mastro*/
    shmIdIndiceBlocco = shmget(IPC_PRIVATE, sizeof(int), 0600 | IPC_CREAT);
    /*Assegno indice iniziale all'id dei blocchi*/
    shmIndiceBloccoPtr = (int *)shmat(shmIdIndiceBlocco, NULL, 0);
    *(shmIndiceBloccoPtr) = 0;

/* SOLO PER TEST */
#if (ENABLE_TEST)
    printf("\nid libro mastro[%d]\n", shmIdMastro);
#endif

    /* Attach del libro mastro alla memoria condivisa*/
    shmLibroMastroPtr = (transazione *)shmat(shmIdMastro, NULL, 0);
    TEST_ERROR;

    /*inizializzazione memoria condivisa array ID coda di messaggi*/
    shmIdNodo = shmget(IPC_PRIVATE, SO_NODES_NUM * sizeof(int), 0600 | IPC_CREAT);
    shmArrayNodeMsgQueuePtr = (int *)shmat(shmIdNodo, NULL, 0); /*eseguo attach per riempire l'array di ID di code di messaggi*/

/* SOLO PER TEST */
#if (ENABLE_TEST)
    printf("\nLa shared memory dell'array contenente gli ID delle message queue è: %d\n", shmIdNodo);
#endif

    /*riempio l'arrayCodeID con gli ID delle code messaggi prima di entrare nel for per la creazione dei nodi*/
    for (i = 0; i < SO_NODES_NUM; i++)
    {
        msgQueueId = msgget(IPC_PRIVATE, 0600 | IPC_CREAT);
        *(shmArrayNodeMsgQueuePtr + i) = msgQueueId;
    }

/* SOLO PER TEST */
#if (ENABLE_TEST)
    for (i = 0; i < SO_NODES_NUM; i++)
    {
        printf("STAMPO TUTTI GLI ID DELLE CODE DI MESSAGGI CREATE\n");
        printf("ID coda n %d = %d\n", i, *(shmArrayNodeMsgQueuePtr + i));
    }
#endif

    /*Inizializzo set semafori per gestire invio transazioni da parte degli utenti*/
    semSetMsgQueueId = semget(IPC_PRIVATE, SO_NODES_NUM, 0600 | IPC_CREAT);
    TEST_ERROR;
    /*INIZIALIZZO L'ARRAY ATTRAVERSO UNA CHIAMATA DI CALLOC*/
    semMsgQueueValArr = (unsigned short *)calloc(SO_NODES_NUM, sizeof(unsigned short));
    /*Popolo array valore semafori */
    for (i = 0; i < SO_NODES_NUM; i++)
    {
        semMsgQueueValArr[i] = SO_TP_SIZE;
        printf("Valore indice %d = %d\n", i, semMsgQueueValArr[i]);
    }

    /*assegno array valori semafori alla union*/
    semMsgQueueUnion.array = semMsgQueueValArr;

    /*imposto valore semafori*/
    semctl(semSetMsgQueueId, 0, SETALL, semMsgQueueUnion);
    TEST_ERROR;

/*SOLO PER TEST*/
#if (ENABLE_TEST)
    TEST_GETALL.array = TEST_GETALL_ARRAY;
    semctl(semSetMsgQueueId, 0, GETALL, TEST_GETALL);
    for (i = 0; i < SO_NODES_NUM; i++)
    {
        printf("Valore semaforo ID : %d -> %d\n", i, TEST_GETALL.array[i]);
    }
#endif

    /*inizializzazione memoria condivisa array PID processi utente*/
    shmIdUtente = shmget(IPC_PRIVATE, SO_USERS_NUM * sizeof(int), 0600 | IPC_CREAT);
    shmArrayUsersPidPtr = (int *)shmat(shmIdUtente, NULL, 0);

    /*inizializzazione semaforo per garantire l'accesso alla memoria - TODO*/

/* SOLO PER TEST */
#if (ENABLE_TEST)
    printf("\nLa shared memory dell'array contenente gli ID delle message queue è: %d\n", shmIdUtente);
#endif

    /*Inizializzazione del semaforo che si occupa di avviare tutti i processi NODO*/
    semSyncStartId = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    TEST_ERROR
#if (ENABLE_TEST)
    printf("ID del semaforo di sincronizzazione dei processi -> %d", semSyncStartId);
#endif
    sops.sem_flg = 0; /*no flags*/
    sops.sem_num = 0; /*primo semaforo nel set*/
    sops.sem_op = SO_NODES_NUM + 1;
    semop(semSyncStartId, &sops, 1);
    TEST_ERROR

    for (i = 0; i < SO_NODES_NUM; i++)
    {
        switch (childPid = fork())
        {
        case -1:
            fprintf(stderr, "%s, %d: Errore (%d) nella fork\n",
                    __FILE__, __LINE__, errno);
            exit(EXIT_FAILURE);
            break;
        case 0:
            /*Codice che eseguira ogni NODO*/

            /*Avviso PARENT che sono nato*/
            sops.sem_flg = 0;
            sops.sem_num = 0;
            sops.sem_op = -1;
            semop(semSyncStartId, &sops, 1);
            TEST_ERROR

            /*Mi metto in attesa, finche' non ricevo VERDE dal PARENT*/
            sops.sem_flg = 0;
            sops.sem_num = 0;
            sops.sem_op = 0;
            semop(semSyncStartId, &sops, 1);
            TEST_ERROR

            /*Qui posso effetivante iniziare la propria parte*/
            /*effettuo l'attach al libro mastro*/
            shmLibroMastroPtr = (transazione *)shmat(shmIdMastro, NULL, 0);
            /*Creo la transaction pool*/
            transactionPool = calloc(SO_TP_SIZE, sizeof(transazione_tp));
            /*Prima cella utile e' la 0-esima*/
            nextTransactionPoolIndex = 0;
            /*Popolo la transaction pool con membri DUMMY*/
            initDummyTransactionPool(transactionPool, SO_TP_SIZE);
            isTransactionPoolFull = NOT_FULL; /**/
            /*Creo il block-buffer*/
            blocco = calloc(SO_BLOCK_SIZE, sizeof(transazione));
            nextBloccoIndex = 0;
            bI = 0;
            /*creo la transazione di reward*/
            transazioneReward.sender = -1; /*MACRO!!!*/
            transazioneReward.receiver = getpid();
            transazioneReward.reward = 0;
            transazioneReward.quantita = 0;
            /*Caso migliore - NODO e' sempre disponibile per gestire le transazioni*/
            while (1)
            {
                /*Pesco primo messaggio dalla coda, se la coda e' vuota - ritorna con errno settato*/
                msgrcv(*(shmArrayNodeMsgQueuePtr + i), &msgbuf, sizeof(transazione_tp), 0, IPC_NOWAIT);
                if (errno == EAGAIN)
                {
                    /*Caso della coda vuota*/
                    /*DO NOTHING - da ripensare*/
                }
                else
                {
                    /*Caso generale : la coda di messaggi non e' vuota*/
                    /*TP ha delle celle disponibili*/
                    if (!isTransactionPoolFull)
                    {
                        /*copio il body del messaggio*/
                        transactionPool[nextTransactionPoolIndex].transazione = msgbuf.body;
                        transactionPool[nextTransactionPoolIndex].state = TRANSAZIONE_OK;
                        nextTransactionPoolIndex = getNextIndex(transactionPool, SO_TP_SIZE);
                        if (nextTransactionPoolIndex == -1)
                        {
                            isTransactionPoolFull = FULL;
                        }
                        else
                        {
                            isTransactionPoolFull = NOT_FULL;
                        }
                    }
                    /*TP e' piena*/
                    else
                    {
                        /*registro la transazione da memoriazzare nel libro mastro*/
                        blocco[bI] = transactionPool[nextBloccoIndex].transazione;
                        /*aggiorno il campo di quantita'*/
                        transazioneReward.quantita = transactionPool[nextBloccoIndex].transazione.reward;
                        /*punto alla successiva cella occupata da gestire, se sono qua alloa la TP e' piena*/
                        nextBloccoIndex++;
                        if (nextBloccoIndex == SO_TP_SIZE)
                        {
                            nextBloccoIndex = 0;
                        }
                        bI++;
                        /*blocco pronto da inviare*/
                        if (bI == SO_BLOCK_SIZE - 1)
                        {
                            clock_gettime(CLOCK_BOOTTIME, &transazioneReward.timestamp);
                            blocco[bI] = transazioneReward;
                            /*invio la transazione*/
                            /*riservo l'indice*/
                            sops.sem_flg = 0;
                            sops.sem_num = 0;
                            sops.sem_op = -1;
                            semop(semIndiceLibroMastro, &sops, 1);
                            TEST_ERROR;

                            /*Effettuto l'operazione di scrittura sul libro mastro*/
                            /*effettuo l'attach al indice blocco del libro mastro*/
                            shmIndiceBloccoPtr = (transazione *)shmat(shmIdIndiceBlocco, NULL, 0);
                            TEST_ERROR;
                            bI = 0;
                            for (bI; bI < SO_BLOCK_SIZE; bI++)
                            {
                                shmLibroMastroPtr[*(shmIndiceBloccoPtr)*SO_BLOCK_SIZE + bI] = blocco[bI];
                            }
                            /*effettuo la detach*/
                            shmdt(shmIndiceBloccoPtr);
                            TEST_ERROR;

                            /*maschero alcuni segnali*/
                            sigactionForNode.sa_flags = 0;
                            sigemptyset(&sigactionForNode.sa_mask);
                            sigaddset(&sigactionForNode.sa_mask, SIGINT);
                            sigprocmask(SIG_BLOCK, &sigactionForNode.sa_mask, NULL);
                            /*ispirato da 06-test-signal-mask*/
                            /*************************/
                            attesaNonAttiva(&timeToSleep, &remainedTimeToSleep, SO_MIN_TRANS_PROC_NSEC, SO_MAX_TRANS_PROC_NSEC);
                            /*recupero la maschera*/
                            sigaddset(&sigactionForNode.sa_mask, SIGINT);
                            sigprocmask(SIG_UNBLOCK, &sigactionForNode.sa_mask, NULL);
                            /**********************/

                            /*incremento l'indice*/
                            *(shmIndiceBloccoPtr) += 1;
                            /*verifico eventuale riempimento del registro*/
                            if (*(shmIndiceBloccoPtr) == SO_REGISTRY_SIZE)
                            {
                                /*avviso il PADRE*/
                            }

                            /*rilascio l'indice*/
                            sops.sem_flg = 0;
                            sops.sem_num = 0;
                            sops.sem_op = 1;
                            semop(semIndiceLibroMastro, &sops, 1);
                            TEST_ERROR;

                            bI = 0;

                            /*Marcare le transazioni inviate*/
                            marcaTransazioni(transactionPool, blocco, SO_TP_SIZE, SO_BLOCK_SIZE);
                            /*aggiorno la cella dove andare a scrivere*/
                            nextTransactionPoolIndex = getNextIndex(transactionPool, SO_TP_SIZE);
                            isTransactionPoolFull = NOT_FULL;
                        }
                    }
                }
            }

        default:
            break;
        }
    }

    /* creazione figli users con operazioni annesse */
    for (i = 0; i < SO_USERS_NUM; i++)
    {
        switch (childPid = fork())
        {

        case -1:
            fprintf(stderr, "%s, %d: Errore (%d) nella fork\n",
                    __FILE__, __LINE__, errno);
            exit(EXIT_FAILURE);
            break;
        case 0:
/* SOLO PER TEST*/
#if (ENABLE_TEST)
            printf("sono il figio: %d, in posizione: %d\n", getpid(), i);
#endif
            sops.sem_flg = 0;
            sops.sem_num = 0;
            sops.sem_op = -1;
            semop(semSyncStartId, &sops, 1);
            TEST_ERROR
/* SOLO PER TEST*/
#if (ENABLE_TEST)
            printf("%d is waiting for zero\n", i);
#endif
            sops.sem_flg = 0;
            sops.sem_num = 0;
            sops.sem_op = 0;
            semop(semSyncStartId, &sops, 1);
#if (ENABLE_TEST)
            printf("%d sono risvegliato\n", i);
#endif
            printf("SONO %d e Voglio creare la transazione e inviarla al nodo : %d\n", getpid(), getRandomUser(SO_USERS_NUM, getpid(), shmArrayUsersPidPtr));
            exit(EXIT_SUCCESS);
            break;

        default:
            /* riempimento array PID utenti nella memoria condivisa */
            *(shmArrayUsersPidPtr + i) = childPid;
            break;
        }
    }
    /* SOLO PER TEST*/
#if (ENABLE_TEST)
    printf("Sono #%d processo PARENT e risveglio i figli!\n", getpid());
#endif
    sops.sem_flg = 0;
    sops.sem_num = 0;
    sops.sem_op = -1;
    semop(semSyncStartId, &sops, 1);
#if (ENABLE_TEST)
    printf("Sono #%d processo PARENT e ho risvegliato i figli!\n", getpid());
#endif

    /*POSSIBILE IMPLEMENTAZIONE DELL'ATTESA DELLA TERMINAZIONE DEI FIGLI*/
    while ((childPidWait = waitpid(-1, &childStatus, 0)) != -1)
    {
        if (WIFEXITED(childStatus))
        {
            printf("%d terminated with status %d - terminated by %d\n", childPidWait, WEXITSTATUS(childStatus), WTERMSIG(childStatus));
        }
        else if (WIFSIGNALED(childStatus))
        {
            printf("%d stopped by signal %d\n", childPidWait, WTERMSIG(childStatus));
        }
        else if (WCOREDUMP(childStatus))
        {
            printf("%d ha generato CORE DUMP\n", childPidWait);
        }
        else if (WIFSTOPPED(childStatus))
        {
            printf("%d stoppato da un SIGNAL di tipo %d\n", childPidWait, WSTOPSIG(childStatus));
        }
        else if (WIFCONTINUED(childStatus))
        {
            printf("%d e' stato risvegliato da un SIGNAL\n", childPid);
        }
        else
        {
            printf("ERRORE\n");
        }
    }

/* SOLO PER TEST*/
#if (ENABLE_TEST)
    for (i = 0; i < SO_USERS_NUM; i++)
    {
        printf("\nSTAMPO TUTTI GLI ID DEGLI UTENTI NELL'ARRAY CREATE\n");
        printf("PID utente %d = %d\n", i, *(shmArrayUsersPidPtr + i));
    }
#endif

    /*EFFETTUO LE DETACH, ALTRIMENTI LA OPERAZIONE NON AVRA' ALCUN EFFETTO*/
    shmdt(shmIndiceBloccoPtr);
    TEST_ERROR;
    shmdt(shmLibroMastroPtr);
    TEST_ERROR;
    shmdt(shmArrayNodeMsgQueuePtr);
    TEST_ERROR;
    shmdt(shmArrayUsersPidPtr);
    TEST_ERROR;
#if (ENABLE_TEST)
    printf("Id libro mastro - %d\nId Indice blocco - %d\nId Nodo - %d\nId Id utente - %d\n", shmIdMastro, shmIdIndiceBlocco, shmIdNodo, shmIdUtente);
#endif
    /*CHIUSURA SM*/
    shmctl(shmIdMastro, IPC_RMID, 0);
    TEST_ERROR;
    shmctl(shmIdIndiceBlocco, IPC_RMID, 0);
    TEST_ERROR;
    shmctl(shmIdNodo, IPC_RMID, 0);
    TEST_ERROR;
    shmctl(shmIdUtente, IPC_RMID, 0);
    TEST_ERROR;
    printf("QUA\n");

    sleep(3);

    printf("DOPO LA SLEEP\n");
    return 0;
}

int read_all_parameters()
{
    char buffer[256] = {};
    char *token;
    char *delimeter = "= ";
    int parsedValue;
    bzero(buffer, 256);
    FILE *configFile = fopen("parameters.txt", "r+");
    if (configFile != NULL)
    {
        // printf("File aperto!\n");
        while (fgets(buffer, 256, configFile) > 0)
        {
            token = strtok(buffer, delimeter);
            if (strcmp(token, "SO_USERS_NUM") == 0)
            {
                SO_USERS_NUM = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_NODES_NUM") == 0)
            {
                SO_NODES_NUM = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_REWARD") == 0)
            {
                SO_REWARD = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_MIN_TRANS_GEN_NSEC") == 0)
            {
                SO_MIN_TRANS_GEN_NSEC = atol(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_MAX_TRANS_GEN_NSEC") == 0)
            {
                SO_MAX_TRANS_GEN_NSEC = atol(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_RETRY") == 0)
            {
                SO_RETRY = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_TP_SIZE") == 0)
            {
                SO_TP_SIZE = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_BLOCK_SIZE") == 0)
            {
                SO_BLOCK_SIZE = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_MIN_TRANS_PROC_NSEC") == 0)
            {
                SO_MIN_TRANS_PROC_NSEC = atol(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_MAX_TRANS_PROC_NSEC") == 0)
            {
                SO_MAX_TRANS_PROC_NSEC = atol(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_REGISTRY_SIZE") == 0)
            {
                SO_REGISTRY_SIZE = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_BUDGET_INIT") == 0)
            {
                SO_BUDGET_INIT = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_SIM_SEC") == 0)
            {
                SO_SIM_SEC = atoi(strtok(NULL, delimeter));
            }
            else if (strcmp(token, "SO_FRIENDS_NUM") == 0)
            {
                SO_FRIENDS_NUM = atoi(strtok(NULL, delimeter));
            }
            else
            {
                printf("Errore durante il parsing dei parametri - token %s\n", token);
                fclose(configFile);
                return -1;
                /*exit(EXIT_FAILURE);*/
            }

            // while(token != NULL)
            // {
            //     printf("%s", token);
            //     token = strtok(NULL, "=");
            // }
        }
        // printf("\n");
        int closeResponse = fclose(configFile);
        if (closeResponse == -1)
        {
            perror("fclose");
            return -1;
            /*exit(EXIT_FAILURE);*/
        }

/* PER TESTING */
#if (ENABLE_TEST)
        printf("Stampo i parametri parsati!\n");
        printf("SO_USERS_NUM=%d\n", SO_USERS_NUM);
        printf("SO_NODES_NUM=%d\n", SO_NODES_NUM);
        printf("SO_REWARD=%d\n", SO_REWARD);
        printf("SO_MIN_TRANS_GEN_NSEC=%ld\n", SO_MIN_TRANS_GEN_NSEC);
        printf("SO_MAX_TRANS_GEN_NSEC=%ld\n", SO_MAX_TRANS_GEN_NSEC);
        printf("SO_RETRY=%d\n", SO_RETRY);
        printf("SO_TP_SIZE=%d\n", SO_TP_SIZE);
        printf("SO_BLOCK_SIZE=%d\n", SO_BLOCK_SIZE);
        printf("SO_MIN_TRANS_PROC_NSEC=%ld\n", SO_MIN_TRANS_PROC_NSEC);
        printf("SO_MAX_TRANS_PROC_NSEC=%ld\n", SO_MAX_TRANS_PROC_NSEC);
        printf("SO_REGISTRY_SIZE=%d\n", SO_REGISTRY_SIZE);
        printf("SO_BUDGET_INIT=%d\n", SO_BUDGET_INIT);
        printf("SO_SIM_SEC=%d\n", SO_SIM_SEC);
        printf("SO_FRIENDS_NUM=%d\n", SO_FRIENDS_NUM);
#endif

        return 0;
    }
    else
    {
        perror("fopen");
        return -1;
        /*exit(EXIT_FAILURE);*/
    }
}

void alarmHandler(int signum)
{
    printf("L'ALLARM HANDLER!\n");
}

/*
La seguente funzione restituisce il PID dell'UTENTE scelto a caso a cui inviare la transazione.
Alcune assunzioni:
1. La memoria condivisa esiste ed e' POPOLATA
2. max rappresenta il valore di NUM_USERS che al momento della invocazione deve avere un valore valido.
*/
int getRandomUser(int max, int myPid, int *shmArrayUsersPidPtr)
{
    int userPidId;
    srand(myPid);

    do
    {
        userPidId = rand() % (max - 1);
    } while (*(shmArrayUsersPidPtr + userPidId) == myPid && *(shmArrayUsersPidPtr + userPidId) != -1);

    return *(shmArrayUsersPidPtr + userPidId);
}

/*
La seguente funzione attravera la shared memory, verificando se il contenuto della cella coincide con il valore passato come pidToRemove.
Alcune assunzioni:
1. La memoria condivisa esiste ed e' POPOLATA
2. numUsers rappresenta il valore di NUM_USERS che al momento della invocazione deve avere un valore valido.

DA CONCORDRE - assegnare al processo terminato -1 oppure un altro valore.
*/
void updateShmArrayUsersPid(int pidToRemove, int *shmArrayUsersPidPtr, int numUsers)
{
    /*ACCEDO IN MODO ESCLUSIVO ALLA SM - TODO*/
    for (int i = 0; i < numUsers; i++)
    {
        if (*(shmArrayUsersPidPtr + i) == pidToRemove)
        {
            *(shmArrayUsersPidPtr + i) = -1;
            break;
        }
    }
    /*RILASCIO LA RISORSA - TODO*/
}

/*
La seguente funzione restituisce il MQID della coda associata ad ogni NODO scelto a caso a cui inviare la transazione.
Alcune assunzioni:
1. La memoria condivisa esiste ed e' POPOLATA
2. max rappresenta il valore di NUM_NODES che al momento della invocazione deve avere un valore valido.
*/
int getRandomUser(int max, int *shmArrayNodeMsgQueuePtr)
{
    srand(getpid());
    return *(shmArrayNodeMsgQueuePtr + rand() % (max - 1));
}

/*
La seguente funzione restituisce il bilancio dell'utente.
Alcune assunzioni:
1. La memoria condivisa esiste ed e' POPOLATA
2. max rappresenta il valore di SO_BLOCK_SIZE che al momento della invocazione deve avere un valore valido.
3. myPid del processo utente in esecuzione
*/
int getMyBilancio(int myPid, transazione *shmLibroMastroPtr, int *index, int max)
{
    int bilancio = 0;
    for (index; index < max; index++)
    {
        if (((*(shmLibroMastroPtr + *index)).receiver == myPid || (*(shmLibroMastroPtr + *index)).sender == myPid) && (*(shmLibroMastroPtr + *index)).quantita != 0)
        {
            bilancio += (*(shmLibroMastroPtr + *index)).receiver;
        }
    }
    return bilancio;
}

/*
La seguente funzione riempie la transaction pool con i membri fittizi, dummy
*/
void initDummyTransactionPool(transazione_tp *tp, int tpSize)
{
    int j;
    j = 0;
    transazione_tp dummyTransazione;
    dummyTransazione.state = TRANSAZIONE_KO;
    dummyTransazione.transazione.quantita = 0;
    dummyTransazione.transazione.receiver = 0;
    dummyTransazione.transazione.sender = 0;
    dummyTransazione.transazione.reward = 0;
    clock_gettime(CLOCK_BOOTTIME, &dummyTransazione.transazione.timestamp);

    for (j; j < tpSize; j++)
    {
        tp[j] = dummyTransazione;
    }

    return;
}

/*
La segunete funzione determina se la TP e' piena o meno
*/
int isFull(transazione_tp *t, int tpSize)
{
    int i;
    int count;
    count = 0;
    for (i = 0; i < tpSize; i++)
    {
        if (t[i].state == TRANSAZIONE_OK)
        {
            count++;
        }
    }

    return (count == tpSize) ? 1 : 0;
}

/*
    La seguente funzione restituisce la prima cella dove salvare il messaggio
*/
int getNextIndex(transazione_tp *tp, int tpSize)
{
    int i;
    i = 0;
    for (i = 0; i < tpSize; i++)
    {
        if (tp[i].state == TRANSAZIONE_KO)
        {
            return i;
        }
    }
    return -1;
}

/*
Implementazione della attesa non attiva
*/
void attesaNonAttiva(struct timespec *req, struct timespec *rem, long minNsec, long maxNsec)
{
    long nsecInSec;
    long randSleep;

    nsecInSec = 1e9L;
    srand(getpid());
    randSleep = minNsec + rand() % (maxNsec - minNsec);

    req->tv_sec = randSleep / nsecInSec;
    req->tv_nsec = randSleep - (nsecInSec * randSleep);
    nanosleep(req, rem);
}

/*Permette di marcare le transazioni che possono essere sovrascritte*/
void marcaTransazioni(transazione_tp *tp, transazione *b, int tpSize, int bSize)
{
    /*indice per tp*/
    int i1;
    /*indice per blocco b*/
    int i2;

    i1 = i2 = 0;

    while (i2 != tpSize)
    {
        while (i1 != bSize)
        {
            if (tp[i1].transazione.timestamp.tv_sec == b[i2].timestamp.tv_sec && tp[i1].transazione.timestamp.tv_nsec == b[i2].timestamp.tv_nsec && tp[i1].transazione.sender == b[i2].sender && tp[i1].transazione.receiver == b[i2].receiver && tp[i1].state == TRANSAZIONE_KO)
            {
                tp[i1].state = TRANSAZIONE_OK;
                i1 = 0;
                i2++;
            }
            else
            {
                i1++;
            }
        }
    }
}