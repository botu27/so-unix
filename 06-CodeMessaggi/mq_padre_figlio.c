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

// metodo alternativo
typedef struct mq_struct
{
    long mtype;
    char buffer[MQ_BUFFER_MAX_SIZE];
} mq_struct;

int main()
{
    int mq_id = msgget(MQ_ID_DEFINED_BY_PROC_FATHER, IPC_CREAT | 0600);
    CHECK_MQ(mq_id);
    int f_pid;
    mq_struct my_message;
    int msgrcv_response;
    int msgctl_remove;
    int msgsnd_send;
    switch (fork())
    {
    case 0:
        snprintf(my_message.buffer, sizeof(my_message.buffer), "Saluti da FIGLIO[%d]\n", getpid());
        sleep(2);
        msgsnd_send = msgsnd(mq_id, &my_message, sizeof(my_message.buffer), 0);
        if (msgsnd_send == -1)
        {
            PRINT_TRACE("msgsnd");
        }
        else
        {
            printf("FIGLIO[%d] ha inviato correttamente il messaggio in coda id[%d]\n", getpid(), mq_id);
        }
        exit(EXIT_SUCCESS);
        break;
    case -1:
        PRINT_TRACE("fork");
        break;
    default:
        msgrcv_response = msgrcv(mq_id, &my_message, sizeof(my_message.buffer), 0, 0);
        if (msgrcv_response == -1)
        {
            PRINT_TRACE("msgrcv");
        }
        else
        {
            printf("Messaggio ricevuto: %s", my_message.buffer);
            msgctl_remove = msgctl(mq_id, IPC_RMID, NULL);
            if (msgctl_remove == 0)
            {
                printf("Coda eliminata con successo!\n");
            }
            else
            {
                PRINT_TRACE("msgctl (IPC_RMID)");
            }
        }
        break;
    }
    return 0;
}