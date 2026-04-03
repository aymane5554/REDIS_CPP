#pragma once

#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include "Cache.hpp"

#define PORT        8080
#define MAX_EVENTS  64
#define BUF_SIZE    4096

class Server
{
    Cache cache;
    int server_fd;
    int epoll_fd;
    struct epoll_event ev, events[MAX_EVENTS];
    public:
        void run();
        Server();
        ~Server();
};

