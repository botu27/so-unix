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

int main() {
    // recupore l'id della coda
    int mq_id = msgget(MQ_ID, IPC_CREAT | 0600);
    CHECK_MQ(mq_id);
    int what_mtype = mq_id;
    int msgrcv_response;
    struct mq_struct my_message;
    while(what_mtype >= 0) {
        printf("Che mtype vuole prelevare dalla coda?\n");
        scanf("%d", &what_mtype);
        if(what_mtype < 0) {
            break;
        }
        msgrcv_response = msgrcv(mq_id, &my_message, sizeof(my_message.buffer), what_mtype, IPC_NOWAIT);
        if(msgrcv_response) {
            perror("msgrcv");
        }
        printf("Letti %d bytes\n", msgrcv_response);
        printf("Ricevuto: %s\n\n", my_message.buffer);
    }
    printf("Fine ricezione dati!\n");
    printf("Stato della coda ID[%d]\n", mq_id);
    struct msqid_ds mq_info;
    if (msgctl(mq_id, IPC_STAT, &mq_info) < 0)
    {
        PRINT_TRACE("msgctl (IPC_STAT)")
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