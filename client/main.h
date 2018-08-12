#ifndef MAIN_H_
#define MAIN_H_

#include<iostream>    //cout
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent
#include<getopt.h>

class Arg {
    public:
        Arg(int argc, char** argv);
        std::string getIp();
        int getPort();
        std::string getCmd();
        int getDay();
        int getHour();

    private:
        std::string m_ip;
        int m_port;
        std::string m_cmd; // can be hello, list, query
        int m_day = 0;
        int m_hour = 0;

        void printHelp();
};

/**
    TCP Client class
*/
class Client
{
private:
    int m_sock;

public:
    Client();
    bool conn(std::string, int);
    bool sendData(std::string data);
    std::string recvData(int);
};
#endif /* MAIN_H_ */
