#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <zmq.h>

#define MAX_NODES 100
#define MAX_STRING_LENGTH 256

typedef struct
{
    int id;
    int parentId;
    pid_t pid;
    void *socket;
} Node;

typedef struct
{
    Node nodes[MAX_NODES];
    int node_count;
} Tree;

void printTree(Tree *tree, Node *controlNode)
{
    system("clear");
    printf("Tree Structure:\n");
    printf("%d (Control Node pid: %d)\n", controlNode->id, controlNode->pid);

    for (int i = 2; i < tree->node_count; i++)
    {
        if (tree->nodes[i].id != -1)
        {
            printf("  └─ %d (Compute Node, Parent: %d, pid: %d)\n", tree->nodes[i].id, tree->nodes[i].parentId, tree->nodes[i].pid);
        }
    }

    fflush(stdout);
}

void addNodeToTree(Tree *tree, Node newNode)
{
    tree->nodes[tree->node_count++] = newNode;
}

Node createNode(int id, int parent_id, Tree *tree)
{
    Node node;
    node.id = id;
    node.parentId = parent_id;
    pid_t child_pid = fork();
    void *context = zmq_ctx_new();

    if (child_pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (child_pid == 0)
    {
        node.pid = getpid();
        node.socket = zmq_socket(context, ZMQ_REP);

        if (parent_id != 1)
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
                node.id = -1;
                return node;
            }

            char endpoint[20];
            sprintf(endpoint, "tcp://localhost:%d", tree->nodes[parentId].id);

            int rc = zmq_connect(node.socket, endpoint);
            if (rc != 0)
            {
                perror("zmq_connect");
                exit(EXIT_FAILURE);
            }

            char create_message[256];
            sprintf(create_message, "create %d %d", id, tree->nodes[parentId].id);
            zmq_send(node.socket, create_message, strlen(create_message), 0);

            // Дождитесь ответа от распределенного узла
            char response[256];
            zmq_recv(node.socket, response, sizeof(response), 0);
            printf("%s\n", response);

            usleep(1000000);

            addNodeToTree(tree, node);

            while (1)
            {
                char command[256];
                zmq_recv(node.socket, command, sizeof(command), 0);

                if (strncmp(command, "exec", 4) == 0)
                {
                    char text[MAX_STRING_LENGTH];
                    char pattern[MAX_STRING_LENGTH];
                    sscanf(command, "exec %s %s", text, pattern);
                }
            }
        }
        else
        {
            addNodeToTree(tree, node);
        }

        return node;
    }
    else
    {
        int status;
        waitpid(child_pid, &status, 0);
        exit(EXIT_SUCCESS);
    }
}

void executeCommand(Node *controlNode, Tree *tree, const char *command)
{
    if (strncmp(command, "exec", 4) == 0)
    {
        int id;
        char text[MAX_STRING_LENGTH];
        char pattern[MAX_STRING_LENGTH];
        sscanf(command, "exec %d %s %s", &id, text, pattern);
        printf("this one'%d'\n", getpid());

        // if (id > 1 && id < tree->node_count && tree->nodes[id].id != -1)
        // {
        //     pid_t client_pid = fork();

        //     if (client_pid == -1)
        //     {
        //         perror("fork");
        //         exit(EXIT_FAILURE);
        //     }
        //     else if (client_pid == 0)
        //     {

        //         char *args[] = {"client", (char *)malloc(2), text, pattern, NULL};
        //         sprintf(args[1], "%d", id);

        //         execv("./client", args);

        //         // Если execv вернул ошибку, выведите сообщение об ошибке
        //         perror("execv");
        //         exit(EXIT_FAILURE);
        //     }
        //     // The main program continues running here
        //     // Optionally, you can wait for the client process to finish
        //     else
        //     {
        //         int status;
        //         waitpid(client_pid, &status, 0);
        //         printf("Client process (ID: %d) finished with status: %d\n", client_pid, status);
        //     }
        // }
    }
    else if (strncmp(command, "create", 6) == 0)
    {
        int id, parent_id;
        sscanf(command, "create %d %d", &id, &parent_id);

        Node newNode = createNode(id, parent_id, tree);

        if (newNode.id == id)
        {
            printf("New Compute Node Created: %d (pid: %d)\n", newNode.id, newNode.pid);
            printTree(tree, controlNode);
            fflush(stdout);
        }
        else
        {
            char response[256];
            sprintf(response, "Error: Already exists");
            zmq_send(controlNode->socket, response, strlen(response), 0);
            fflush(stdout);
        }
    }
}

int main()
{
    system("clear");

    Tree tree;
    tree.node_count = 1;

    Node controlNode;
    controlNode.id = 1;
    controlNode.parentId = -1;
    controlNode.pid = getpid();
    controlNode.socket = NULL;

    addNodeToTree(&tree, controlNode);

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
