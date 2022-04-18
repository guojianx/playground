#include "tcp.h"
#include <iostream>


int main(int argc, char **argv)
{
    TcpContext tcp(0);
    tcp.Open("192.168.72.128", "21111");
    return 0;
}
