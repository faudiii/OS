#include <stdio.h>
#include <pthread.h>
#include <time.h>

int player1_wins = 0;
int player2_wins = 0;
int draw = 0;
int player1_total, player2_total;
pthread_mutex_t mutex;

void calculate_wins(int i, int j) {
    if (player1_total + i > player2_total + j) {
        player1_wins++;
    } else if (player1_total + i < player2_total + j) {
        player2_wins++;
    } else {
        draw++;
    }
}

// void* calculate_wins_thread(void* args) {
//     int* thread_args = (int*)args;
//     int i = thread_args[0];
//     int j = thread_args[1];
    
//     calculate_wins(i, j);
    
//     pthread_exit(NULL);
// }

int main() {
    int K;
    int max_threads;

    printf("Enter the number of rounds (K): ");
    scanf("%d", &K);

    printf("Enter player 1's total score: ");
    scanf("%d", &player1_total);

    printf("Enter player 2's total score: ");
    scanf("%d", &player2_total);

    printf("Enter the maximum number of threads: ");
    scanf("%d", &max_threads);

    int min_points = K * 2;
    int max_points = K * 12;

    pthread_mutex_init(&mutex, NULL);

    pthread_t tid[max_threads];
    
    int thread_args[max_threads][2];

    int count = 0;

    clock_t start_time = clock(); 


    for (int i = min_points; i <= max_points; i++) {
        for (int j = min_points; j <= max_points; j++) {
            thread_args[count][0] = i;
            thread_args[count][1] = j;
            pthread_create(&tid[count], NULL, calculate_wins, &thread_args[count]);
            count++;

            if (count == max_threads) {
                for (int k = 0; k < max_threads; k++) {
                    pthread_join(tid[k], NULL);
                }
                count = 0; // Сбрасываем счетчик потоков
            }
        }
    }

    clock_t end_time = clock(); 
    double execution_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    // Ожидание оставшихся потоков
    for (int k = 0; k < count; k++) {
        pthread_join(tid[k], NULL);
    }

    pthread_mutex_destroy(&mutex);

    printf("Player 1 wins: %d times\n", player1_wins);
    printf("Player 2 wins: %d times\n", player2_wins);
    printf("Draw: %d times\n", draw);
    printf("Execution time: %.6f seconds\n", execution_time);

    return 0;
}
