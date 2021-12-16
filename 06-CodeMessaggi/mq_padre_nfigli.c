#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <wait.h>
#include "mq_utils.h"
#define NUM_KIDS 12
#define GO 1
#define STOP 0

// metodo alternativo
typedef struct mq_struct
{
    long mtype;
    int r_num;
} mq_struct;

// int end = 0;

// void signalHandler(int sig)
// {
//     switch (sig)
//     {
//     case SIGINT:
//         printf("CHILD %d - Received: SIGINT\n", getpid());
//         end = 1;
//         break;
//     case SIGUSR1:
//         printf("CHILD %d - Received: SIGUSR1\n", getpid());
//         break;
//     case SIGUSR2:
//         printf("CHILD %d - Received: SIGUSR2\n", getpid());
//         break;
//     }
//     printf("CHILD %d - ok\n", getpid());
// }

int main()
{
    int mq_id = msgget(IPC_PRIVATE, IPC_CREAT | 0600);
    CHECK_MQ(mq_id);
    printf("***MQ[%d]***\n", mq_id);
    int children_alive = NUM_KIDS;
    int kinder_garden[NUM_KIDS] = {};
    int c_pid;
    int min_c_pid = 0;
    int min_r_num = 21;
    mq_struct message;
    struct sigaction sa_new;
    sigset_t my_sa_mask;
    sigset_t temp_sa_mask;
    int msgrcv_response;
    int msgctl_remove;
    int msgsnd_send;
    int r_num;
    int cont;
    int f_result;

    for (int j = 0; j < NUM_KIDS; j++)
    {
        f_result = fork();
        switch (f_result)
        {
        case 0:
            // printf("Figlio[%d]\n", getpid());
            srand(getpid());
            message.mtype = getpid();
            cont = GO;

            while (cont)
            {
                r_num = rand() % NUM_KIDS + 1;
                message.r_num = r_num;

                msgsnd_send = msgsnd(mq_id, &message, sizeof(message.r_num), 0);
                if (msgsnd_send == -1)
                {
                    PRINT_THEN_EXIT("msgsnd", EXIT_FAILURE);
                }

                // printf("Attendo risposta...\n");
                msgrcv_response = msgrcv(mq_id, &message, sizeof(message.r_num), getpid(), 0);
                if (msgrcv_response != -1)
                {
                    if (message.r_num == 0)
                    {
                        cont = STOP;
                    }
                    else
                    {
                        cont = GO;
                    }
                }
                else
                {
                    PRINT_TRACE("msgrcv");
                }
            }

            // sigemptyset(&my_sa_mask);        // creo un empty set, ovvero nessun seganle e' mascherato
            // sigemptyset(&temp_sa_mask);      // idem
            // sigaddset(&my_sa_mask, SIGINT);  // aggiungo al set un segnale da mascherare
            // sigaddset(&my_sa_mask, SIGUSR1); // idem
            // sa_new.sa_mask = temp_sa_mask;
            // sa_new.sa_handler = signalHandler;
            // sa_new.sa_flags = 0; // no flags
            // sigaction(SIGINT, &sa_new, NULL); // imposto signal handler a sigint
            // sigaction(SIGUSR1, &sa_new, NULL); // questa volta a sigusr1
            // sigprocmask(SIG_SETMASK, &my_sa_mask, NULL); // imposto il set di mashere al mio sigaction

            // sigsuspend(&temp_sa_mask);
            // printf("FIGLIO[%d] ha terminato!\n", getpid());
            exit(EXIT_SUCCESS);
            break;
        case -1:
            PRINT_THEN_EXIT("fork", EXIT_FAILURE);
            break;
        default:
            break;
        }
    }

    printf("PARENT PID %5d\n", getpid());

    while (children_alive > 0)
    {
        for (int i = 0; i < children_alive; i++)
        {
            int value;
            pid_t cPid;
            msgrcv_response = msgrcv(mq_id, &message, sizeof(message.r_num), 0, 0);
            if (msgrcv_response != -1)
            {
                value = message.r_num;
                cPid = message.mtype;
                printf("PARENT - (%d) From %d - Value %d\n", i, cPid, value);

                if (value <= min_r_num)
                {
                    min_c_pid = cPid;
                    min_r_num = value;
                }
                kinder_garden[i] = cPid;
            }
            else
            {
                PRINT_TRACE("msgrcv");
            }
        }

        printf("PARENT - Min value %d from %d\n",min_r_num, min_c_pid);

        for (int k = 0; k < children_alive; k++)
        {
            message.r_num = (kinder_garden[k] == min_c_pid) ? 0 : 1;
            message.mtype = kinder_garden[k];
            msgsnd_send = msgsnd(mq_id, &message, sizeof(message.r_num), 0);
            if (msgsnd_send != -1)
            {
                printf("Figlio[%d] notificato correttamente con flag[%d]!\n", kinder_garden[k], message.r_num);
            }
            else
            {
                PRINT_TRACE("msgsnd");
            }
        }
        children_alive--;
        min_r_num = NUM_KIDS + 1;
    }

    int status;
    while (wait(&status) != -1)
    {

    } // aspetto la terminazione di tutti i figli
    if (msgctl(mq_id, IPC_RMID, NULL) == -1)
    {
        PRINT_THEN_EXIT("msgctl (IPC_RMID)", -1);
    }

    printf("PARENT: exit\n");
    exit(EXIT_SUCCESS);
    // sleep(3);
}