#include "tcp.h"
#include <iostream>


int main(int argc, char **argv)
{
    uint8_t buf[10] = { 1, 2, 3, 4, 5, 6, 8, 7, 9 , 15};
    TcpContext tcp(0);

    if (argc < 2) {
        std::cout << "usage: " << argv[0] << " <ip>"  << std::endl;
        return -1;
    }


    tcp.Open(argv[1], "21111");
    tcp.Write(buf, sizeof(buf));
    return 0;
}
