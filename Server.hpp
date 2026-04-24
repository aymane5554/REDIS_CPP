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
#include <fstream>
#include <vector>
#include <list>
#include <exception>
#include <thread>
#include <mutex>
#include <atomic>
#include <algorithm>

#include "Cache.hpp"

#define DB_FILE     "costum.db"
#define WAL_FILE     "wal.log"
#define PORT        8080
#define MAX_EVENTS  64
#define BUF_SIZE    4096
#define TTL_SLEEP_TIME 60
#define SERIALIZE_TIME 30
#define TIMEOUT 10
#define MEMORY_LIMIT_MB 16 // or higher

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
    long long last_active_time_s;
    int lines;
    int bytes;
    int send;
    int sent;
    int fd;
    bool bad_alloc;
    bool quit;

    Client()
    {
        lines = 0;
        bytes = -1;
        send = 0;
        sent = 0;
        quit = false;
    }
    void clear()
    {
        lines = 0;
        bytes = -1;
        send = 0;
        sent = 0;
        quit = false;
        res_buff.clear();
        cmd.clear();
    }
};

void make_client_readable(int fd,  int epoll_fd, Client &cl);

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
        void Set(int fd);
        void Get(int fd);
        void Del(int fd);
        void Exists(int fd);
        void Expire(int fd);
        void Ttl(int fd);
        void Flush(int fd);
        void Type(int fd);
        void Quit(int fd);
        void Lpush(int fd);
        void Rpush(int fd);
        void Lpop(int fd);
        void Rpop(int fd);
        void Lrange(int fd);
        void ttl_thread();
        int Wal(std::vector <str> &cmd);
        int read_wal();
        Server();
        ~Server();
};

