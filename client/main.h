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
#include <uv.h>

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
        int m_hour = 1;

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

class PingHdl
{
    public:
        static void init();
        static void run();
    private:
        static uv_loop_t *m_loop;
        static uv_tcp_t m_server;

        typedef struct {
            uv_write_t req;
            uv_buf_t buf;
        } write_req_t;

        static void on_new_connection(uv_stream_t *server, int status);
        static void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
        static void echo_write(uv_write_t *req, int status);
        static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
        static void free_write_req(uv_write_t *req);
};
#endif /* MAIN_H_ */
