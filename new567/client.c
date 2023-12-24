#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <zmq.h>

#define WAIT_TIME 1000
#define PORT_BASE 5050

int send_message(void *socket, const char *message_string)
{
    zmq_msg_t message;
    zmq_msg_init_size(&message, strlen(message_string));
    memcpy(zmq_msg_data(&message), message_string, strlen(message_string));

    int sent = zmq_msg_send(&message, socket, 0);
    zmq_msg_close(&message);

    return sent;
}

char *receive_message(void *socket)
{
    zmq_msg_t message;
    zmq_msg_init(&message);

    int received = zmq_msg_recv(&message, socket, 0);
    char *received_message = NULL;

    if (received != -1)
    {
        received_message = malloc(zmq_msg_size(&message) + 1);
        memcpy(received_message, zmq_msg_data(&message), zmq_msg_size(&message));
        received_message[zmq_msg_size(&message)] = '\0';
    }

    zmq_msg_close(&message);

    return received_message;
}

void create_node(int id, int port)
{
    char arg1[20];
    char arg2[20];
    snprintf(arg1, sizeof(arg1), "%d", id);
    snprintf(arg2, sizeof(arg2), "%d", port);

    char *args[] = {"./client", arg1, arg2, NULL};
    execv("./client", args);
}

char *get_port_name(const int port)
{
    char *port_name = malloc(30);
    snprintf(port_name, 30, "tcp://127.0.0.1:%d", port);
    return port_name;
}

void rl_create(void *parent_socket, void *socket, int *create_id, int *id, pid_t *pid)
{
    if (*pid == -1)
    {
        send_message(parent_socket, "Error: Cannot fork");
        *pid = 0;
    }
    else if (*pid == 0)
    {
        create_node(*create_id, PORT_BASE + *create_id);
    }
    else
    {
        *id = *create_id;
        send_message(socket, "pid");
        send_message(parent_socket, receive_message(socket));
    }
}

void rl_kill(void *parent_socket, void *socket, int *delete_id, int *id, pid_t *pid, char *request_string)
{
    if (*id == 0)
    {
        send_message(parent_socket, "Error: Not found");
    }
    else if (*id == *delete_id)
    {
        send_message(socket, "kill_children");
        receive_message(socket);
        kill(*pid, SIGTERM);
        kill(*pid, SIGKILL);
        *id = 0;
        *pid = 0;
        send_message(parent_socket, "Ok");
    }
    else
    {
        send_message(socket, request_string);
        send_message(parent_socket, receive_message(socket));
    }
}

void rl_exec(void *parent_socket, void *socket, int *id, pid_t *pid, char *request_string)
{
    if (*pid == 0)
    {
        char error_message[50];
        snprintf(error_message, sizeof(error_message), "Error:%d: Not found", *id);
        send_message(parent_socket, error_message);
    }
    else
    {
        send_message(socket, request_string);
        send_message(parent_socket, receive_message(socket));
    }
}

void exec(
    char *command_stream,
    void *parent_socket,
    void *left_socket,
    void *right_socket,
    pid_t *left_pid,
    pid_t *right_pid,
    int *id,
    char *request_string,
    Timer *timer)
{
    char subcommand[10];
    int exec_id;
    sscanf(command_stream, "%d", &exec_id);

    if (exec_id == *id)
    {
        sscanf(command_stream, "%*d %s", subcommand);
        char receive_message[50];

        if (strcmp(subcommand, "start") == 0)
        {
            timer->start_timer();
            snprintf(receive_message, sizeof(receive_message), "Ok:%d", *id);
            send_message(parent_socket, receive_message);
        }
        else if (strcmp(subcommand, "stop") == 0)
        {
            timer->stop_timer();
            snprintf(receive_message, sizeof(receive_message), "Ok:%d", *id);
            send_message(parent_socket, receive_message);
        }
        else if (strcmp(subcommand, "time") == 0)
        {
            snprintf(receive_message, sizeof(receive_message), "Ok:%d: %d", *id, timer->get_time());
            send_message(parent_socket, receive_message);
        }
    }
    else if (exec_id < *id)
    {
        rl_exec(parent_socket, left_socket, &exec_id, left_pid, request_string);
    }
    else
    {
        rl_exec(parent_socket, right_socket, &exec_id, right_pid, request_string);
    }
}

