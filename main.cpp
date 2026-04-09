#include "Server.hpp"

std::atomic<bool> sigint = 0;

int main()
{
    Server server;

    signal(SIGPIPE, SIG_IGN);
    server.run();
    return 0;
}