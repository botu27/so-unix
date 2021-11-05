#include <stdio.h>
#include "somma.h"
#include "moltiplicazione.h"

int main() {
    int x = 2;
    int y = 3;

    printf("Somma: %d\nMoltiplicazione: %d\n", somma(x, y), moltiplica(x, y));

    return 0;
}