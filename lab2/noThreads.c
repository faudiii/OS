#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int player1_wins = 0;
int player2_wins = 0;
int draw = 0;
int player1_total;
int player2_total;

void run_experiment(int K)
{
    int player1_score = 0;
    int player2_score = 0;

    // Бросаем 2 кости K раз для каждого игрока
    for (int i = 0; i < K; i++)
    {
        int roll1 = rand() % 6 + 1; // Случайное число от 1 до 6
        int roll2 = rand() % 6 + 1;
        player1_score += roll1 + roll2;

        int roll3 = rand() % 6 + 1;
        int roll4 = rand() % 6 + 1;
        player2_score += roll3 + roll4;
    }

    // Сравниваем суммарные результаты
    player1_score += player1_total; // Добавляем начальный счет
    player2_score += player2_total; // Добавляем начальный счет

    if (player1_score > player2_score)
    {
        player1_wins++;
    }
    else if (player1_score < player2_score)
    {
        player2_wins++;
    }
    else
    {
        draw++;
    }
}

int main()
{
    int K, experiments;

    printf("Enter the number of rounds (K): ");
    scanf("%d", &K);

    printf("Enter player 1's total score: ");
    scanf("%d", &player1_total);

    printf("Enter player 2's total score: ");
    scanf("%d", &player2_total);

    printf("Enter the number of experiments: ");
    scanf("%d", &experiments);

    clock_t start_time = clock();

    for (int i = 0; i < experiments; i++)
    {
        run_experiment(K);
    }

    clock_t end_time = clock();
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Player 1 wins: %d times\n", player1_wins);
    printf("Player 2 wins: %d times\n", player2_wins);
    printf("Draw: %d times\n", draw);
    printf("Execution time: %.6f seconds\n", execution_time);

    return 0;
}
