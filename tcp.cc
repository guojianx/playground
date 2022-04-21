#include "tcp.h"

#include <string>
#include <iostream>

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>


static int NetError(void)
{
    std::string msg = strerror(errno);
    std::cout << "Net Error: " << msg << std::endl;
    return errno;
}

static int NetError(int err_code)
{
    std::string msg = gai_strerror(err_code);
    std::cout << "Net Error: " << msg << std::endl;
    return err_code;
}

static void NetShowInfo(const struct addrinfo &ai)
{
    char ip[16] {};
    uint16_t port = 0;
    struct sockaddr_in *sin = nullptr;
    const struct addrinfo *ai_iterator = &ai;

    while (ai_iterator) {
        if (ai_iterator->ai_family == AF_INET)

            std::cout << "ai_family   : AF_INET (IPv4)" << std::endl;
        else if (ai_iterator->ai_family == AF_INET6)
            std::cout << "ai_family   : AF_INET (IPv6)" << std::endl;
        else
            std::cout << "ai_family   : Unknown" << std::endl;

        if (ai_iterator->ai_socktype == SOCK_STREAM)
            std::cout << "ai_socktype : SOCK_STREAM (tcp)" << std::endl;
        else
            std::cout << "ai_socktype : wrong type" << std::endl;

        std::cout << "ai_protocol : " << ai_iterator->ai_protocol
                  << " (/etc/protocols: tcp = 6)"  << std::endl;

        sin = (struct sockaddr_in*)ai_iterator->ai_addr;
        inet_ntop(ai_iterator->ai_family, &sin->sin_addr, ip, sizeof(ip));
        port = htons(sin->sin_port);
        std::cout << "ai_addr.ip  : " << ip << std::endl;
        std::cout << "ai_addr.port: " << port << std::endl;
        std::cout << "======================================" << std::endl;

        ai_iterator = ai_iterator->ai_next;
    }
    return;
}

static int NetListenBind(int &sockfd, const struct addrinfo *ai, int backlog)
{
    int ret;

    ret = bind(sockfd, ai->ai_addr, ai->ai_addrlen);
    if (ret)
        return NetError();

    ret = listen(sockfd, backlog);
    if (ret)
        return NetError();

    ret = accept(sockfd, nullptr, nullptr);
    if (ret < 0)
        return NetError();

    close(sockfd);
    sockfd = ret;
    return 0;
}

static int NetConnect(int sockfd, const struct addrinfo *ai)
{
    //fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFL) | O_NONBLOCK);  // nonblock

    if (connect(sockfd, ai->ai_addr, ai->ai_addrlen))
        return NetError();
    else
        return 0;
}

TcpContext::TcpContext(int listen) :
    _fd(-1), _listen(listen)
{ }

TcpContext::~TcpContext()
{
    Close();
}

int TcpContext::Close()
{
    if (_fd > 0) {
        close(_fd);
        _fd = -1;
    }

    return 0;
}

int TcpContext::Read(uint8_t *buf, size_t size)
{
    int ret;
    /* TODO: poll() wait for readable. */
    
    ret = recv(_fd, buf, size, 0);
    return ret < 0 ? NetError() : ret;
}

int TcpContext::Write(uint8_t *buf, size_t size)
{
    int ret;
    /* TODO: poll() wait for writable. */
    ret = send(_fd, buf, size, MSG_NOSIGNAL);
    return ret < 0 ? NetError() : ret;
}

int TcpContext::Open(const std::string &ip, const std::string &port)
{
    struct addrinfo hints {};
    struct addrinfo *ai = nullptr;
    int ret = -1;
    int sockfd = -1;

    if (port.empty()) {
        std::cout << "TcpContext::Open(" << port << "): "
            "Invalid Argument - Have to spcify a valid port." << std::endl;
        return -1;
    }

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags += AI_PASSIVE;
    ret = getaddrinfo(ip.empty() ? nullptr : ip.c_str(), port.c_str(), &hints, &ai);
    if (ret) {
        ret = NetError(ret);
        goto fail;
    }

    NetShowInfo(*ai);

    sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd == -1) {
        ret = NetError();
        goto fail;
    }

    if (_listen > 0) {
        /**
         * `_listen` defines the maximum length to which
         * the queue of pending connections for sockfd may grow.
         * If a connection request arrives when the queue is full,
         * the client may receive an error with an indication of ECONNREFUSED. 
         */
        /* this mutates the sockfd, it will be the connection fd */
        ret = NetListenBind(sockfd, ai, _listen);
        if (ret)
            goto fail;    
    }
    else {
        ret = NetConnect(sockfd, ai);
        if (ret)
            goto fail;
    }

    freeaddrinfo(ai);
    _fd = sockfd;
    return 0;

fail:
    freeaddrinfo(ai);
    close(sockfd);
    return ret;
}
