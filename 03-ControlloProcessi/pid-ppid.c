#include <stdio.h>
#include <unistd.h>

int main() {
    // ogni processo ha un ID associato PID(un intero >0) che identifica univoc. il proc. nel sistema
    // da utilizzare la seguente system call
    printf("PID di questo processo : %d", getpid());
    // ogni proc. ha inoltre un genitore(il proc. che lo ha creato)
    // le relazioni tra processi costituiscono una struttura ad albero, radice del quale e' il proc. init
    // Attenzione
    // printf("Parent PID di questo processo : %d", getppid()); // Nei sistemi WINDOWS, a diferenza di sistemi UNIX, la relazione padre - figlio non e' definita
    return 0;
}