#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "translation.h"

char *translation(long x)
{
    // Вычисляем максимальную длину двоичной строки
    int maxBinaryLength = 0;
    long temp = x;

    while (temp > 0)
    {
        maxBinaryLength++;
        temp /= 2;
    }

    // Выделяем память под строку для двоичного представления числа
    char *binaryString = (char *)malloc(maxBinaryLength + 1);

    if (binaryString == NULL)
    {
        fprintf(stderr, "Ошибка выделения памяти.\n");
        exit(EXIT_FAILURE);
    }

    // Заполняем строку нулями
    memset(binaryString, '0', maxBinaryLength);
    binaryString[maxBinaryLength] = '\0';

    // Индекс для заполнения строки в обратном порядке
    int index = maxBinaryLength - 1;

    // Переводим число в двоичную систему
    while (x > 0)
    {
        // Записываем остаток от деления (0 или 1)
        binaryString[index] = (x % 2) + '0';

        // Делаем целочисленное деление на 2
        x /= 2;

        // Уменьшаем индекс
        index--;
    }

    return binaryString;
}