void pingall(
    void *parent_socket,
    int *id,
    void *left_socket,
    void *right_socket,
    pid_t *left_pid,
    pid_t *right_pid)
{
    char res[50];
    char left_res[25];
    char right_res[25];

    snprintf(res, sizeof(res), "%d", *id);

    if (*left_pid != 0)
    {
        send_message(left_socket, "pingall");
        strcpy(left_res, receive_message(left_socket));
    }

    if (*right_pid != 0)
    {
        send_message(right_socket, "pingall");
        strcpy(right_res, receive_message(right_socket));
    }

    if (strlen(left_res) > 0 && strncmp(left_res, "Error", 5) != 0)
    {
        snprintf(res + strlen(res), sizeof(res) - strlen(res), " %s", left_res);
    }

    if (strlen(right_res) > 0 && strncmp(right_res, "Error", 5) != 0)
    {
        snprintf(res + strlen(res), sizeof(res) - strlen(res), " %s", right_res);
    }

    send_message(parent_socket, res);
}

void kill_children(void *parent_socket, void *left_socket, void *right_socket, pid_t *left_pid, pid_t *right_pid)
{
    if (*left_pid == 0 && *right_pid == 0)
    {
        send_message(parent_socket, "Ok");
    }
    else
    {
        if (*left_pid != 0)
        {
            send_message(left_socket, "kill_children");
            receive_message(left_socket);
            kill(*left_pid, SIGTERM);
            kill(*left_pid, SIGKILL);
        }

        if (*right_pid != 0)
        {
            send_message(right_socket, "kill_children");
            receive_message(right_socket);
            kill(*right_pid, SIGTERM);
            kill(*right_pid, SIGKILL);
        }

        send_message(parent_socket, "Ok");
    }
}

int main(int argc, char **argv)
{
    Timer timer;
    int id = atoi(argv[1]);
    int parent_port = atoi(argv[2]);

    void *context = zmq_ctx_new();
    void *parent_socket = zmq_socket(context, ZMQ_REP);
    zmq_connect(parent_socket, get_port_name(parent_port));

    pid_t left_pid = 0;
    pid_t right_pid = 0;
    int left_id = 0;
    int right_id = 0;

    void *left_socket = zmq_socket(context, ZMQ_REQ);
    void *right_socket = zmq_socket(context, ZMQ_REQ);

    while (1)
    {
        char *request_string = receive_message(parent_socket);
        char command[10];
        sscanf(request_string, "%s", command);

        if (strcmp(command, "id") == 0)
        {
            char parent_string[20];
            snprintf(parent_string, sizeof(parent_string), "Ok:%d", id);
            send_message(parent_socket, parent_string);
        }
        else if (strcmp(command, "pid") == 0)
        {
            char parent_string[20];
            snprintf(parent_string, sizeof(parent_string), "Ok:%d", getpid());
            send_message(parent_socket, parent_string);
        }
        else if (strcmp(command, "create") == 0)
        {
            int create_id;
            sscanf(request_string, "%*s %d", &create_id);

            if (create_id == id)
            {
                char message_string[30];
                snprintf(message_string, sizeof(message_string), "Error: Already exists");
                send_message(parent_socket, message_string);
            }
            else if (create_id < id)
            {
                if (left_pid == 0)
                {
                    zmq_bind(left_socket, get_port_name(PORT_BASE + create_id));
                    left_pid = fork();
                    rl_create(parent_socket, left_socket, &create_id, &left_id, &left_pid);
                }
                else
                {
                    send_message(left_socket, request_string);
                    send_message(parent_socket, receive_message(left_socket));
                }
            }
            else
            {
                if (right_pid == 0)
                {
                    zmq_bind(right_socket, get_port_name(PORT_BASE + create_id));
                    right_pid = fork();
                    rl_create(parent_socket, right_socket, &create_id, &right_id, &right_pid);
                }
                else
                {
                    send_message(right_socket, request_string);
                    send_message(parent_socket, receive_message(right_socket));
                }
            }
        }
        else if (strcmp(command, "kill") == 0)
        {
            int delete_id;
            sscanf(request_string, "%*s %d", &delete_id);

            if (delete_id < id)
            {
                rl_kill(parent_socket, left_socket, &delete_id, &left_id, &left_pid, request_string);
            }
            else
            {
                rl_kill(parent_socket, right_socket, &delete_id, &right_id, &right_pid, request_string);
            }
        }
        else if (strcmp(command, "exec") == 0)
        {
            exec(request_string, parent_socket, left_socket, right_socket, &left_pid, &right_pid, &id, request_string, &timer);
        }
        else if (strcmp(command, "pingall") == 0)
        {
            pingall(parent_socket, &id, left_socket, right_socket, &left_pid, &right_pid);
        }
        else if (strcmp(command, "kill_children") == 0)
        {
            kill_children(parent_socket, left_socket, right_socket, &left_pid, &right_pid);
        }

        free(request_string);

        if (parent_port == 0)
        {
            break;
        }
    }

    zmq_close(parent_socket);
    zmq_close(left_socket);
    zmq_close(right_socket);
    zmq_ctx_destroy(context);

    return 0;
}
