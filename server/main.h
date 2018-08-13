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
#include <map>
#include <ctime>
#include <chrono>
#include <vector>
#include <thread>         // std::thread
#include <mutex>          // std::mutex

typedef struct {
    bool isAlive;
    std::chrono::system_clock::time_point startLive;
}ClStat;

class Db {
    public:
        void insertClient(std::string ip);
        void update(std::string ip, bool status);

        std::vector<std::string> getAllClient();
        std::vector<std::string> getAliveClient();
        std::map<std::string, std::string> getAliveClientFrom(int hours);
    private:
        std::map<std::string, ClStat> m_clDb;
        std::mutex m_mu;
};

class CmdHdl
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

        static std::string getClientIP(uv_stream_t *client);
        static void on_new_connection(uv_stream_t *server, int status);
        static void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf);
        static void echo_write(uv_write_t *req, int status);
        static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
        static void free_write_req(uv_write_t *req);
};

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
