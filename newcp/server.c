#include "help.h"
#include <stdlib.h>
#include <string.h>

char *generateWord()
{
    return "apple";
}

int main(int argc, char **argv)
{
    printf("Server start.\n");
    char answer[MSG_SIZE];

    size_t countOfPlayers;
    int id[4] = {0};
    Args round;

    char *secretWord = generateWord();

    void *context = zmq_ctx_new();
    if (!context)
    {
        perror("zmq_ctx_new");
        return -1;
    }

    void *responder = zmq_socket(context, ZMQ_REP);
    if (!responder)
    {
        perror("zmq_socket");
        return -1;
    }

    int bind = zmq_bind(responder, "tcp://*:3250");
    if (bind)
    {
        perror("bind");
        return -1;
    }

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
        zmq_recv(responder, &round, sizeof(Args), 0);
        if (!round.players)
        {
            printf("User %s connected\n", round.name);
            round.id1 = ready;
            round.players = countOfPlayers;
            printf("\t%zu players are ready\n", ready + 1);
        }
        else
        {
            --ready;
        }
        zmq_send(responder, &round, sizeof(Args), 0);
    }

    printf("All players are connected\n");

    while (countOfPlayers)
    {
        zmq_recv(responder, &round, sizeof(Args), 0);
        if (!id[round.id1])
        {
            --countOfPlayers;
            id[round.id1] = 1;

            strcpy(answer, "guess the word.");
            zmq_send(responder, &answer, sizeof(answer), 0);

            while (1)
            {
                zmq_recv(responder, &round, sizeof(Args), 0);
                if (strcmp(round.log, secretWord) == 0)
                {
                    strcpy(answer, "Congratulations! You guessed the word.");
                    zmq_send(responder, &answer, sizeof(answer), 0);
                    printf("Player %s guessed %s (correct)\n", round.name, round.log);
                    break;
                }
                else
                {
                    sprintf(answer, "Incorrect guess. Try again.");
                    zmq_send(responder, &answer, sizeof(answer), 0);
                    printf("Player %s guessed %s (Incorrect)\n", round.name, round.log);
                }
            }
        }
        zmq_send(responder, &round, sizeof(Args), 0);
    }

    printf("Game over. The word was: %s\n", secretWord);

    zmq_close(responder);
    zmq_ctx_destroy(context);
    return 0;
}