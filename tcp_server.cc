#include <iostream>
#include <string>

#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int NetError(void)
{
    std::string msg = strerror(errno);
    std::cout << "Net Error: " << msg << std::endl;
    return errno;
}

int NetError(int err_code)
{
    std::string msg = gai_strerror(err_code);
    std::cout << "Net Error: " << msg << std::endl;
    return err_code;
}

void NetShowInfo(const struct addrinfo &ai)
{
    char ip[16] {};
    uint16_t port = 0;
    struct sockaddr_in *sin = nullptr;

    if (ai.ai_family == AF_INET)
        std::cout << "ai_family: AF_INET (IPv4)" << std::endl;
    else if (ai.ai_family == AF_INET6)
        std::cout << "ai_family: AF_INET (IPv6)" << std::endl;
    else
        std::cout << "ai_family: Unknown" << std::endl;

    if (ai.ai_socktype == SOCK_STREAM)
        std::cout << "ai_socktype: SOCK_STREAM (tcp)" << std::endl;
    else
        std::cout << "ai_socktype: wrong type" << std::endl;

    std::cout <<  "ai_protocol: " << ai.ai_protocol << std::endl;

    sin = (struct sockaddr_in*)ai.ai_addr;
    inet_ntop(ai.ai_family, &sin->sin_addr, ip, sizeof(ip));
    port = htons(sin->sin_port);
    std::cout << "ai_addr.ip  : " << ip << std::endl;
    std::cout << "ai_addr.port: " << port << std::endl;
    return;
}

int main(int argc, char **argv)
{
    struct addrinfo hints {};
    struct addrinfo *ai = nullptr;
    int ret = -1;
    int sockfd = -1;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags += AI_PASSIVE;
    ret = getaddrinfo(nullptr, "21111", &hints, &ai);
    if (ret) {
        ret = NetError(ret);
        goto fail;
    }

    NetShowInfo(*ai);

    /**
     * domain   = AF_INET     for IPv4
     * type     = SOCK_STREAM for TCP
     * protocol = 0
     */
    sockfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);
    if (sockfd == -1) {
        ret = NetError();
        goto fail;
    }

    /*
    ret = bind(sockfd, addr, addrlen);
    if (ret) {
        ret = NetError();
        goto fail;
    }
    */

fail:
    return ret;
}
