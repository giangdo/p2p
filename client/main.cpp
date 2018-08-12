/**
    C++ client example using sockets
*/
#include<iostream>    //cout
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<string>  //string
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<netdb.h> //hostent
#include<getopt.h>
#include "json.hpp"
#include "main.h"

using namespace std;
using Json = nlohmann::json;

Arg::Arg(int argc, char** argv) {
    if (argc < 7) {
        printHelp();
    }

    const char* const short_opts = "i:p:c:d:h:";
    const option long_opts[] = {
            {"ip"    , required_argument , nullptr , 'i'} ,
            {"port"  , required_argument , nullptr , 'p'} ,
            {"cmd"   , required_argument , nullptr , 'c'} ,
            {"day"   , required_argument , nullptr , 'd'} ,
            {"hour"  , required_argument , nullptr , 'h'} ,
            {nullptr , no_argument       , nullptr ,  0}
    };

    while (true) {
        const auto opt = getopt_long(argc, argv, short_opts, long_opts, nullptr);
        if (-1 == opt)
            break;
        switch (opt)
        {
        case 'i':
            m_ip = std::string(optarg);
            break;
        case 'p':
            m_port = std::stoi(optarg);
            break;
        case 'c':
            m_cmd = std::string(optarg);
            break;
        case 'd':
            m_day = std::stoi(optarg);
            break;
        case 'h':
            m_hour = std::stoi(optarg);
            break;
        case '?':
        default:
            printHelp();
            break;
        }
    }
}

void Arg::printHelp() {
    std::cout <<
            "--ip<i>:    Ip address of server\n"
            "--port<p>:  Port of server\n"
            "--cmd<c>:   Command: [hello | list | query]\n"
            "--day<d>:   Number of days for \"query\" command\n"
            "--hour<h>:  Number of hours for \"query\" command\n"
            "EX: ./client -i 172.18.0.2 -p 9090 -c hello\n"
            "EX: ./client -i 172.18.0.2 -p 9090 -c list\n"
            "EX: ./client -i 172.18.0.2 -p 9090 -c query -d 1 -h 1\n";

    exit(1);
}

std::string Arg::getIp() {
    return m_ip;
}

int Arg::getPort() {
    return m_port;
}

std::string Arg::getCmd() {
    return m_cmd;
}

int Arg::getDay() {
    return m_day;
}

int Arg::getHour() {
    return m_hour;
}

Client::Client()
{
    m_sock = -1;
}

/**
    Connect to a host on a certain port number
*/
bool Client::conn(string address , int port)
{
    //create socket if it is not already created
    if(m_sock == -1)
    {
        //Create socket
        m_sock = socket(AF_INET , SOCK_STREAM , 0);
        if (m_sock == -1)
        {
            perror("Could not create socket");
        }

        cout<<"Socket created\n";
    }
    else    {   /* OK , nothing */  }

    //setup address structure
    int result = inet_addr(address.c_str());

    struct sockaddr_in server;
    if (result == -1)
    {
        struct hostent *he;
        struct in_addr **addr_list;

        //resolve the hostname, its not an ip address
        if ( (he = gethostbyname( address.c_str() ) ) == NULL)
        {
            //gethostbyname failed
            herror("gethostbyname");
            cout<<"Failed to resolve hostname\n";

            return false;
        }

        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;

        for(int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];

            cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<endl;

            break;
        }
    }

    //plain ip address
    else
    {
        server.sin_addr.s_addr = inet_addr( address.c_str() );
    }

    server.sin_family = AF_INET;
    server.sin_port = htons( port );

    //Connect to remote server
    if (connect(m_sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    cout<<"Connected\n";
    return true;
}

/**
    Send data to the connected host
*/
bool Client::sendData(string data)
{
    //Send some data
    if( send(m_sock , data.c_str() , strlen( data.c_str() ) , 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    cout<<"Data send\n";

    return true;
}

/**
    Receive data from the connected host
*/
string Client::recvData(int size=512)
{
    char buffer[size];
    string reply;

    //Receive a reply from the server
    if( recv(m_sock , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("recv failed");
    }

    reply = buffer;
    return reply;
}

int main(int argc , char *argv[])
{
    Arg arg(argc, argv);
    std::cout << "ip: " << arg.getIp() << " port: " << arg.getPort() << " cmd: " << arg.getCmd() << std::endl;
    std::cout << "day: " << arg.getDay() << " hour: " << arg.getHour() << std::endl;
    Client cl;

    //connect to host
    cl.conn(arg.getIp(), arg.getPort());

    //send some data
    Json j = {
        {"cmd", arg.getCmd()},
        {"day", arg.getDay()},
        {"hour", arg.getHour()},
    };

    std::string msg = j.dump(4);

    cout << msg << std::endl;
    cl.sendData(msg);

    //receive and echo reply
    cout<<"----------------------------\n\n";
    cout<<cl.recvData(1024);
    cout<<"\n\n----------------------------\n\n";

    //done
    return 0;
}
