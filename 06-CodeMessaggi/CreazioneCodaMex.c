#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include "mq_utils.h"

// msgget(int key, int flags...) - system call che ritorna la coda dei messaggi associuata all'id key oppure crearne una nuova avente l'id passato
// se la key ha come valore IPC_PRIVATE allora viene generato un id e creata una nuova coda
// oppure miente IPC_PRIVATE ma nei flag e' presente IPC_CREAT e la key passata non e' presente nel SO, allora la coda viene creata
// flags - contengono le informazioni riguardanti i permessi(bit meno significativo)
// se viene specificato IPC_CREAT(per creare la coda) + IPC_EXCL, allora nel caso la coda con key passato esista, il metodo ritorna errore ed errno assume valore d'errore
// se la coda viene creata, ad essa viene associata una struct msqid ds
// se la cosa esiste, allora con il metodo si puo' verificare i permessi di essa e verificare se e' possibile distruggerla
// valore di ritorno - interop non negativo che specifica l'id della coda
// altrimenti -1 ed errno e' settato in modo opportuno

// msgctl(id, flag, msqid*) - effettua le operazioni di controlle specificati dai flag sulla coda passata come argomento
// valore di ritorno
// 0 su alcuni flag che effetutano la cancellazione della coda oopure ottenimento delle informazioni
// -1 se si verifica un errore
// >0 altrimenti

struct mymsg {
    long mtype;
    char buf[100];
};

int main() {
    // int id = msgget(MSG_QUEUE, IPC_CREAT | IPC_EXCL | 0600); // cosi' creo la coda se non esiste
    // int id2 = msgget(MSG_QUEUE, IPC_EXCL | 0600); // cosi'  ottengo la coda dato l'id
    // int id3 = msgget(IPC_PRIVATE, 0600); // cosi' creo la coda, utilizzo macro IPC_PRIVATE
    int id4 = msgget(ftok("/home/borist/unito/SO/labUnix/06-CodeMessaggi/CreazioneCodaMex.c", 1), IPC_CREAT | 0600); // tecnica di creazione della coda molto avanzata, ftok utilizza un algoritmo di conversione da path name + inter => key unica
    int ret = -1;
    
    ret = msgctl(5, IPC_RMID, NULL);
    struct mymsg mex;
    mex.mtype = 1;
    snprintf(mex.buf, sizeof(mex.buf), "Questo e' primo messaggio inviato da pid[%d]", getpid());

    if(id4) {
        printf("%d ha creato/ottenuto la coda con l'id [%d]\n", getpid(), id4);    
        // invio mex
        if(msgsnd(id4, &mex, sizeof(mex.buf), IPC_NOWAIT) >= 0) {
            printf("Messaggio inviato\n");
        }
        else {
            perror("msgsnd");
        }
        // ricevo mex
        if(msgrcv(id4, &mex, sizeof(mex.buf), 0, IPC_NOWAIT) >= 0) {
            printf("Messaggio ricevuto: %s\n", mex.buf);
        }
        else {
            perror("msgsnd");
        }
        ret = msgctl(id4, IPC_RMID, NULL);
        if(ret == 0) {
            printf("La coda e' stata eliminata con successo!\n");
        }
    }
    else {
        perror("msgget");
    }
    return 0;
}