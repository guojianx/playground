#include <iostream>
#include <string>

#include "tcp.h"

int main(int argc, char **argv)
{
    TcpContext tcp(0);
    tcp.TcpOpen("", "21111");
    return 0;
}
