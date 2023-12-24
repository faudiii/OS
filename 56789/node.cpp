#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cctype>
#include <algorithm>
#include <vector>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <queue>
#include <stack>
#include <limits>
#include <numeric>
#include <functional>
#include <iterator>
#include <cassert>
#include "zmq.h"
#include <unistd.h> 
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <zmq.h>
#include <unistd.h>
#include <sys/types.h>
#include <cstdio> // Include this header for printf

const int DEFAULT_PORT = 6688;
int nl = 2, nr = 2;

bool send_message(void* socket, const std::string& message_string) {
    int rc;
    zmq_msg_t msg;

    rc = zmq_msg_init_size(&msg, message_string.size());

    if (rc != 0) {
        std::cerr << "Error while initializing message: " << zmq_strerror(errno) << std::endl;
        return false;
    }

    memcpy(zmq_msg_data(&msg), message_string.c_str(), message_string.size());
    rc = zmq_msg_send(&msg, socket, 0);

    if (rc == -1) {
        std::cerr << "Error while sending message: " << zmq_strerror(errno) << std::endl;
        zmq_msg_close(&msg);
        return false;
    }

    zmq_msg_close(&msg);
    return true;
}

std::string receive_message(void* socket) {
    int rc;
    zmq_msg_t msg;

    rc = zmq_msg_init(&msg);

    if (rc != 0) {
        std::cerr << "Error while initializing message: " << zmq_strerror(errno) << std::endl;
        return "";
    }

    rc = zmq_msg_recv(&msg, socket, 0);

    if (rc == -1) {
        std::cerr << "Error while receiving message: " << zmq_strerror(errno) << std::endl;
        zmq_msg_close(&msg);
        return "";
    }

    std::string received_message(static_cast<char*>(zmq_msg_data(&msg)), zmq_msg_size(&msg));
    zmq_msg_close(&msg);
    return received_message;
}

void create_node(const int& id, const int& port) {
    char* arg0 = strdup("./node");
    char* arg1 = strdup((std::to_string(id)).c_str());
    char* arg2 = strdup((std::to_string(port)).c_str());
    char* args[] = {arg0, arg1, arg2, nullptr};
    execv("./node", args);
}

std::string get_port_name(const int& port) {
    return "tcp://127.0.0.1:" + std::to_string(port);
}

void create_helper(void* parent_socket, void* socket, int& create_id, int& id, int& pid) {
    if (pid == -1) {
        send_message(parent_socket, "Error: Cannot fork");
        pid = 0;
    } else if (pid == 0) {
        create_node(create_id, DEFAULT_PORT + create_id);
    } else {
        id = create_id;
        send_message(socket, "pid");
        send_message(parent_socket, receive_message(socket));
    }
}

void exec_helper(void* parent_socket, void* socket, int& id, int& pid, std::string& request_string) {
    if (pid == 0) {
        std::string receive_message = "Error:" + std::to_string(id);
        receive_message += ": Not found";
        send_message(parent_socket, receive_message);
    } else {
        send_message(socket, request_string);
        std::string str = receive_message(socket);
        if (str == "")
            str = "Error: Node is unavailable";
        send_message(parent_socket, str);
    }
}

void ping_helper(void* parent_socket, void* socket, int& id, int& pid, std::string& request_string) {
    if (pid == 0) {
        std::string receive_message = "Error:" + std::to_string(id);
        receive_message += ": Not found";
        send_message(parent_socket, receive_message);
    } else {
        send_message(socket, request_string);
        std::string str = receive_message(socket);
        if (str == "") {
            str = "Ok: 0";
        }
        send_message(parent_socket, str);
    }
}

void exec(std::istringstream& command_stream, void* parent_socket, void* left_socket, void* right_socket,
          int& left_pid, int& right_pid, int& id, std::string& request_string) {
    std::string text_string, pattern_string;
    int exec_id;
    command_stream >> exec_id;

    if (exec_id == id) {
        command_stream >> text_string;
        command_stream >> pattern_string;
        std::string reply = "";
        std::string answer = "";
        int index = 0;

        while ((index = text_string.find(pattern_string, index)) != std::string::npos) {
            answer += std::to_string(index) + ";";
            index += 1;
        }

        if (!answer.empty()) {
            answer.pop_back();
        }

        reply = "Ok:" + std::to_string(id) + ":";
        printf("Process ID: %d\n", getpid());  // Print the process ID

        if (!answer.empty()) {
            reply += answer;
        } else {
            reply += "-1";
        }

        send_message(parent_socket, reply);
    } else if (exec_id < id) {
        exec_helper(parent_socket, left_socket, exec_id, left_pid, request_string);
    } else {
        exec_helper(parent_socket, right_socket, exec_id, right_pid, request_string);
    }
}

void ping(std::istringstream& command_stream, void* parent_socket, void* left_socket, void* right_socket,
          int& left_pid, int& right_pid, int& id, std::string& request_string) {
    int ping_id;
    std::string reply;
    command_stream >> ping_id;

    if (ping_id == id) {
        reply = "Ok: 1";
        send_message(parent_socket, reply);
    } else if (ping_id < id) {
        ping_helper(parent_socket, left_socket, ping_id, left_pid, request_string);
    } else {
        ping_helper(parent_socket, right_socket, ping_id, right_pid, request_string);
    }
}

