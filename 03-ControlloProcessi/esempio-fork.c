#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    // Creazione di proc. e' uno strumento molto versatile per suddividere un compito
    // dopo l'esec. della fork esistono 2 proc. e in ciascuno l'esecuzione riprende dal punto in cui la fork() restituisce il PID del processo filgio
    // dopo fork(), ogni processo puo' modificare le variabili in tali segmenti senza influenzare l'altro processo
    int c = 1;
    pid_t pid = fork();
    // nell'amnbiente del padre, fork restituisce il PID del figlio
    // figlio invece riceve 0
    switch(pid) {
        case 0:
            c *= 10;
            printf("Ho cambiato localmente il valore della variabile c : %d\n", c);
            printf("Sono filgio, il mio PID : %d\n", getpid());
            printf("Sono filgio, il PID del mio padre : %d\n", getppid());
            break;
        case -1:
            printf("Errore!");
            exit(EXIT_FAILURE);
        default:
            sleep(3);
            printf("non ho cambiato localmente il valore della variabile c : %d\n", c);
            printf("Sono padre, il mio PID : %d\n", getpid());
            break;
    }

    // importante: il figlio riceve tutti i duplicati di tutti i descrittori di file del padre
    return 0;
}