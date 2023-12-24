#ifndef ARG_H
#define ARG_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "zmq.h"

#define SIZE_LOG 100
#define MSG_SIZE 1000

typedef struct _args
{
    char name[SIZE_LOG];
    char log[SIZE_LOG];
    int id1;
    size_t players;
    void *requester;
} Args;

#endif