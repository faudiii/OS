#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
    printf("in child.c '%d'\n", getpid());

    int shm_fd = shm_open("/my_shared_memory", O_RDWR, 0666);
    printf("shm_fd: %d\n", shm_fd);

    if (shm_fd == -1)
    {
        perror("Ошибка при вызове shm_open1");
        return -1;
    }

    float *shared_sum = (float *)mmap(0, sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    printf("Shared Sum Value2: %f\n", *shared_sum);

    if (shared_sum == MAP_FAILED)
    {
        perror("Ошибка отображения разделяемой памяти");
        close(shm_fd);
        return -1;
    }

    float sum = 0.0f;
    float num;

    while (scanf("%f", &num) == 1)
    {
        sum += num;
    }

    *shared_sum = sum;
    printf("Sum calculated by child: %.2f\n", sum);

    msync(shared_sum, sizeof(float), MS_SYNC);
    shm_unlink("/my_shared_memory");

    munmap(shared_sum, sizeof(float));
    close(shm_fd);

    return 0;
}
