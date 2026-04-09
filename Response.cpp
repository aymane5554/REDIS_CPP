#include "Server.hpp"

void make_client_readable(int fd,  int epoll_fd, Client &cl)
{
    epoll_event event;
    memset(&event, 0, sizeof(event));
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
    event.data.fd = fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &event);
    cl.clear();
}

void Server::send_response(int fd)
{
    Client &cl = clients[fd];
    std::vector <str> &cmd = cl.cmd;

    if (cl.send != cl.sent)
    {
        cl.send = cl.res_buff.length() - cl.sent;
        cl.sent = send(fd, cl.res_buff.c_str() + cl.sent, cl.send, MSG_NOSIGNAL);
        if (cl.send == cl.sent)
            make_client_readable(fd, epoll_fd, cl);
        return ;
    }
    if (cmd_func.find(cmd[0]) != cmd_func.end())
    {
        (this->*cmd_func[cmd[0]])(fd);
    }
    else
    {
        throw ERROR("-ERR Unsupported Command\r\n");
    }
    if (cl.send == cl.sent)
    {
        make_client_readable(fd, epoll_fd, cl);
    }
}
