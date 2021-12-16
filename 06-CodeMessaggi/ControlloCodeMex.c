#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "mq_utils.h"

struct mq_struct
{
    long mtype;
    char buffer[100];
};

int main()
{
    // creo una nuova coda di messaggi
    int mq_id = msgget(MQ_ID, IPC_CREAT | 0600);
    if (mq_id >= 0)
    {
        char *m_buffer = "Ciao, sono PROCESSO[%d] e questo e' il mio primo messaggio\n";
        struct mq_struct m;
        m.mtype = 1;
        snprintf(m.buffer, sizeof(m.buffer), m_buffer, getpid());
        int msgsnd_response = msgsnd(mq_id, &m, sizeof(m.buffer), IPC_NOWAIT);
        printf("msgsnd responded with : %d\n", msgsnd_response);
        if (msgsnd_response)
        {
            PRINT_THEN_EXIT("msgsnd", -1)
        }
        else
        {
            printf("Message send, returned value: %d\n", msgsnd_response);
        }
        // definisco la struttura dove salvare le informazioni sulla coda appena creata
        struct msqid_ds mq_description;
        int msgctl_response = msgctl(mq_id, IPC_STAT, &mq_description);
        if (msgctl_response >= 0)
        {
            printf("Ho ricevuto le seguenti informazioni, mio PID[%d]:\n", getpid());
            printf("Time of last msgsnd : %ld\n", mq_description.msg_ctime);
            printf("Time of last msgrcv : %ld\n", mq_description.msg_rtime);
            printf("Time of last change : %ld\n", mq_description.msg_ctime);
            printf("Current bytes written on queue : %ld\n", mq_description.__msg_cbytes);
            printf("Current messages in queue : %ld\n", mq_description.msg_qnum);
            printf("Las send PID : %d\n", mq_description.msg_lspid);
        }
        else
        {
            PRINT_THEN_EXIT("msgctl", -1)
        }
        // elimino la coda
        /*if (!msgctl(mq_id, IPC_RMID, NULL))
        {
            PRINT_THEN_EXIT("msgctl with IPC_RMID", -1)
        }
        else
        {
            printf("Coda eliminata con successo!\n");
        }*/
    }
    else
    {
        PRINT_THEN_EXIT("msgget", -1)
    }
    return 0;
}