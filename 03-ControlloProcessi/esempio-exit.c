#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void cleanup() {
    printf("Cleanup\n");
}

void bye() {
    printf("Bye\n");
}

int main() {
    atexit(bye);
    atexit(cleanup);
    exit(EXIT_SUCCESS);
    return 0;
}