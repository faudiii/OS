#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <zmq.h>

#define MAX_NODES 100

typedef struct
{
    int id;
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
    printf("%d (Control Node)\n", controlNode->id);

    for (int i = 2; i < tree->node_count; i++)
    {
        if (tree->nodes[i].id != 1)
        {
            printf("  └─ %d (Compute Node)\n", tree->nodes[i].id);
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
    node.pid = getpid();

    void *context = zmq_ctx_new();
    node.socket = zmq_socket(context, ZMQ_REQ);

    if (parent_id != 1)
    {
        char endpoint[20];
        sprintf(endpoint, "tcp://localhost:%d", parent_id);

        int rc = zmq_connect(node.socket, endpoint);
        if (rc != 0)
        {
            char response[256];
            sprintf(response, "Error: Parent is unavailable");
            zmq_send(node.socket, response, strlen(response), 0);
            zmq_close(node.socket);
            node.id = -1;
            return node;
        }

        // Notify the parent about the new connection
        char create_message[256];
        sprintf(create_message, "create %d %d", id, parent_id);
        zmq_send(node.socket, create_message, strlen(create_message), 0);

        usleep(1000000);

        // Add the node to the tree after establishing the connection
        addNodeToTree(tree, node);
    }
    else
    {
        // Add the control node to the tree
        addNodeToTree(tree, node);
    }

    return node;
}

void executeCommand(Node *controlNode, Tree *tree, const char *command)
{
    if (strncmp(command, "exec", 4) == 0)
    {
        // ...
    }
    else if (strncmp(command, "create", 6) == 0)
    {
        int id, parent_id;
        sscanf(command, "create %d %d", &id, &parent_id);

        Node newNode = createNode(id, parent_id, tree);

        if (newNode.id == id)
        {
            printf("New Compute Node Created: %d (pid: %d)\n", newNode.id, newNode.pid);
            zmq_close(newNode.socket);
        }
        else
        {
            char response[256];
            sprintf(response, "Error: Already exists");
            zmq_send(controlNode->socket, response, strlen(response), 0);
        }
    }
}

int main()
{
    system("clear");

    Tree tree;
    tree.node_count = 0;

    Node controlNode;
    controlNode.id = 1;
    controlNode.pid = getpid();
    controlNode.socket = NULL;

    addNodeToTree(&tree, controlNode);

    Node computeNode = createNode(2, controlNode.id, &tree);
    addNodeToTree(&tree, computeNode);

    if (computeNode.id != -1)
    {
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
    }

    return 0;
}
