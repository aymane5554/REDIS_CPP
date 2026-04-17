#include "Server.hpp"

void Server::LRU(int fd)
{
    Client &cl = clients[fd];

    if (clients[fd].cmd.size() != 1)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.LRU();
    }

    cl.res_buff = "+OK\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}


void Server::Del(int fd)
{
    Client &cl = clients[fd];

    if (clients[fd].cmd.size() != 2)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.Del(clients[fd].cmd[1]);
    }

    cl.res_buff = ":1\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Get(int fd)
{
    Client &cl = clients[fd];
    str val;

    if (clients[fd].cmd.size() != 2)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        val = cache.Get(clients[fd].cmd[1]);
    }

    cl.res_buff = val + "\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Set(int fd)
{
    Client &cl = clients[fd];

    if (clients[fd].cmd.size() != 3)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.Set(clients[fd].cmd);
    }

    cl.res_buff = "+OK\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Exists(int fd)
{
    Client &cl = clients[fd];

    if (clients[fd].cmd.size() != 2)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.Exists(clients[fd].cmd[1]);
    }

    cl.res_buff = ":1\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Expire(int fd)
{
    Client &cl = clients[fd];
    long long seconds;

    if (clients[fd].cmd.size() != 3)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }
    try 
    {
        seconds = std::stol(clients[fd].cmd[2]);
    }
    catch (std::exception &e)
    {
        throw ERROR("-ERR value is not an integer or out of range\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.Expire(clients[fd].cmd[1], seconds);
    }

    cl.res_buff = ":1\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Ttl(int fd)
{
    Client &cl = clients[fd];
    long long seconds;

    if (clients[fd].cmd.size() != 2)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        seconds = cache.Ttl(clients[fd].cmd[1]);
    }

    cl.res_buff = ":" + std::to_string(seconds);
    cl.res_buff += "\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}

void Server::Flush(int fd)
{
    Client &cl = clients[fd];

    if (clients[fd].cmd.size() != 1)
    {
        throw ERROR("-ERR unvalid number of argument\r\n");
    }

    {
        std::lock_guard<std::mutex> lock(mtx);
        cache.Flush();
    }

    cl.res_buff = "+OK\r\n";
    cl.send = cl.res_buff.length();
    cl.sent = send(fd, cl.res_buff.c_str(), cl.send, MSG_NOSIGNAL);
}
