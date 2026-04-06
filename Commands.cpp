#include "Server.hpp"

void Server::Del(int fd)
{
    if (clients[fd].cmd.size() != 2)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Del(clients[fd].cmd[1]);
}

void Server::Get(int fd)
{
    if (clients[fd].cmd.size() != 2)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Get(clients[fd].cmd[1]);
}

void Server::Set(int fd)
{
    if (clients[fd].cmd.size() != 3)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Set(clients[fd].cmd);
}

void Server::Exists(int fd)
{
    if (clients[fd].cmd.size() != 2)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Exists(clients[fd].cmd[1]);
}

void Server::Expire(int fd)
{
    if (clients[fd].cmd.size() != 3)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Expire(clients[fd].cmd[1], std::stol(clients[fd].cmd[2]));
}

void Server::Ttl(int fd)
{
    if (clients[fd].cmd.size() != 2)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Ttl(clients[fd].cmd[1]);
}

void Server::Flush(int fd)
{
    if (clients[fd].cmd.size() != 1)
    {
        throw std::runtime_error("unvalid command");
    }
    cache.Flush();
}
