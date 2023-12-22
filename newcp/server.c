#include "argument.h"
#include <stdlib.h>
#include <string.h>

char *generateWord()
{
    // In a real scenario, you might have a more sophisticated logic to generate words.
    return "apple";
}

int main(int argc, char **argv)
{
    printf("Server start.\n");
    char answer[MSG_SIZE];

    size_t countOfPlayers;
    int id[4] = {0};
    Args tmp;

    char *secretWord = generateWord();

    void *context = zmq_ctx_new();
    if (!context)
        error("zmq_ctx_new");

    void *responder = zmq_socket(context, ZMQ_REP);
    if (!responder)
        error("zmq_socket");

    int bind = zmq_bind(responder, "tcp://*:8888");
    if (bind)
        error("bind");

    if (argc == 2)
    {
        countOfPlayers = atoi(argv[1]);
    }
    else
    {
        printf("Usage: %s <number_of_players>\n", argv[0]);
        return 0;
    }

    for (size_t ready = 0; ready < countOfPlayers; ++ready)
    {
        zmq_recv(responder, &tmp, sizeof(Args), 0);
        if (!tmp.players)
        {
            printf("User %s connected and start to play\n", tmp.log);
            tmp.id1 = ready;
            tmp.players = countOfPlayers;
            printf("\t%zu players are ready\n", ready + 1);
        }
        else
        {
            --ready;
        }
        zmq_send(responder, &tmp, sizeof(Args), 0);
    }

    printf("All players connected\n");

    while (countOfPlayers)
    {
        zmq_recv(responder, &tmp, sizeof(Args), 0);
        if (!id[tmp.id1])
        {
            --countOfPlayers;
            id[tmp.id1] = 1;
            printf("User %s started guessing the word\n", tmp.log);
            strcpy(answer, "Welcome! Try to guess the word.");
            zmq_send(responder, &answer, sizeof(answer), 0);

            while (1)
            {
                zmq_recv(responder, &tmp, sizeof(Args), 0);
                if (strcmp(tmp.log, secretWord) == 0)
                {
                    strcpy(answer, "Congratulations! You guessed the word.");
                    zmq_send(responder, &answer, sizeof(answer), 0);
                    break;
                }
                else
                {
                    sprintf(answer, "Incorrect guess. Try again.");
                    zmq_send(responder, &answer, sizeof(answer), 0);
                    printf("Player %s guessed %s (Incorrect)\n", tmp.log, tmp.log);
                }
            }
        }
        zmq_send(responder, &tmp, sizeof(Args), 0);
    }

    printf("Game over. The word was: %s\n", secretWord);

    zmq_close(responder);
    zmq_ctx_destroy(context);
    return 0;
}
