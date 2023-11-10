#include <stdio.h>
#include <stdlib.h>
#include "translation.h"

extern char *translation(long x);

int main()
{
    long decimalNumber;

    printf("Введите десятичное число: ");
    scanf("%ld", &decimalNumber);

    char *binaryResult = translation(decimalNumber);

    printf("Десятичное число: %ld\n", decimalNumber);
    printf("Двоичное представление: %s\n", binaryResult);

    free(binaryResult);

    return 0;
}
