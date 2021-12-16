#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define NUM_MAX 30000
#define NUM_KIDS 3

#define TEST_ERROR    if (errno) {fprintf(stderr, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno));}

#define MSG_SIZE (sizeof(int)+sizeof(pid_t))

#define GO 1
#define STOP 0

struct msgbuf {
	long mtype;             /* message type, must be > 0 */
	int number;    /* message data */
	pid_t pid;
};

struct msg_reply {
	long mtype;             /* message type, must be > 0 */
	int rst;    /* message data */
};

int main() {
	int q_id;
	pid_t fork_result;
	        	int end;
        	int iter;
        	int num_bytes;
        	int status;

	struct msgbuf my_message;
	struct msg_reply reply;

	q_id = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0600);
	TEST_ERROR;


    for (int i=0; i<NUM_KIDS; i++) {
      fork_result = fork();
      switch (fork_result) {
        case -1:
          fprintf(stderr, "%s: %d. Error in fork #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
          exit(EXIT_FAILURE);
        case 0:
          printf("CHILD PID %5d - hello!\n", getpid());

          end=0;
          iter=0;

          srand(getpid());

          struct msgbuf my_message;

          pid_t pid=getpid();

          while(!end){
            int n=rand()%NUM_MAX+1;
            printf("CHILD %d - Number %d\n", getpid(), n);

            my_message.mtype=getppid();
            my_message.number=n;
            my_message.pid=getpid();

            msgsnd(q_id, &my_message, MSG_SIZE, 0);

            printf("CHILD %d - Waiting for master answer...\n", getpid());
            if ((num_bytes = msgrcv(q_id, &reply, sizeof(int), getpid(), 0)) == -1) {
            	printf("CHILD %d - Message receive error: EXIT!\n", getpid());
							exit(EXIT_FAILURE);
						} 

						printf("CHILD %d - Answer received!\n", getpid());

						if (reply.rst==STOP)
							end=1;
						else printf("CHILD %d - Continue...\n", getpid());
			      
			      iter++;
        	}

          printf("CHILD %d - exit\n",getpid());
          exit(EXIT_SUCCESS);
          break;
        
        default:
          break;
          
      }
  	}
   
    printf("PARENT PID %5d\n", getpid());
    

    int remainingChildred=NUM_KIDS;

    int i=0;

    //sleep(5);

    while (remainingChildred>0) {
    	printf("PARENT - Remaining children: %d\n", remainingChildred);

      int minValue=INT_MAX;
      pid_t minPid=-1;

      pid_t remainingPid[remainingChildred];

      for (int i=0;i<remainingChildred;i++) {
        int value;
        pid_t cPid;


        printf("PARENT - Waiting for CHILDREN number (%d)\n", i);
        if ((num_bytes = msgrcv(q_id, &my_message, MSG_SIZE, getpid(), 0)) == -1) {
        	printf("PARENT %d - Message receive error: EXIT!\n", getpid());
					exit(EXIT_FAILURE);
				} 

				cPid=my_message.pid;
				value=my_message.number;

        printf("PARENT - (%d) From %d - Value %d\n",i,cPid,value);

        remainingPid[i]=cPid;

        if (value<minValue) {
          minValue=value;
          minPid=cPid;
        }
      }

      printf("PARENT - Min value %d from %d\n",minValue,minPid);

      for (int i=0;i<remainingChildred;i++) {
      	reply.mtype=remainingPid[i];
        if (remainingPid[i]==minPid) {
        	reply.rst=STOP;
          printf("PARENT - Sent %d to %d\n",reply.rst,remainingPid[i]);
        }
        else {
          reply.rst=GO;
          printf("PARENT - Sent %d to %d\n",reply.rst,remainingPid[i]);
        }
        if (msgsnd(q_id, &reply, sizeof(int), 0)==-1) {
        		fprintf(stderr, "%s: %d. Error in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
          		exit(EXIT_FAILURE);
        	}
      }

      remainingChildred--;
      //sleep(1);
    }

    while(wait(&status)!=-1);

    if (msgctl(q_id, IPC_RMID, NULL)==-1) {
        		fprintf(stderr, "%s: %d. Error in msgsnd #%03d: %s\n", __FILE__, __LINE__, errno, strerror(errno));
          		exit(EXIT_FAILURE);
        	}

    printf("PARENT: exit\n");
    exit(EXIT_SUCCESS);
}
