#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

void handler(int sgnl) {
    signal(SIGTERM, SIG_DFL);
    printf("\nRICEVUTO SIGTERM\n");
}

int main() {
    int pid = getpid();
    printf("%d", getpid());
    signal(SIGTERM, handler);
    printf("Prima del LOOP");

    for(pid = 1000;pid >= 0;pid--) {
        printf("Prima del LOOP");
        fflush(stdout);
        sleep(1);
    }

    printf("Dopo il LOOP");
    fflush(stdout);
    return 0;
}