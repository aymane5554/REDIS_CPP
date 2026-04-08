#include "Server.hpp"

std::atomic<bool> sigint = 0;

int main()
{
    Server server;

    server.run();
    return 0;
}