#include "Server.hpp"

void lines(str &req_buff, std::vector<str> &cmd, int &lines, int &bytes)
{
    str line;
    size_t indx;
    size_t end;
    size_t start = 0;


    if (lines == 0)
    {
        indx = req_buff.find("\r\n", start); // what if he sent 1 then waited the sent 0
        if (req_buff[0] != '*')
            throw std::runtime_error("bad request");
        try
        {
            lines = std::stoi(req_buff.substr(1), &end);
            if (lines < 0 || req_buff.substr(end + 1) != "\r\n")
                throw std::runtime_error("bad request");
        }
        catch(const std::exception& e)
        {
            throw std::runtime_error("bad request");
        }
        start = indx + 2;
        indx = req_buff.find("\r\n", start);
    }
    indx = req_buff.find("\r\n", start);
    while (indx != str::npos)
    {
        line = req_buff.substr(start, indx);
        if (line[0] != '$')
        {
            if ((int)line.size() != bytes)
                throw std::runtime_error("bad request");
            cmd.push_back(line);
            bytes = -1;
        }
        else
        {
            bytes = std::stoi(line.substr(1), &end);
            if (bytes < 0 || line[end + 1])
                throw std::runtime_error("bad request");
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
    if (len <= 0)
    {
        safe_close(fd);
        return ;
    }
    client.req_buff.append(buff, len);
    lines(client.req_buff, client.cmd, client.lines, client.bytes);
    client.req_buff.clear();
    if (client.lines == (int)client.cmd.size())
        make_client_writable(fd, epoll_fd);
}
