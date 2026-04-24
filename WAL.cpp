#include "Server.hpp"

typedef enum e_cmd
{
    SET,
    DEL,
    LPUSH,
    RPUSH,
    LPOP,
    RPOP,
    Expire,
    FLUSH
} t_wcmd; // wal command

int read_wal_cmd(std::ifstream &file, int &end)
{
    str line;

    std::getline(file, line);
}

int Server::read_wal()
{
    std::ifstream wal_file(WAL_FILE);
    int end = 0;

    if (wal_file.fail())
        return -1;
    while (!end)
    {
        
    }
    return 0;
}

int Server::Wal(std::vector <str> &cmd)
{
    str lines;
    int fd = open(WAL_FILE, O_CREAT | O_APPEND | O_WRONLY, 0777);
    if (fd == -1)
        return -1;
    lines = std::to_string(cmd.size());
    write(fd, lines.c_str(), lines.size());
    for (size_t i = 0; i < cmd.size(); i++)
    {
        write(fd, cmd[i].c_str(), cmd[i].length());
        write(fd, "\r\n", 2);
    }
    fsync(fd);
    close(fd);
    return 0;
}
