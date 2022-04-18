#include <iostream>
#include <string>

#include "tcp.h"

int main(int argc, char **argv)
{
    TcpContext tcp(1);
    tcp.Open("", "21111");
    return 0;
}
