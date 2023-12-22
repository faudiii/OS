#include <stdio.h>
#include <string.h>
#include <zmq.h>

typedef struct
{
    int id;
    char iq[64];
    char data[256]; // Поле для хранения данных процесса
} Process;

Process processes[10]; // Пример хранения информации о процессах

int findProcessIndexById(int id)
{
    for (int i = 0; i < 10; ++i)
    {
        if (processes[i].id == id)
        {
            return i;
        }
    }
    return -1; // Процесс с указанным ID не найден
}

int main()
{
    void *context = zmq_ctx_new();
    void *responder = zmq_socket(context, ZMQ_REP);
    zmq_bind(responder, "tcp://*:5555");

    while (1)
    {
        char buffer[256];
        zmq_recv(responder, buffer, sizeof(buffer), 0);
        printf("Received request: %s\n", buffer);

        if (strncmp(buffer, "create", 6) == 0)
        {
            // Обработка команды create
            int id;
            char iq[64];
            if (sscanf(buffer, "create %d %s", &id, iq) == 2)
            {
                int index = findProcessIndexById(id);
                if (index == -1)
                {
                    // Создаем новый процесс
                    int newIndex = -1;
                    for (int i = 0; i < 10; ++i)
                    {
                        if (processes[i].id == 0)
                        {
                            newIndex = i;
                            break;
                        }
                    }
                    if (newIndex != -1)
                    {
                        processes[newIndex].id = id;
                        strcpy(processes[newIndex].iq, iq);
                        sprintf(processes[newIndex].data, "Data for process %d", id);
                        zmq_send(responder, "Process created", 16, 0);
                    }
                    else
                    {
                        zmq_send(responder, "Too many processes", 19, 0);
                    }
                }
                else
                {
                    zmq_send(responder, "Process with the same ID already exists", 39, 0);
                }
            }
            else
            {
                zmq_send(responder, "Invalid parameters for create command", 37, 0);
            }
        }
        else if (strncmp(buffer, "exec", 4) == 0)
        {
            // Обработка команды exec
            int id;
            char substring[256];
            if (sscanf(buffer, "exec %d %s", &id, substring) == 2)
            {
                int index = findProcessIndexById(id);
                if (index != -1)
                {
                    // Ищем подстроку в данных процесса
                    if (strstr(processes[index].data, substring) != NULL)
                    {
                        zmq_send(responder, "Substring found", 16, 0);
                    }
                    else
                    {
                        zmq_send(responder, "Substring not found", 20, 0);
                    }
                }
                else
                {
                    zmq_send(responder, "Process not found", 17, 0);
                }
            }
            else
            {
                zmq_send(responder, "Invalid parameters for exec command", 35, 0);
            }
        }
        else if (strncmp(buffer, "exit", 4) == 0)
        {
            // Обработка команды exit
            zmq_send(responder, "Server is shutting down", 23, 0);
            break; // Выход из бесконечного цикла
        }
        else
        {
            zmq_send(responder, "Invalid command", 16, 0);
        }
    }

    zmq_close(responder);
    zmq_ctx_destroy(context);

    return 0;
}
