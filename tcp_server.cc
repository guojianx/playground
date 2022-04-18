#include <iostream>
#include <string>

#include "tcp.h"

int main(int argc, char **argv)
{
    TcpContext tcp(0);
    tcp.TcpOpen("172.16.11.237", "21111");
    return 0;
}
