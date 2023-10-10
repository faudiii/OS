#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int player1_wins = 0;
int player2_wins = 0;
int draw = 0;

int player1_total;
int player2_total;

int K = 0;

void *run_experiment(void *args)
{
    int batch_size = *((int *)args);

    for (int i = 0; i < batch_size; i++)
    {
        int player1_score = 0;
        int player2_score = 0;

        for (int j = 0; j < K; j++)
        {
            player1_score += 2 * (rand() % 6 + 1);
            player2_score += 2 * (rand() % 6 + 1);
        }

        int player1_total_with_score = player1_total + player1_score;
        int player2_total_with_score = player2_total + player2_score;

        if (player1_total_with_score > player2_total_with_score)
        {
            player1_wins++;
        }
        else if (player1_total_with_score < player2_total_with_score)
        {
            player2_wins++;
        }
        else
        {
            draw++;
        }
    }

    pthread_exit(NULL);
}

int main()
{
    int experiments, max_threads;

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

    pthread_t tid[max_threads];
    int batch_sizes[max_threads];
    int remaining_experiments = experiments;
    int batch_index = 0;

    clock_t start_time = clock();

    while (remaining_experiments > 0)
    {
        batch_sizes[batch_index] = remaining_experiments < max_threads ? remaining_experiments : max_threads;
        remaining_experiments -= batch_sizes[batch_index];

        pthread_create(&tid[batch_index], NULL, run_experiment, &batch_sizes[batch_index]);
        batch_index++;

        if (batch_index == max_threads || remaining_experiments == 0)
        {
            for (int j = 0; j < batch_index; j++)
            {
                pthread_join(tid[j], NULL);
            }
            batch_index = 0;
        }
    }

    clock_t end = clock();

    printf("Player 1 wins: %d times\n", player1_wins);
    printf("Player 2 wins: %d times\n", player2_wins);
    printf("Draw: %d times\n", draw);

    printf("elapsed %3.6f ms\n", ((double)(end - start_time) / CLOCKS_PER_SEC) * 1000);

    return 0;
}
