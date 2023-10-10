#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int player1_wins = 0;
int player2_wins = 0;
int draw = 0;
int player1_total;
int player2_total;

pthread_mutex_t mutex;

struct ThreadArgs
{
    int K;
    int player1_total;
    int player2_total;
};

void *run_experiment(void *args)
{
    struct ThreadArgs *threadArgs = (struct ThreadArgs *)args;
    int K = threadArgs->K;
    int player1_score = 0;
    int player2_score = 0;

    for (int i = 0; i < K; i++)
    {
        player1_score += (rand() % 6 + 1) + (rand() % 6 + 1);
        player2_score += (rand() % 6 + 1) + (rand() % 6 + 1);
    }

    int player1_total_with_score = threadArgs->player1_total + player1_score;
    int player2_total_with_score = threadArgs->player2_total + player2_score;

    if (player1_total_with_score > player2_total_with_score)
    {
        pthread_mutex_lock(&mutex);
        player1_wins++;
        pthread_mutex_unlock(&mutex);
    }
    else if (player1_total_with_score < player2_total_with_score)
    {
        pthread_mutex_lock(&mutex);
        player2_wins++;
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        pthread_mutex_lock(&mutex);
        draw++;
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(NULL);
}

int main()
{
    int K, experiments, max_threads;

    printf("Enter the number of rounds (K): ");
    scanf("%d", &K);

    printf("Enter player 1's total score: ");
    scanf("%d", &player1_total);

    printf("Enter player 2's total score: ");
    scanf("%d", &player2_total);

    printf("Enter the number of experiments: ");
    scanf("%d", &experiments);

    printf("Enter the maximum number of threads: ");
    scanf("%d", &max_threads);

    pthread_mutex_init(&mutex, NULL);

    pthread_t tid[max_threads];
    struct ThreadArgs threadArgs;

    threadArgs.K = K;
    threadArgs.player1_total = player1_total;
    threadArgs.player2_total = player2_total;

    clock_t start_time = clock();

    for (int i = 0; i < experiments; i++)
    {
        pthread_create(&tid[i % max_threads], NULL, run_experiment, &threadArgs);

        if ((i + 1) % max_threads == 0)
        {
            for (int j = 0; j < max_threads; j++)
            {
                pthread_join(tid[j], NULL);
            }
        }
    }

    for (int i = 0; i < experiments % max_threads; i++)
    {
        pthread_join(tid[i], NULL);
    }

    pthread_mutex_destroy(&mutex);

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Player 1 wins: %d times\n", player1_wins);
    printf("Player 2 wins: %d times\n", player2_wins);
    printf("Draw: %d times\n", draw);
    printf("Execution time: %.6f seconds\n", execution_time);

    return 0;
}
