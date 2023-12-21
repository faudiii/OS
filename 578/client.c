#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "unistd.h" // fork() and getpid() declarations are here

void substringSearch(const char *text, const char *pattern, int nodeId)
{
    char response[256];
    int position = -1;
    const char *found = text;

    while (found != NULL)
    {
        found = strstr(found, pattern);

        if (found != NULL)
        {
            position = found - text;
            sprintf(response, "Okii:%d:%d", nodeId, position);
            printf("%s\n", response);
            found++; // Move to the next character after the found substring
        }
    }
}

int main(int argc, char *argv[])
{
    printf("start pid '%d'\n", getpid());

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <nodeId> <text> <pattern>\n", argv[0]);
        return 1;
    }

    int nodeId = atoi(argv[1]);
    const char *text = argv[2];
    const char *pattern = argv[3];

    substringSearch(text, pattern, nodeId);

    // Wait for user input before exiting
    printf("Client with ID %d has completed the search. Press Enter to exit.\n", nodeId);
    getchar();

    return 0;
}
