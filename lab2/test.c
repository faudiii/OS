#include <stdio.h>
#include <pthread.h>
#include <time.h>

int sum = 0;

void *increment_function_plus_200(void *args)
{
    for (int i = 0; i < 100; i++)
    {
        sum = sum + 2;
    }
    pthread_exit(0);
}

int main()
{
    clock_t start_time = clock();

    const int threadCount = 1;
    pthread_t tid[threadCount];
    for (int i = 0; i < threadCount; i++)
    {
        pthread_create(&tid[i], NULL, increment_function_plus_200, NULL);
    }
    for (int i = 0; i < threadCount; i++)
    {
        pthread_join(tid[i], NULL);
    }
    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    printf("sum = %d\n", sum);
    printf("Execution time: %.6f seconds\n", execution_time);
    return 0;
}