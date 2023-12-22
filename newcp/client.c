#include "help.h"
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s [name]\n", argv[0]);
        return 0;
    }
    char answer[MSG_SIZE];

    void *context = zmq_ctx_new();
    if (!context)
    {
        perror("Ошибка создания zmq_ctx_new");
        return -1;
    }

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (!requester)
    {
        perror("Ошибка создания zmq_socket");
        return -1;
    }

    int cn = zmq_connect(requester, "tcp://localhost:3249");
    printf("Connect to tcp://localhost:4444\n");
    if (cn)
    {
        perror("Ошибка создания zmq_connect");
        return -1;
    }

    Args round;
    round.players = 0;
    strcpy(round.name, argv[1]);
    zmq_send(requester, &round, sizeof(Args), 0);
    zmq_recv(requester, &round, sizeof(Args), 0);

    if (round.players == 0)
    {
        printf("no more places\n");
        return 0;
    }

    printf("guess the word.\n");

    bool guessedCorrectly = false;

    while (!guessedCorrectly)
    {
        printf("Enter your guess: ");
        scanf("%s", round.log);

        zmq_send(requester, &round, sizeof(Args), 0);
        zmq_recv(requester, &answer, sizeof(answer), 0);

        printf("%s\n", answer);

        if (strstr(answer, "Congratulations") != NULL)
        {
            guessedCorrectly = true;
        }
    }

    zmq_close(requester);
    zmq_ctx_destroy(context);
    return 0;
}