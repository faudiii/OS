#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "translation.h"

char *translation(long x)
{
    int maxBinaryLength = 0;
    long temp = x;

    while (temp > 0)
    {
        maxBinaryLength++;
        temp /= 2;
    }

    char *binaryString = (char *)malloc(maxBinaryLength + 1);

    if (binaryString == NULL)
    {
        fprintf(stderr, "Ошибка выделения памяти.\n");
        exit(EXIT_FAILURE);
    }

    memset(binaryString, '0', maxBinaryLength);
    binaryString[maxBinaryLength] = '\0';

    int index = maxBinaryLength - 1;

    while (x > 0)
    {
        binaryString[index] = (x % 2) + '0';

        x /= 2;

        index--;
    }

    return binaryString;
}
