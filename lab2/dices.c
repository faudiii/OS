#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

int player1_wins = 0;
int player2_wins = 0;
int draw = 0;

int player1_total;
int player2_total;

int K;
int rounds;
int curr_round;

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
    scanf("%d", &rounds);

    printf("Enter the current  round : ");
    scanf("%d", &curr_round);

    K = rounds - curr_round + 1;

    printf("Enter player 1's total score: ");
    scanf("%d", &player1_total);

    printf("Enter player 2's total score: ");
    scanf("%d", &player2_total);

    if (player1_total > (12 * (curr_round - 1)) || player1_total < (2 * (curr_round - 1)) ||
        player2_total > (12 * (curr_round - 1)) || player2_total < (2 * (curr_round - 1)))
    {
        fprintf(stderr, "Ошибка: недопустимые входные данные \n");
        exit(1);
    }

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

    int total_games = player1_wins + player2_wins + draw;
    float player1_win_probability = (float)player1_wins / total_games * 100.0;
    float player2_win_probability = (float)player2_wins / total_games * 100.0;
    float draw_probability = (float)draw / total_games * 100.0;

    printf("Player 1 wins: %d times (%.2f%% probability)\n", player1_wins, player1_win_probability);
    printf("Player 2 wins: %d times (%.2f%% probability)\n", player2_wins, player2_win_probability);
    printf("Draw: %d times (%.2f%% probability)\n", draw, draw_probability);

    printf("elapsed %3.6f ms\n", ((double)(end - start_time) / CLOCKS_PER_SEC) * 1000);

    return 0;
}
