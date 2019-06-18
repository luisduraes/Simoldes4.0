#include "tcp_client.h"
#include <QDebug>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>

TCP_client::TCP_client(){}

TCP_client::~TCP_client(){}

uint16_t TCP_client::ConnectToHost(const char *inIPAddress, const uint16_t inPort){

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        qDebug() << "\n Socket creation error \n";
        return -1;
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(inPort);

    if(inet_pton(AF_INET, inIPAddress, &serv_addr.sin_addr) <= 0){
        qDebug() << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        qDebug() << "\nConnection Failed \n";
        return -1;
    }

    return 0;
}

uint16_t TCP_client::getBufferSize(){

    return BUFFER_SIZE;
}

uint16_t TCP_client::SendMessage(const std::stringstream &ssOutput){

    std::string sOutput = ssOutput.str();
    const char *cChar = sOutput.c_str();
    qDebug() << cChar;

    if(send(sock, cChar, strlen(cChar), 0) != strlen(cChar)){
        qDebug() << "TCP ERROR: Size of data does not match !";
        return -1;
    }

    if(shutdown(sock, SHUT_WR) < 0){
        qDebug() << "TCP ERROR: Socket-shutdown not completed !";
        return -1;
    }

    if(recv(sock, readBuf, BUFFER_SIZE, 0) == -1){
        qDebug() << "TCP ERROR: Fail to read from " << sock << "!\n";
        return -1;
    }

    qDebug() << readBuf;
    close(sock);

    return 0;
}
