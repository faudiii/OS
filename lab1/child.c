#include <stdio.h>
#include <stdlib.h>
#include "unistd.h" // fork() and getpid() declarations are here

int main(int argc, char *argv[]) {
   
    printf("in child.c '%d'\n", getpid()); 
    float sum = 0;
    float num;

    while (scanf("%f", &num) == 1) {
        sum += num;
    }

    printf("Сумма чисел из файла: %.2f\n", sum);

    return 0;
}
