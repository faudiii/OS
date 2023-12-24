#include <stdbool.h>
#include <stdint.h>
#include <time.h>

typedef struct {
    bool is_timer_started;
    struct timespec start;
    struct timespec finish;
} Timer;

void start_timer(Timer *timer) {
    timer->is_timer_started = true;
    clock_gettime(CLOCK_MONOTONIC, &timer->start);
}

void stop_timer(Timer *timer) {
    if (timer->is_timer_started) {
        timer->is_timer_started = false;
        clock_gettime(CLOCK_MONOTONIC, &timer->finish);
    }
}

int get_time(Timer *timer) {
    if (timer->is_timer_started) {
        clock_gettime(CLOCK_MONOTONIC, &timer->finish);
    }

    uint64_t start_ms = timer->start.tv_sec * 1000 + timer->start.tv_nsec / 1000000;
    uint64_t finish_ms = timer->finish.tv_sec * 1000 + timer->finish.tv_nsec / 1000000;

    return (int)(finish_ms - start_ms);
}
