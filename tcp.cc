#include "tcp.h"

TcpContext::TcpContext(int listen) :
    _sockfd(-1), _connfd(-1), _listen(listen), _hostname("")
{ }

int TcpContext::TcpOpen(const std::string &ip, const std::string &port)
{
    return 0;
}
