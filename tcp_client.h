#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <netinet/in.h>
#include <sstream>

#define BUFFER_SIZE 1024

class TCP_client{

    char *IPAddress;
    char readBuf[BUFFER_SIZE] = {0};
    struct sockaddr_in serv_addr;
    int sock;

    public:
        explicit TCP_client();
        ~TCP_client();
        uint16_t getBufferSize();
        uint16_t ConnectToHost(const char *, const uint16_t);
        uint16_t SendMessage(const std::stringstream &);
};

#endif // TCP_CLIENT_H
