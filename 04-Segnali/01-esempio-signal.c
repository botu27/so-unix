#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

#define TIME 7

int time=TIME;

void alarmHandler(int sig) {
  printf("Allarme ricevuto e trattato\n");
  alarm(TIME);
  time=TIME;
}

int main() {
  if (signal(SIGALRM, alarmHandler)==SIG_ERR) {
    printf("\nErrore della disposizione dell'handler\n");
    exit(EXIT_FAILURE);
  }

  alarm(TIME);

  printf("Quindi\n");

  while(1) {
    printf("tic\n");
    printf("mancano %d secondi\n",alarm(time--));
    sleep(1);
  };

  printf("FINE\n");

}
