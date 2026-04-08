#include "Server.hpp"

int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void Server::safe_close(int fd)
{
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    clients.erase(fd);
    /*i will keep this here cause i want to add keep-alive later
    clients[fd].res_buff.clear();
    clients[fd].cmd.clear();
    clients[fd].clear()
    */
}

void sigint_handler(int sig)
{
    (void)sig;
    sigint = 1;
}

void Server::ttl_thread()
{
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    while (!sigint)
    {
        sleep(TTL_SLEEP_TIME);
        lock.lock();
        cache.check_expired_values();
        lock.unlock();
    }
}

void Server::run()
{
    std::thread ttl(&Server::ttl_thread, this);

    struct epoll_event events[MAX_EVENTS];
    signal(SIGINT, sigint_handler);
    while (!sigint)
    {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, 1000);
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;
            try
            {
                if (fd == server_fd)
                {
                    struct sockaddr_in client_addr;
                    socklen_t len = sizeof(client_addr);
                    int conn_fd = accept(server_fd, (struct sockaddr*)&client_addr, &len);
                    if (conn_fd == -1) {
                        perror("accept");
                        continue;
                    }
                    struct   epoll_event client_event;
                    set_nonblocking(conn_fd);
                    client_event.data.fd = conn_fd;
                    client_event.events = EPOLLIN | EPOLLHUP | EPOLLERR;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, conn_fd, &client_event);
                    Client obj;
                    clients.insert(std::make_pair(conn_fd, obj));
                }
                else if (events[i].events & EPOLLIN)
                {
                    parse_request(fd);
                    continue ;
                }
                else if (events[i].events & EPOLLOUT)
                {
                    send_response(fd);
                    continue;
                }
                else if (events[i].events & (EPOLLHUP | EPOLLERR))
                {
                    safe_close(fd);
                    continue;
                }
            }
            catch (ERROR &e)
            {
                clients[fd].res_buff = e.what();
                clients[fd].send = clients[fd].res_buff.length();
                clients[fd].sent = send(fd, clients[fd].res_buff.c_str(), clients[fd].send, MSG_NOSIGNAL);
                if (clients[fd].send == clients[fd].sent)
                    safe_close(fd);
                continue ;
            }
            catch (std::exception &e)
            {
                std::cerr << "ERROR " << e.what() << std::endl;
                safe_close(fd);
            }
        }
    }
    ttl.detach();
};

Server::Server()
{
    struct epoll_event ev;
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

    cmd_func.insert(std::make_pair("GET", &Server::Get));
    cmd_func.insert(std::make_pair("SET", &Server::Set));
    cmd_func.insert(std::make_pair("DEL", &Server::Del));
    cmd_func.insert(std::make_pair("EXISTS", &Server::Exists));
    cmd_func.insert(std::make_pair("EXPIRE", &Server::Expire));
    cmd_func.insert(std::make_pair("TTL", &Server::Ttl));
    cmd_func.insert(std::make_pair("FLUSH", &Server::Flush));
}

Server::~Server()
{
    std::cout << "Server Shutdown..." << std::endl;
}
