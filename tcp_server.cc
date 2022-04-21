#include <iostream>
#include <string>

#include "tcp.h"

int main(int argc, char **argv)
{
    int i, ret = 0;
    uint8_t buf[1024] {};
    TcpContext tcp(1);

    tcp.Open("", "21111");
    ret = tcp.Read(buf, sizeof(buf));
    std::cout << "recv return " << ret << std::endl;
    for (i = 0; i < ret; i++)
        printf("0x%hhx ", buf[i]);
    std::cout << std::dec << std::endl;
    
    return 0;
}
