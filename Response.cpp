#include "Server.hpp"

void Server::send_response(int fd)
{
    std::vector <str> &cmd = clients[fd].cmd;

    if (cmd_func.find(cmd[0]) != cmd_func.end())
    {
        (this->*cmd_func[cmd[0]])();
    }
    else
    {
        throw std::runtime_error("Unsupported Command");
    }
    safe_close(fd);
}
