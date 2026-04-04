#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#include <iostream>
#include <vector>
#include <exception>
#include <cstring>

#include "Cache.hpp"

#define PORT        8080
#define MAX_EVENTS  64
#define BUF_SIZE    4096

struct Client
{
    str res_buff;
    str req_buff;
    epoll_event *event;
    std::vector <str> cmd;
    int lines;
    int bytes;

    Client()
    {
        lines = 0;
        bytes = -1;
    }
};

class Server
{
    std::unordered_map <int, Client> clients;
    Cache cache;
    int server_fd;
    int epoll_fd;
    public:
        void run();
        void parse_request(int fd);
        void send_response(int fd);
        void safe_close(int fd);
        Server();
        ~Server();
};

