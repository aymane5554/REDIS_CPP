#pragma once

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <ctime>
#include <cstring>

#include <iostream>
#include <vector>
#include <list>
#include <exception>
#include <thread>
#include <mutex>
#include <atomic>

#include "Cache.hpp"

#define PORT        8080
#define MAX_EVENTS  64
#define BUF_SIZE    4096
#define TTL_SLEEP_TIME 60
#define SERIALIZE_TIME 30
#define MEMORY_LIMIT_MB 16

extern std::atomic<bool> sigint;

class ERROR:std::exception
{
    private:
        std::string msg;
    public:
        ERROR(str m)
        {
            msg = m;
        }
        ~ERROR() throw()
        {}
        virtual const char *what() const throw()
        {
            return msg.c_str();
        }
};

struct Client
{
    str res_buff;
    str req_buff;
    std::vector <str> cmd;
    int lines;
    int bytes;
    int send;
    int sent;

    Client()
    {
        lines = 0;
        bytes = -1;
        send = 0;
        sent = 0;
    }
    void clear()
    {
        lines = 0;
        bytes = -1;
        send = 0;
        sent = 0;
        res_buff.clear();
        cmd.clear();
    }
};

class Server
{
    std::unordered_map <int, Client> clients;
    std::unordered_map <str, void(Server::*)(int)> cmd_func;
    Cache cache;
    std::mutex mtx;
    int server_fd;
    int epoll_fd;
    long long last_serialization;
    public:
        void run();
        void parse_request(int fd);
        void send_response(int fd);
        void safe_close(int fd);
        void Set(int fd); // SET key value → store it
        void Get(int fd); // GET key → retrieve it
        void Del(int fd); // DEL key → delete it
        void Exists(int fd); // EXISTS key → does it exist?
        void Expire(int fd); // EXPIRE key seconds → auto-delete after N seconds
        void Ttl(int fd); // TTL key → how many seconds left?
        void Flush(int fd);
        void ttl_thread();
        Server();
        ~Server();
};

