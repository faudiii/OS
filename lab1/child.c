#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    printf("in child.c '%d'\n", getpid());

    float sum = 0;
    char temp[10000];
    int temp_index = 0;
    ssize_t bytesRead;

    while (1)
    {
        char c;
        ssize_t bytesRead = read(STDIN_FILENO, &c, 1);

        if (bytesRead <= 0)
        {
            // Конец файла, проверяем, есть ли несчитанные данные в буфере
            if (temp_index > 0)
            {
                temp[temp_index] = '\0';
                float num = atof(temp);
                sum += num;
            }
            break;
        }

        if ((c >= '0' && c <= '9') || c == '-' || c == '.') // Разрешаем цифры, минус, точку
        {
            temp[temp_index++] = c;
        }
        else if (temp_index > 0)
        {
            temp[temp_index] = '\0';
            float num = atof(temp); // Используем atof для конвертации
            sum += num;
            temp_index = 0;
        }
    }

    if (bytesRead == -1)
    {
        perror("Error reading from stdin");
        return 1;
    }

    char result[100]; // Буфер для результата
    int result_length = snprintf(result, sizeof(result), "Сумма чисел из файла: %.2f\n", sum);

    if (write(STDOUT_FILENO, result, result_length) == -1)
    {
        perror("Error writing to stdout");
        return 1;
    }

    return 0;
}
