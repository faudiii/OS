#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <zmq.h>

#define WAIT_TIME 1000
#define PORT_BASE 5050
#define MSG_SIZE 256

typedef struct {
    int id;
    int pid;
} NodeInfo;

int send_message(void *socket, const char *message_string) {
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen(message_string));
    memcpy(zmq_msg_data(&message), message_string, strlen(message_string));
    int result = zmq_msg_send(&message, socket, 0);
    zmq_msg_close(&message);
    return result;
}

char *receive_message(void *socket) {
    zmq_msg_t message;
    zmq_msg_init(&message);
    zmq_msg_recv(&message, socket, 0);
    char *received_message = strdup((char *)zmq_msg_data(&message));
    zmq_msg_close(&message);
    return received_message;
}

void create_node(int id, int port) {
    char arg0[] = "./client";
    char arg1[10], arg2[10];
    snprintf(arg1, sizeof(arg1), "%d", id);
    snprintf(arg2, sizeof(arg2), "%d", port);

    char *args[] = {arg0, arg1, arg2, NULL};
    execv("./client", args);
}

char *get_port_name(int port) {
    char *port_name = malloc(MSG_SIZE);
    snprintf(port_name, MSG_SIZE, "tcp://127.0.0.1:%d", port);
    return port_name;
}

void rl_create(void *parent_socket, void *socket, NodeInfo *info) {
    if (info->pid == -1) {
        send_message(parent_socket, "Error: Cannot fork");
        info->pid = 0;
    } else if (info->pid == 0) {
        create_node(info->id, PORT_BASE + info->id);
    } else {
        send_message(socket, "pid");
        send_message(parent_socket, receive_message(socket));
    }
}

void rl_kill(void *parent_socket, void *socket, NodeInfo *info, int delete_id, const char *request_string) {
    if (info->id == 0) {
        send_message(parent_socket, "Error: Not found");
    } else if (info->id == delete_id) {
        send_message(socket, "kill_children");
        receive_message(socket);
        kill(info->pid, SIGTERM);
        kill(info->pid, SIGKILL);
        info->id = 0;
        info->pid = 0;
        send_message(parent_socket, "Ok");
    } else {
        send_message(socket, request_string);
        send_message(parent_socket, receive_message(socket));
    }
}

void rl_exec(void *parent_socket, void *socket, NodeInfo *info, const char *request_string) {
    if (info->pid == 0) {
        char response[MSG_SIZE];
        snprintf(response, MSG_SIZE, "Error: %d: Not found", info->id);
        send_message(parent_socket, response);
    } else {
        send_message(socket, request_string);
        send_message(parent_socket, receive_message(socket));
    }
}

void exec(zmq_msg_t *command_msg, void *parent_socket, void *left_socket, void *right_socket, NodeInfo *left_info, NodeInfo *right_info) {
    int exec_id;
    sscanf(zmq_msg_data(command_msg), "%d", &exec_id);

    if (exec_id == left_info->id) {
        char subcommand[10];
        sscanf(zmq_msg_data(command_msg), "%*d %s", subcommand);
        char response[MSG_SIZE];

        if (strcmp(subcommand, "start") == 0) {
            // Handle start
        } else if (strcmp(subcommand, "stop") == 0) {
            // Handle stop
        } else if (strcmp(subcommand, "time") == 0) {
            // Handle time
        }

        snprintf(response, MSG_SIZE, "Ok:%d", left_info->id);
        send_message(parent_socket, response);
    } else if (exec_id < left_info->id) {
        rl_exec(parent_socket, left_socket, left_info, zmq_msg_data(command_msg));
    } else {
        rl_exec(parent_socket, right_socket, right_info, zmq_msg_data(command_msg));
    }
}

void pingall(void *parent_socket, NodeInfo *info, void *left_socket, void *right_socket, NodeInfo *left_info, NodeInfo *right_info) {
    char response[MSG_SIZE];
    snprintf(response, MSG_SIZE, "%d", info->id);

    if (left_info->pid != 0) {
        send_message(left_socket, "pingall");
        strcat(response, " ");
        strcat(response, receive_message(left_socket));
    }

    if (right_info->pid != 0) {
        send_message(right_socket, "pingall");
        strcat(response, " ");
        strcat(response, receive_message(right_socket));
    }

    send_message(parent_socket, response);
}