void kill_node(void* parent_socket, void* socket, int& delete_id, int& id, int& pid, std::string& request_string) {
    if (id == 0) {
        send_message(parent_socket, "Error: Not found");
    } else if (id == delete_id) {
        send_message(socket, "kill_children");
        receive_message(socket);
        kill(pid, SIGKILL);
        id = 0;
        pid = 0;
        send_message(parent_socket, "Ok");
    } else {
        send_message(socket, request_string);
        send_message(parent_socket, receive_message(socket));
    }
}

void kill_children(void* parent_socket, void* left_socket, void* right_socket, int& left_pid, int& right_pid) {
    if (left_pid == 0 && right_pid == 0) {
        send_message(parent_socket, "Ok");
    } else {
        if (left_pid != 0) {
            send_message(left_socket, "kill_children");
            receive_message(left_socket);
            kill(left_pid, SIGKILL);
        }

        if (right_pid != 0) {
            send_message(right_socket, "kill_children");
            receive_message(right_socket);
            kill(right_pid, SIGKILL);
        }

        send_message(parent_socket, "Ok");
    }
}

int main(int argc, char** argv) {
    int id = std::stoi(argv[1]);
    int parent_port = std::stoi(argv[2]);
    void* context = zmq_ctx_new();
    void* parent_socket = zmq_socket(context, ZMQ_REP);
    void* left_socket = zmq_socket(context, ZMQ_REQ);
    void* right_socket = zmq_socket(context, ZMQ_REQ);

    zmq_connect(parent_socket, get_port_name(parent_port).c_str());
    zmq_setsockopt(parent_socket, ZMQ_RCVTIMEO, NULL, 500);
    zmq_setsockopt(parent_socket, ZMQ_SNDTIMEO, NULL, 500);

    int left_id = 0, right_id = 0, left_pid = 0, right_pid = 0;

    while (true) {
        std::string request = receive_message(parent_socket);
        std::istringstream command_stream(request);
        std::string command;
        command_stream >> command;

        if (command == "id") {
            std::string reply = "Ok: " + std::to_string(id);
            send_message(parent_socket, reply);
        }

        if (command == "pid") {
            std::string reply = "Ok: " + std::to_string(getpid());
            send_message(parent_socket, reply);
        }

        if (command == "create") {
            int create_id;
            command_stream >> create_id;

            if (create_id == id) {
                std::string reply = "Error: Already exists";
                send_message(parent_socket, reply);
            } else if (create_id < id) {
                ++nl;

                if (left_pid == 0) {
                    zmq_bind(left_socket, get_port_name(DEFAULT_PORT + create_id).c_str());
                    zmq_setsockopt(left_socket, ZMQ_RCVTIMEO, NULL, nl * 500);
                    zmq_setsockopt(left_socket, ZMQ_SNDTIMEO, NULL, nl * 500);
                    left_pid = fork();
                    create_helper(parent_socket, left_socket, create_id, left_id, left_pid);
                } else {
                    send_message(left_socket, request);
                    std::string reply = receive_message(left_socket);

                    if (reply == "") {
                        reply = "Error: Node is unavailable";
                    } else {
                        zmq_setsockopt(left_socket, ZMQ_RCVTIMEO, NULL, nl * 500);
                        zmq_setsockopt(left_socket, ZMQ_SNDTIMEO, NULL, nl * 500);
                    }

                    send_message(parent_socket, reply);
                }
            } else {
                ++nr;

                if (right_pid == 0) {
                    zmq_bind(right_socket, get_port_name(DEFAULT_PORT + create_id).c_str());
                    zmq_setsockopt(right_socket, ZMQ_RCVTIMEO, NULL, nr * 500);
                    zmq_setsockopt(right_socket, ZMQ_SNDTIMEO, NULL, nr * 500);
                    right_pid = fork();
                    create_helper(parent_socket, right_socket, create_id, right_id, right_pid);
                } else {
                    send_message(right_socket, request);
                    std::string reply = receive_message(right_socket);

                    if (reply == "") {
                        reply = "Error: Node is unavailable";
                    } else {
                        zmq_setsockopt(right_socket, ZMQ_RCVTIMEO, NULL, nr * 500);
                        zmq_setsockopt(right_socket, ZMQ_SNDTIMEO, NULL, nr * 500);
                    }

                    send_message(parent_socket, reply);
                }
            }
        }

        if (command == "exec") {
            exec(command_stream, parent_socket, left_socket, right_socket, left_pid, right_pid, id, request);
        }

        if (command == "ping") {
            ping(command_stream, parent_socket, left_socket, right_socket, left_pid, right_pid, id, request);
        }

        if (command == "kill") {
            int delete_id;
            command_stream >> delete_id;

            if (delete_id < id) {
                kill_node(parent_socket, left_socket, delete_id, left_id, left_pid, request);
            } else {
                kill_node(parent_socket, right_socket, delete_id, right_id, right_pid, request);
            }
        }

        if (command == "kill_children") {
            kill_children(parent_socket, left_socket, right_socket, left_pid, right_pid);
        }
    }

    zmq_close(left_socket);
    zmq_close(right_socket);
    zmq_ctx_destroy(context);
    return 0;
}
