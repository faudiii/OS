#include <zmq.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void *context;
void *subscriber;
void *notifier;

int main()
{
    context = zmq_ctx_new();

    // Socket for receiving words
    subscriber = zmq_socket(context, ZMQ_SUB);
    zmq_connect(subscriber, "tcp://localhost:5555");
    zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, "", 0);

     while (1)
    {
        char notification[256];
        zmq_recv(notifier, notification, sizeof(notification), 0);

        if (strcmp(notification, "new_word") == 0)
        {
            char word[256];
            zmq_recv(subscriber, word, sizeof(word), 0);
            printf("Received word to guess: %s\n", word);

            char guess[256];
            printf("Enter your guess: ");
            fgets(guess, sizeof(guess), stdin);
            guess[strlen(guess) - 1] = '\0'; // Remove the newline character

            if (strcmp(word, guess) == 0)
            {
                printf("Congratulations! You guessed the word.\n");
            }
            else
            {
                printf("Sorry, that's not the correct word.\n");
            }
        }
    }

    zmq_close(subscriber);
    zmq_close(notifier);
    zmq_ctx_destroy(context);

    return 0;
}
