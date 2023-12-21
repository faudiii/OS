#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <zmq.h>

#define MAX_NODES 100

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

    if (child_pid == -1)
    {
        // Error handling
        perror("fork");
        exit(EXIT_FAILURE);
    }
    else if (child_pid == 0)
    {
        // Child process
        node.pid = getpid();
        void *context = zmq_ctx_new();
        node.socket = zmq_socket(context, ZMQ_REQ);

        if (parent_id != 1)
        {
            // Find the actual parent node based on parent_id
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
                char response[256];
                sprintf(response, "Error: Parent is unavailable");
                zmq_send(node.socket, response, strlen(response), 0);
                zmq_close(node.socket);
                node.id = -1;
                return node;
            }

            // Notify the parent about the new connection
            char create_message[256];
            sprintf(create_message, "create %d %d", id, tree->nodes[parentId].id);
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
    else
    {
        // Parent process
        // Wait for the child to finish creating the node
        int status;
        waitpid(child_pid, &status, 0);
        exit(EXIT_SUCCESS);
    }
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
            printTree(tree, controlNode);

            zmq_close(newNode.socket);
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
