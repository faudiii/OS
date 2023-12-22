#include "argument.h"
#include <stdio.h>
#include <stdbool.h>

int main()
{
    char answer[MSG_SIZE];

    void *context = zmq_ctx_new();
    if (!context)
        error("zmq_ctx_new");

    void *requester = zmq_socket(context, ZMQ_REQ);
    if (!requester)
        error("zmq_socket");

    int cn = zmq_connect(requester, "tcp://localhost:8888");
    printf("Connect to tcp://localhost:8888\n");
    if (cn)
        error("connect");

    Args myGame;
    myGame.players = 0;
    myGame.status = -1;

    zmq_send(requester, &myGame, sizeof(Args), 0);
    zmq_recv(requester, &myGame, sizeof(Args), 0);

    if (myGame.players == 0)
    {
        printf("There are no more places\n");
        return 0;
    }

    printf("Welcome! Try to guess the word.\n");

    bool guessedCorrectly = false;

    while (!guessedCorrectly)
    {
        printf("Enter your guess: ");
        scanf("%s", myGame.log);

        zmq_send(requester, &myGame, sizeof(Args), 0);
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
