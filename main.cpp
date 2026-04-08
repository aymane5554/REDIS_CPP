#include "Server.hpp"

int sigint = 0;


int main()
{
    Server server;
    
    server.run();
    return 0;
}