#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>

int main()
{
    printf("start pid '%d'\n", getpid());

    char *filename = NULL;
    size_t len = 0;

    printf("Введите имя файла: ");

    if (getline(&filename, &len, stdin) == -1)
    {
        perror("filename error");
        return -1;
    }

    filename[strcspn(filename, "\n")] = '\0';

    int file_fd = open(filename, O_RDONLY);
    if (file_fd == -1)
    {
        perror("Ошибка открытия файла");
        free(filename);
        return -1;
    }

    int shm_fd = shm_open("/my_shared_memory", O_CREAT | O_RDWR, 0666);
    printf("shm_fd: %d\n", shm_fd);

    if (shm_fd == -1)
    {
        perror("Ошибка при вызове shm_open");
        free(filename);
        return -1;
    }

    if (ftruncate(shm_fd, sizeof(float)) == -1)
    {
        perror("ftruncate failed");
        close(shm_fd);
        shm_unlink("/my_shared_memory");
        free(filename);
        return -1;
    }

    float *shared_sum = (float *)mmap(0, sizeof(float), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    printf("Shared Sum Value: %f\n", *shared_sum);
    if (shared_sum == MAP_FAILED)
    {
        perror("Ошибка отображения разделяемой памяти");
        close(file_fd);
        close(shm_fd);
        shm_unlink("/my_shared_memory");
        free(filename);
        return -1;
    }

    *shared_sum = 0;
    printf("Сумма чисел из файла до форка : %.2f\n", *shared_sum);

    pid_t child_pid = fork();
    if (child_pid == -1)
    {
        perror("Ошибка создания дочернего процесса");
        munmap(shared_sum, sizeof(float));
        close(file_fd);
        close(shm_fd);
        shm_unlink("/my_shared_memory");
        free(filename);
        return -1;
    }

    if (child_pid == 0)
    {
        dup2(file_fd, STDIN_FILENO);
        close(file_fd);

        execl("./child", "./child", NULL);

        perror("Ошибка при выполнении execl");
        exit(EXIT_FAILURE);
    }
    else
    {
        close(file_fd);

        wait(NULL);

        printf("Сумма чисел из файла: %.2f\n", *shared_sum);

        munmap(shared_sum, sizeof(float));
        close(shm_fd);
        shm_unlink("/my_shared_memory");
        free(filename);
    }

    return 0;
}
