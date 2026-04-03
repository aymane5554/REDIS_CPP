#include "Server.hpp"

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::run()
{
    while (1)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            if (fd == server_fd)
            {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
                if (conn_fd == -1) {
                    perror("accept");
                    continue;
                }
                set_nonblocking(conn_fd);
                ev.events  = EPOLLIN;
                ev.data.fd = conn_fd;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &ev);
            }
            else if (events[i].events & EPOLLIN)
            {
                // recv request
            }
            else if (events[i].events & EPOLLOUT)
            {
                // send response
            }
        }
    }
};

Server::Server()
{
    std::cout << "Server Init..." << std::endl;
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    struct sockaddr_in addr;
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(PORT);
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, SOMAXCONN);
    set_nonblocking(server_fd);

    std::cout << "Listening on port" << PORT << std::endl;
    epoll_fd = epoll_create1(0);
    ev.events  = EPOLLIN;
    ev.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);
}

Server::~Server()
{
    std::cout << "Server Shutdown..." << std::endl;
}