void kill_children(void *parent_socket, void *left_socket, void *right_socket, NodeInfo *left_info, NodeInfo *right_info) {
    if (left_info->pid == 0 && right_info->pid == 0) {
        send_message(parent_socket, "Ok");
    } else {
        if (left_info->pid != 0) {
            send_message(left_socket, "kill_children");
            receive_message(left_socket);
            kill(left_info->pid, SIGTERM);
            kill(left_info->pid, SIGKILL);
        }

        if (right_info->pid != 0) {
            send_message(right_socket, "kill_children");
            receive_message(right_socket);
            kill(right_info->pid, SIGTERM);
            kill(right_info->pid, SIGKILL);
        }

        send_message(parent_socket, "Ok");
    }
}

int main(int argc, char **argv) {
    Timer timer;
    NodeInfo info;
    info.id = atoi(argv[1]);
    int parent_port = atoi(argv[2]);

    void *context = zmq_ctx_new();
    void *parent_socket = zmq_socket(context, ZMQ_REP);
    zmq_connect(parent_socket, get_port_name(parent_port));

    NodeInfo left_info, right_info;
    left_info.pid = 0;
    right_info.pid = 0;

    void *left_socket = zmq_socket(context, ZMQ_REQ);
    void *right_socket = zmq_socket(context, ZMQ_REQ);

    while (1) {
        char *request_string = receive_message(parent_socket);
        zmq_msg_t command_msg;
        zmq_msg_init_data(&command_msg, request_string, strlen(request_string), NULL, NULL);

        char command[10];
        sscanf(request_string, "%s", command);

        if (strcmp(command, "id") == 0) {
            char parent_string[MSG_SIZE];
            snprintf(parent_string, MSG_SIZE, "Ok:%d", info.id);
            send_message(parent_socket, parent_string);
        } else if (strcmp(command, "pid") == 0) {
            char parent_string[MSG_SIZE];
            snprintf(parent_string, MSG_SIZE, "Ok:%d", getpid());
            send_message(parent_socket, parent_string);
        } else if (strcmp(command, "create") == 0) {
            int create_id;
            sscanf(request_string, "%*s %d", &create_id);

            if (create_id == info.id) {
                char message_string[MSG_SIZE];
                snprintf(message_string, MSG_SIZE, "Error: Already exists");
                send_message(parent_socket, message_string);
            } else if (create_id < info.id) {
                if (left_info.pid == 0) {
                    zmq_bind(left_socket, get_port_name(PORT_BASE + create_id));
                    left_info.pid = fork();
                    rl_create(parent_socket, left_socket, &left_info);
                } else {
                    send_message(left_socket, request_string);
                    send_message(parent_socket, receive_message(left_socket));
                }
            } else {
                if (right_info.pid == 0) {
                    zmq_bind(right_socket, get_port_name(PORT_BASE + create_id));
                    right_info.pid = fork();
                    rl_create(parent_socket, right_socket, &right_info);
                } else {
                    send_message(right_socket, request_string);
                    send_message(parent_socket, receive_message(right_socket));
                }
            }
        } else if (strcmp(command, "kill") == 0) {
            int delete_id;
            sscanf(request_string, "%*s %d", &delete_id);

            if (delete_id < info.id) {
                rl_kill(parent_socket, left_socket, &left_info, delete_id, request_string);
            } else {
                rl_kill(parent_socket, right_socket, &right_info, delete_id, request_string);
            }
        } else if (strcmp(command, "exec") == 0) {
            exec(&command_msg, parent_socket, left_socket, right_socket, &left_info, &right_info);
        } else if (strcmp(command, "pingall") == 0) {
            pingall(parent_socket, &info, left_socket, right_socket, &left_info, &right_info);
        } else if (strcmp(command, "kill_children") == 0) {
            kill_children(parent_socket, left_socket, right_socket, &left_info, &right_info);
        }

        if (parent_port == 0) {
            break;
        }

        zmq_msg_close(&command_msg);
        free(request_string);
    }

    zmq_close(parent_socket);
    zmq_close(left_socket);
    zmq_close(right_socket);
    zmq_ctx_destroy(context);

    return 0;
}
