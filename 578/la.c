#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <zmq.h>

#define MAX_NODES 100
#define MAX_STRING_LENGTH 256

typedef struct
{
    int id;
    int parentId;
    pthread_t thread;
    void *context;
    void *socket;
} Node;

typedef struct
{
    Node nodes[MAX_NODES];
    int node_count;
} Tree;

void *executeThread(void *data)
{
    Node *node = (Node *)data;

    while (1)
    {
        char command[256];
        zmq_recv(node->socket, command, sizeof(command), 0);

        if (strncmp(command, "exec", 4) == 0)
        {
            char text[MAX_STRING_LENGTH];
            char pattern[MAX_STRING_LENGTH];
            int id;
            sscanf(command, "exec %d %s %s", &id, text, pattern);

            // Выполняем вычисления
            char response[256];
            sprintf(response, "Okii:%d:%s", id, text);
            zmq_send(node->socket, response, strlen(response), 0);
        }
        else if (strncmp(command, "create", 6) == 0)
        {
            int id, parent_id;
            sscanf(command, "create %d %d", &id, &parent_id);

            Tree *tree = (Tree *)node;
            Node newNode = {0};
            newNode.id = id;
            newNode.parentId = parent_id;
            newNode.context = zmq_ctx_new();
            newNode.socket = zmq_socket(newNode.context, ZMQ_REP);

            if (parent_id != 0)
            {
                int parentId = -1;
                for (int j = 0; j < tree->node_count; ++j)
                {
                    if (tree->nodes[j].id == parent_id)
                    {
                        parentId = j;
                        break;
                    }
                }

                if (parentId == -1)
                {
                    printf("Error: Parent node with id %d not found\n", parent_id);
                    newNode.id = -1;
                    zmq_send(node->socket, "Error: Parent not found", 23, 0);
                    continue;
                }

                char endpoint[20];
                sprintf(endpoint, "tcp://localhost:%d", tree->nodes[parentId].id);

                int rc = zmq_bind(newNode.socket, endpoint);
                if (rc != 0)
                {
                    perror("zmq_bind");
                    exit(EXIT_FAILURE);
                }
            }

            pthread_create(&newNode.thread, NULL, executeThread, (void *)&newNode);

            tree->nodes[tree->node_count++] = newNode;

            char response[256];
            sprintf(response, "New node created: %d", id);
            zmq_send(node->socket, response, strlen(response), 0);
        }
    }

    return NULL;
}

void executeCommand(Node *controlNode, Tree *tree, const char *command)
{
    if (strncmp(command, "exec", 4) == 0 || strncmp(command, "create", 6) == 0)
    {
        int id;
        sscanf(command, "%*s %d", &id);

        if (id >= 0 && id < tree->node_count && tree->nodes[id].id != -1)
        {
            // Отправляем команду узлу через сокет
            zmq_send(tree->nodes[id].socket, command, strlen(command), 0);

            // Дожидаемся ответа от узла
            char response[256];
            zmq_recv(tree->nodes[id].socket, response, sizeof(response), 0);
            printf("%s\n", response);
        }
    }
}

void printTree(Tree *tree, Node *controlNode)
{
    system("clear");
    printf("Tree Structure:\n");
    printf("%d (Control Node)\n", controlNode->id);

    for (int i = 1; i < tree->node_count; i++)
    {
        if (tree->nodes[i].id != -1)
        {
            printf("  └─ %d (Node, Parent: %d)\n", tree->nodes[i].id, tree->nodes[i].parentId);
        }
    }

    fflush(stdout);
}

int main()
{
    system("clear");

    Tree tree;
    tree.node_count = 0;

    Node controlNode;
    controlNode.id = 0;
    controlNode.parentId = -1;
    controlNode.context = zmq_ctx_new();
    controlNode.socket = zmq_socket(controlNode.context, ZMQ_REP);

    int rc = zmq_bind(controlNode.socket, "tcp://*:5555");
    if (rc != 0)
    {
        perror("zmq_bind");
        exit(EXIT_FAILURE);
    }

    pthread_create(&controlNode.thread, NULL, executeThread, (void *)&tree);

    tree.nodes[tree.node_count++] = controlNode;

    printTree(&tree, &controlNode);

    while (1)
    {
        char command[256];
        printf("> ");
        fgets(command, sizeof(command), stdin);

        char *newline = strchr(command, '\n');
        if (newline != NULL)
        {
            *newline = '\0';
        }

        executeCommand(&controlNode, &tree, command);
        printTree(&tree, &controlNode);
    }

    return 0;
}
