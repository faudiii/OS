#include <iostream>
#include <iomanip>
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
#include <unistd.h>  // For execv, fork

const int DEFAULT_PORT = 6688;
int n = 2;

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

bool is_number(const std::string& val) {
    try {
        int tmp = stoi(val);
        return true;
    } catch (std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return false;
    }
}

int main() {
    std::string command;
    int root_id = 0;
    int root_pid = 0;
    void* context = zmq_ctx_new();
    void* root_socket = zmq_socket(context, ZMQ_REQ);

    std::cout << "Commands:\n"
              << "1. create (id)\n"
              << "2. exec (id) (text_string, pattern_string)\n"
              << "3. kill (id)\n"
              << "4. ping (id)\n"
              << "5. exit\n"
              << std::endl;

    while (true) {
        std::cin >> command;
        int node_id = 0;
        std::string id_str = "";
        std::string reply = "";

        if (command == "create") {
            ++n;
            std::cin >> id_str;

            if (!is_number(id_str)) {
                continue;
            }

            node_id = stoi(id_str);

            if (root_pid == 0) {
                zmq_bind(root_socket, get_port_name(DEFAULT_PORT + node_id).c_str());
                zmq_setsockopt(root_socket, ZMQ_RCVTIMEO, NULL, n * 500);
                zmq_setsockopt(root_socket, ZMQ_SNDTIMEO, NULL, n * 500);
                root_pid = fork();

                if (root_pid == -1) {
                    std::cout << "Error: Unable to create first worker node\n";
                    root_pid = 0;
                    exit(1);
                } else if (root_pid == 0) {
                    create_node(node_id, DEFAULT_PORT + node_id);
                } else {
                    root_id = node_id;
                    send_message(root_socket, "pid");
                    reply = receive_message(root_socket);
                }
            } else {
                zmq_setsockopt(root_socket, ZMQ_RCVTIMEO, NULL, n * 500);
                zmq_setsockopt(root_socket, ZMQ_SNDTIMEO, NULL, n * 500);
                std::string request = "create " + std::to_string(node_id);
                send_message(root_socket, request);
                reply = receive_message(root_socket);
            }

            std::cout << reply << std::endl;
        }

        if (command == "kill") {
            std::cin >> id_str;

            if (root_pid == 0) {
                std::cout << "Root is dead!" << std::endl;
                continue;
            }

            if (!is_number(id_str)) {
                continue;
            }

            node_id = stoi(id_str);

            if (node_id == root_id) {
                kill(root_pid, SIGKILL);
                root_id = 0;
                root_pid = 0;
                std::cout << "Ok\n";
                continue;
            }

            std::string request = "kill " + std::to_string(node_id);
            send_message(root_socket, request);
            reply = receive_message(root_socket);
            std::cout << reply << std::endl;
        }

        if (command == "exec") {
            std::string text_str = "";
            std::string pattern_str = "";
            std::cin >> id_str >> text_str >> pattern_str;

            if (root_pid == 0) {
                std::cout << "Root is dead!" << std::endl;
                continue;
            }

            if (!is_number(id_str)) {
                continue;
            }

            node_id = stoi(id_str);
            std::string request = "exec " + std::to_string(node_id) + " " +
                                  text_str + " " + pattern_str;
            send_message(root_socket, request);
            reply = receive_message(root_socket);
            std::cout << reply << std::endl;
        }

        if (command == "ping") {
            std::cin >> id_str;

            if (root_pid == 0) {
                std::cout << "Root is dead!" << std::endl;
                continue;
            }

            if (!is_number(id_str)) {
                continue;
            }

            node_id = stoi(id_str);
            std::string request = "ping " + std::to_string(node_id);
            send_message(root_socket, request);
            reply = receive_message(root_socket);
            std::cout << reply << std::endl;
        }

        if (command == "exit") {
            int t = system("killall -9 node");
            break;
        }
    }

    zmq_close(root_socket);
    zmq_ctx_destroy(context);
    return 0;
}
