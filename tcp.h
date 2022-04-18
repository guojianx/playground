#ifndef TCP_H
#define TCP_H

#include <string>

class TcpContext
{
public:
    TcpContext() = delete;

    /**
     * listen - Specify how many incoming connections listen for
     */
    TcpContext(int listen);

    virtual ~TcpContext();

    int TcpOpen(const std::string &hostname, const std::string &port);

private:
    int _sockfd;
    int _connfd;
    int _listen;
};

#endif
