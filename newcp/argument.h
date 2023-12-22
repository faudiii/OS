#ifndef ARG_H
#define ARG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "zmq.h"

#define SERVER_USAGE "usage: ./server -p <number of players (from 2 to 4)>\n"
#define CLIENT_USAGE "usage: ./client -l <login>\n"

#define SIZE_LOG 100
#define MSG_SIZE 2048

#define error(msg)                             \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)



typedef struct _result {
    char name[SIZE_LOG];
    size_t steps;
    int yep;
} Result;

typedef struct _args{

    char log[SIZE_LOG];
    int id1;
    size_t players;
    void *requester;
    int status;
   
    size_t result;
} Args;

#endif