#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

int main()
{
    printf("start pid '%d'\n", getpid());

    char filename[256];
    printf("Введите имя файла: ");

    scanf("%s", filename);

    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
    {
        perror("Ошибка создания канала");
        return -1;
    }

    int file_fd = open(filename, O_RDONLY);

    pid_t child_pid = fork();

    if (child_pid == -1)
    {
        perror("Ошибка создания дочернего процесса");
        return -1;
    }

    if (child_pid == 0)
    {
        printf("child pid '%d'\n", getpid());

        dup2(file_fd, STDIN_FILENO);

        // Перенаправляем стандартный вывод в pipe_fd[1]
        dup2(pipe_fd[1], STDOUT_FILENO);

        close(pipe_fd[0]); // Закрываем чтение из канала

        execl("./child", "./child", NULL);

        perror("Ошибка в exec");
        return -1;
    }
    else
    {
        printf("parent pid '%d'\n", getpid());

        close(pipe_fd[1]); // Закрываем запись в канал

        char buffer[256];
        ssize_t bytes_read;

        // Читаем данные из канала и выводим их
        while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer))) > 0)
        {
            write(STDOUT_FILENO, buffer, bytes_read);
        }

        wait(NULL);
    }

    return 0;
}
