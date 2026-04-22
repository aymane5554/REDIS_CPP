#include "Server.hpp"

void lines(str &req_buff, std::vector<str> &cmd, int &lines, int &bytes)
{
    str line;
    size_t indx;
    size_t end;
    size_t start = 0;


    if (lines == 0)
    {
        indx = req_buff.find("\r\n", start);
        if (req_buff[0] != '*')
            throw std::runtime_error("line length");
        try
        {
            lines = std::stoi(req_buff.substr(1), &end);
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error("parsing lines number");
        }
        if (lines < 0 || req_buff.substr(end + 1, 2) != "\r\n")
            throw std::runtime_error("line length unvalid number");
        start = indx + 2;
    }
    indx = req_buff.find("\r\n", start);
    while (indx != str::npos)
    {
        line = req_buff.substr(start, indx - start);
        if (line[0] != '$')
        {
            if ((int)line.size() != bytes)
                throw std::runtime_error("missing bytes length");
            cmd.push_back(line);
            bytes = -1;
        }
        else
        {
            try
            {
                bytes = std::stoi(line.substr(1), &end);
            }
            catch (const std::exception& e)
            {
                throw std::runtime_error("parsing bytes number");
            }
            if (bytes < 0 || line[end + 1])
                throw std::runtime_error("unvalid bytes number");
        }
        start = indx + 2;
        indx = req_buff.find("\r\n", start);
    }
}

void make_client_writable(int fd,  int epoll_fd)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLOUT | EPOLLHUP | EPOLLERR;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
}

void Server::parse_request(int fd)
{
    char buff[BUF_SIZE];
    ssize_t len;
    Client &client = clients[fd];

    len = recv(fd, buff, BUF_SIZE, MSG_NOSIGNAL);
    if (len == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
            safe_close(fd);
        return;
    }
    if (len == 0)
        return safe_close(fd);
    client.req_buff.append(buff, len);
    if (client.req_buff.find("\r\n") == str::npos)
        return;
    lines(client.req_buff, client.cmd, client.lines, client.bytes);
    client.req_buff = client.req_buff.substr(client.req_buff.find_last_of("\r\n") + 1);
    if (client.lines == (int)client.cmd.size())
        make_client_writable(fd, epoll_fd);
}
