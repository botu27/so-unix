#include <stdio.h>
#include <stdlib.h>

int main()
{
    short *arr = calloc(10, sizeof(short));
    int q = 1;
    int *ptr = &q;

    for (int i = 0; i < 10; i++)
    {
        arr[i] = 1;
    }
    for (int i = 0; i < 10; i++)
    {
        printf("Valore arr[%i] : %d\n", i, arr[i]);
    }

    printf("Arr.length = %ld\n", sizeof(ptr));

    return 0;
}