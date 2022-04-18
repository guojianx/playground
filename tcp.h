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

    int Open(const std::string &hostname, const std::string &port);
    int Close(void);

private:
    int _fd;
    int _listen;
};

#endif
