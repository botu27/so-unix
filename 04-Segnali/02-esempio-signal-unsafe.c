#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

static void dd_signal_handler(int sig) {
    static int count = 0;
    // NB: questa implementazione è UNSAFE: l'handler utilizza
    // funzioni non-async-signal-safe, come (printf(), exit()  
    if (sig == SIGINT) {
        count++;
        printf("intercettato SIGINT (%d)\n", count);
        return; // l'esecuzione riprenderà dall'istruzione successiva
        // a quella in esecuzione al momento dell'interruzione
    }
    // PRE: se non è SIGINT è SIGQUIT -
    //      stampo un msg prima di terminare
    printf("intercettato SIGQUIT - termino!!!\n");
    exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
    // associo lo stesso handler ai due segnali SIGINT e SIGQUIT
    if (signal(SIGINT, dd_signal_handler) == SIG_ERR)
    printf("ERROR SIG_ERR");
    if (signal(SIGQUIT, dd_signal_handler) == SIG_ERR)
    printf("ERROR SIG_ERR");
    for (;;) // ciclo infinito di attesa dei segnali
    pause(); // il processo bloccato finché non riceve un segnale
}
