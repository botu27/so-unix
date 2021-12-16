#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "mq_utils.h"

struct mq_struct
{
    long mtype;
    char buffer[MQ_BUFFER_MAX_SIZE];
};

int main()
{
    // inizializzo i parametri del messaggio
    int m_mtype;
    char m_mbuffer[MQ_BUFFER_MAX_SIZE] = "";
    // creo la coda di messaggi
    int mq_id = msgget(MQ_ID, IPC_CREAT | 0600);
    CHECK_MQ(mq_id)
    printf("***CODA CON IDENTIFICATORE %d ***\n", mq_id);
    // definisco la struttura del messaggio
    struct mq_struct my_message;
    int msgsnd_response;
    printf("Entro nel ciclo di richiesta dei messaggi(per finire, digiti 0 per m_mtype)\n");
    while (1)
    {
        // prelievo da stdin i parametri
        printf("Inserire un intero >0:\n");
        // int scaned = scanf("%i", &m_mtype); // behaviour not stable
        // printf("scaned %d\n", scaned);
        fgets(m_mbuffer, MQ_BUFFER_MAX_SIZE, stdin);
        m_mtype = atoi(m_mbuffer);
        if (!m_mtype)
        {
            break;
        }
        printf("Inserire il corpo del messaggio:\n");
        fgets(m_mbuffer, MQ_BUFFER_MAX_SIZE, stdin);
        // m_mbuffer[strlen(m_mbuffer) - 1] = '\0';
        // printf("Corpo: %s", m_mbuffer);
        // construisco e invio il messaggio
        my_message.mtype = m_mtype;
        strcpy(my_message.buffer, m_mbuffer);
        msgsnd_response = msgsnd(mq_id, &my_message, MQ_BUFFER_MAX_SIZE, 0);
        if (msgsnd_response < 0)
        {
            fprintf(stderr, "%s: %d. Errore in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
        }
        else
        {
            printf("\n***Messaggio %d inviato con successo!***\n", m_mtype);
        }
    }
    printf("Fine ricezione dati!\n");
    printf("Stato della coda ID[%d]\n", mq_id);
    struct msqid_ds mq_info;
    if (msgctl(mq_id, IPC_STAT, &mq_info) < 0)
    {
        PRINT_TRACE("msgctl (IPC_STAT)");
    }
    else
    {
        printf("Stato di msg_qbytes: %ld\n", mq_info.msg_qbytes);
        printf("Stato di msg_qnum: %ld\n", mq_info.msg_qnum);
        printf("Stato di msg_qbytes: %ld\n", mq_info.msg_qbytes);
        printf("Stato di msg_stime: %s\n", ctime(&mq_info.msg_stime));
    }
    return 0;
}