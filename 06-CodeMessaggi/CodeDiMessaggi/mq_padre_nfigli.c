#include <stdio.h>
#include <unistd.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MSG_SIZE (sizeof(int) + sizeof(pid_t))
#define NUM_KIDS 1100
#define MAX_VALUE 30000
#define GO 1
#define STOP 0

struct msgbuf
{
    long mtype; /* message type, must be > 0 */
    int number; /* message data */
    pid_t pid;
};

struct msg_reply
{
    long mtype; /* message type, must be > 0 */
    int rst;    /* message data */
};

int main(int argc, char *argv[])
{
    int mq_id = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    printf("*** Coda di Messaggi - ID[%d] ***\n", mq_id);
    struct msgbuf my_message;
    struct msg_reply reply;
    // int nk = atoi(argv[1]);
    // printf("***********************%d\n", nk);

    for (int i = 0; i < NUM_KIDS; i++)
    {
        switch (fork())
        {
        case 0:
            printf("CHILD PID %5d - hello!\n", getpid());
            srand(getpid());
            struct msgbuf my_message;
            int end = GO;

            while (end)
            {
                int n = rand() % MAX_VALUE + 1;
                printf("CHILD %d - Number %d\n", getpid(), n);
                my_message.mtype = getppid();
                my_message.number = n;
                my_message.pid = getpid();
                msgsnd(mq_id, &my_message, MSG_SIZE, 0);
                printf("CHILD %d - Waiting for master answer...\n", getpid());
                if (msgrcv(mq_id, &reply, sizeof(int), getpid(), 0) == -1)
                {
                    perror("figlio-msgsnd");
                    exit(EXIT_FAILURE);
                }
                printf("CHILD %d - Answer received!\n", getpid());
                end = reply.rst;
                end == GO ? printf("%d continue...", getpid()) : printf("%d STOP received!\n", getpid());
            }
            printf("Figlio[%d] ha terminato!\n", getpid());
            exit(EXIT_SUCCESS);
            break;
        case -1:
            perror("fork_error_-1");
            exit(EXIT_FAILURE);
            break;
        default:
            break;
        }
    }
    int children[NUM_KIDS] = {};
    int children_alive = NUM_KIDS;
    int min_cPid = -1;
    int min_value = MAX_VALUE + 1;
    while (children_alive > 0)
    {
        printf("Children alive %d\n", children_alive);
        /* Ricezione dei numeri generati dai figli */
        for (int i = 0; i < children_alive; i++)
        {
            printf("Padre tenta di ricevere\n");
            if (msgrcv(mq_id, &my_message, MSG_SIZE, getpid(), 0) == -1)
            {
                perror("padre-msgrcv");
                exit(-1);
            }
            else
            {
                int value_received = my_message.number;
                int cPid = my_message.pid;

                if (value_received <= min_value)
                {
                    min_cPid = cPid;
                    min_value = value_received;
                }

                printf("Padre[%d] ha ricevuto valore[%d] da figlio[%d]\n", getpid(), value_received, cPid);
                children[i] = cPid;
            }
        }
        /* Avviso dei figli */
        for (int i = 0; i < children_alive; i++)
        {
            reply.mtype = children[i];
            if (min_cPid == children[i])
            {
                reply.rst = STOP;
            }
            else
            {
                reply.rst = GO;
            }
            int notify_value = reply.rst;
            printf("Padre tenta di inviare a %d\n", children[i]);
            if (msgsnd(mq_id, &reply, sizeof(reply.rst), 0) == -1)
            {
                perror("padre-msgsnd");
                exit(EXIT_FAILURE);
            }
            else
            {
                printf("Figlio[%d] notificato con valore[%d]\n", children[i], notify_value);
            }
        }

        children_alive--;
        min_value = MAX_VALUE + 1;
    }

    int status;
    while (wait(&status) != -1)
    {
        // perror("wait");
    }

    int msgctl_remove_response = msgctl(mq_id, IPC_RMID, NULL);
    if (msgctl_remove_response != -1)
    {
        printf("Coda [%d] e' stata eliminata con successo!\n", mq_id);
    }
    else
    {
        perror("msg_ctl(IPC_RMID)");
    }
    return 0;
}