#include "Server.hpp"

void Server::send_response(int fd)
{
    Client &cl = clients[fd];
    std::vector <str> &cmd = cl.cmd;

    if (cl.send != cl.sent)
    {
        cl.send = cl.res_buff.length() - cl.sent;
        cl.sent = send(fd, cl.res_buff.c_str() + cl.sent, cl.send, MSG_NOSIGNAL);
    }
    if (cmd_func.find(cmd[0]) != cmd_func.end())
    {
        (this->*cmd_func[cmd[0]])(fd);
    }
    else
    {
        throw std::runtime_error("-ERR Unsupported Command\r\n");
    }
    if (cl.send == cl.sent)
        safe_close(fd);
}
