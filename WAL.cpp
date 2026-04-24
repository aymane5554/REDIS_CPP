#include "Server.hpp"

int Server::Wal(std::vector <str> &cmd)
{
    str lines;
    int fd = open(WAL_FILE, O_CREAT | O_APPEND | O_WRONLY, 0777);
    if (fd == -1)
        return -1;
    lines = std::to_string(cmd.size());
    write(fd, lines.c_str(), lines.size());
    write(fd, "\r\n", 2);
    for (size_t i = 0; i < cmd.size(); i++)
    {
        write(fd, cmd[i].c_str(), cmd[i].length());
        write(fd, "\r\n", 2);
    }
    fsync(fd);
    close(fd);
    return 0;
}

void Server::read_wal()
{
    char buff[BUF_SIZE];
    str wal_content;
    std::vector<str> tokens;
    size_t start = 0;
    size_t end;
    ssize_t r;
    int fd = open(WAL_FILE, O_RDONLY);

    std::cout << "Reading WAL..." << std::endl;
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            std::cout << "WAL file not found" << std::endl;
            return ;
        }
        throw std::runtime_error("Failed to open WAL file");
    }
    r = read(fd, buff, sizeof(buff));
    while (r > 0)
    {
        wal_content.append(buff, r);
        r = read(fd, buff, sizeof(buff));
    }
    close(fd);
    if (r < 0)
        throw std::runtime_error("Failed to read WAL file");
    while ((end = wal_content.find("\r\n", start)) != str::npos)
    {
        tokens.push_back(wal_content.substr(start, end - start));
        start = end + 2;
    }
    if (start != wal_content.size())
        throw std::runtime_error("Invalid WAL format");

    std::lock_guard<std::mutex> lock(mtx);
    for (size_t i = 0; i < tokens.size();)
    {
        size_t argc;
        str first_arg;
        std::vector<str> cmd;
        char *endptr;

        argc = std::strtoul(tokens[i].c_str(), &endptr, 10);
        if (*endptr != '\0' || argc == 0)
            throw std::runtime_error("Invalid WAL format");
        ++i;
        while (cmd.size() < argc && i < tokens.size())
        {
            cmd.push_back(tokens[i]);
            ++i;
        }
        if (cmd.size() != argc || cmd.empty())
            throw std::runtime_error("Invalid WAL format");
        if (cmd[0] == "SET" && cmd.size() == 3)
            cache.Set(cmd);
        else if (cmd[0] == "DEL" && cmd.size() == 2)
            cache.Del(cmd[1]);
        else if (cmd[0] == "LPUSH" && cmd.size() == 3)
            cache.Lpush(cmd);
        else if (cmd[0] == "RPUSH" && cmd.size() == 3)
            cache.Rpush(cmd);
        else if (cmd[0] == "LPOP" && cmd.size() == 2)
            cache.Lpop(cmd[1]);
        else if (cmd[0] == "RPOP" && cmd.size() == 2)
            cache.Rpop(cmd[1]);
        else if (cmd[0] == "FLUSH" && cmd.size() == 1)
            cache.Flush();
        else if (cmd[0] == "EXPIRE" && cmd.size() == 3)
        {
            size_t seconds = std::strtoull(cmd[2].c_str(), &endptr, 10);
            if (*endptr != '\0')
                throw std::runtime_error("Invalid WAL format");
            cache.Expire(cmd[1], seconds);
        }
        else
            throw std::runtime_error("Invalid WAL format");
    }
    std::cout << "WAL loaded successfully" << std::endl;
}
