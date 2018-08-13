#include<iostream>    //cout
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "json.hpp"
#include "main.h"

using Json = nlohmann::json;

Db dataBase;

void Db::insertClient(std::string ip) {
    ClStat cl;
    cl.isAlive = true;
    cl.startLive = std::chrono::system_clock::now();

    std::lock_guard<std::mutex> guard(m_mu);
    m_clDb[ip] = cl;
}

void Db::update(std::string ip, bool status) {
    std::lock_guard<std::mutex> guard(m_mu);

    ClStat cl = m_clDb[ip];
    if ((cl.isAlive == true) && (status == true)) {
    } else {
        cl.isAlive = status;
        cl.startLive = std::chrono::system_clock::now();
    }
    m_clDb[ip] = cl;
}

std::vector<std::string> Db::getAllClient()
{
    std::vector<std::string> clList;
    std::lock_guard<std::mutex> guard(m_mu);
    for(auto const &entry : m_clDb) {
        clList.push_back(entry.first);
    }

    return clList;
}

std::vector<std::string> Db::getAliveClient() {
    std::vector<std::string> clList;

    std::lock_guard<std::mutex> guard(m_mu);
    for(auto const &entry : m_clDb) {
        if (entry.second.isAlive == true) {
            clList.push_back(entry.first);
        }
    }

    return clList;
}

std::map<std::string, std::string> Db::getAliveClientFrom(int hours) {
    std::map<std::string, std::string> clMap;

    std::lock_guard<std::mutex> guard(m_mu);
    for(auto const &entry : m_clDb) {
        // Get the time when this node is started alive
        std::time_t st = std::chrono::system_clock::to_time_t(entry.second.startLive);

        // Get the time satisfied for query command
        std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
        std::time_t now_c = std::chrono::system_clock::to_time_t(now - std::chrono::hours(hours));

        double d = std::difftime(st, now_c);
        //std::cout << "diff :" << d << std::endl;
        if ((entry.second.isAlive == true) && (d > 0)) {
            // TODO add more condition
            std::string nowStr(std::ctime(&st));
            clMap[entry.first] = nowStr;
        }
    }

    return clMap;
}

uv_loop_t* CmdHdl::m_loop = nullptr;
uv_tcp_t CmdHdl::m_server;

void CmdHdl::free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void CmdHdl::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void CmdHdl::echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

std::string CmdHdl::getClientIP(uv_stream_t *client) {
    struct sockaddr_storage peername;
    int namelen = sizeof(peername);
    int rc = uv_tcp_getpeername((uv_tcp_t*)client, (struct sockaddr*)&peername, &namelen);
    if (rc < 0) {
        std::cout << "Can not get peer name " << uv_strerror(rc) << std::endl;
    }
    char hostbuf[NI_MAXHOST], portbuf[NI_MAXSERV];
    char ipStr[INET_ADDRSTRLEN];
    if (getnameinfo((struct sockaddr*)&peername, namelen , hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, 0) == 0) {
        struct sockaddr_in *s = (struct sockaddr_in *)&peername;
        inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof(ipStr));
        std::cout << "hostname: " << hostbuf << " ip: " << ipStr << " port:" << portbuf << std::endl;
    } else {
        std::cout << "peer (unknonwn hostname) connected" << std::endl;
    }
    return std::string(ipStr);
}

void CmdHdl::echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));

        // Get message
        std::string rcvStr = std::string(buf->base);
        std::cout << rcvStr << std::endl;
        auto j = Json::parse(rcvStr);
        std::string cmd = j["cmd"].get<std::string>();
        std::string helloStr("hello");
        std::string listStr("list");
        std::string queryStr("query");

        // Prepare message to send back
        Json jSend;
        if (cmd == helloStr) {
            jSend["response"] = "ok";
            std::string ipStr = getClientIP(client);
            jSend["ip"] = ipStr;
            dataBase.insertClient(ipStr);
        }
        else if (cmd == listStr) {
            std::vector<std::string> clList = dataBase.getAliveClient();
            for(auto const &entry : clList) {
                jSend.push_back(entry);
            }
        }
        else if (cmd == queryStr) {
            int day = j["day"].get<int>();
            int hour = j["hour"].get<int>();
            std::map<std::string, std::string> clMap = dataBase.getAliveClientFrom(day * 24 + hour);
            Json j(clMap);
            jSend = j;
        }

        // Prepare buffer to send back
        std::string strSend = jSend.dump(2);
        int len = strlen(strSend.c_str()) + 1;
        char* bufSend = (char*)malloc(len);
        memset(bufSend, 0, len);
        strncpy(bufSend, strSend.c_str(), len);

        req->buf = uv_buf_init(bufSend, len);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void CmdHdl::on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(m_loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void CmdHdl::init() {
    m_loop = uv_default_loop();
    uv_tcp_init(m_loop, &m_server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 9090, &addr);

    uv_tcp_bind(&m_server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &m_server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        exit(1);
    }
}

void CmdHdl::run(){
    int re = uv_run(m_loop, UV_RUN_DEFAULT);
    if (re) {
        fprintf(stderr, "run error %s\n", uv_strerror(re));
    }
}


Client::Client()
{
    m_sock = -1;
}

/**
    Connect to a host on a certain port number
*/
bool Client::conn(std::string address , int port)
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

        std::cout<<"Socket created\n";
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
            std::cout<<"Failed to resolve hostname\n";

            return false;
        }

        //Cast the h_addr_list to in_addr , since h_addr_list also has the ip address in long format only
        addr_list = (struct in_addr **) he->h_addr_list;

        for(int i = 0; addr_list[i] != NULL; i++)
        {
            //strcpy(ip , inet_ntoa(*addr_list[i]) );
            server.sin_addr = *addr_list[i];

            std::cout<<address<<" resolved to "<<inet_ntoa(*addr_list[i])<<std::endl;

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
        return false;
    }

    std::cout<<"Connected\n";
    return true;
}

/**
    Send data to the connected host
*/
bool Client::sendData(std::string data)
{
    //Send some data
    char buf[512];
    memset(buf, 0, sizeof(buf));
    int len = strlen(data.c_str());
    strncpy(buf, data.c_str(), len);
    if( send(m_sock , buf, len + 1, 0) < 0)
    {
        perror("Send failed : ");
        return false;
    }
    std::cout<<"Data send\n";

    return true;
}

/**
    Receive data from the connected host
*/
std::string Client::recvData(int size=512)
{
    char buffer[size];
    std::string reply;

    //Receive a reply from the server
    if( recv(m_sock , buffer , sizeof(buffer) , 0) < 0)
    {
        puts("recv failed");
    }

    reply = buffer;
    return reply;
}

void pingFrq() {
    while (1) {
        std::this_thread::sleep_for(std::chrono::microseconds(1000*1000*20));
        // Copy out the list of client first
        std::vector<std::string> allCls = dataBase.getAllClient();

        for (auto const &entry : allCls) {
            Client cl;
            if (!cl.conn(entry, 8080)) {
                dataBase.update(entry, false);
                continue;
            }

            Json jSend = {{"cmd", "ping"}};
            std::string msg =  jSend.dump(2);
            if (!cl.sendData(msg)) {
                dataBase.update(entry, false);
                continue;
            }

            std::string recvMsg = cl.recvData(1024);
            auto j = Json::parse(recvMsg);
            std::string rsp = j["response"].get<std::string>();
            std::string okStr("ok");
            if (rsp == okStr) {
                dataBase.update(entry, true);
            } else {
                dataBase.update(entry, false);
            }
        }
    }
}

int main() {
    std::cout << "Seeder listen command  on port 9090" << std::endl;
    CmdHdl::init();
    std::thread t1(CmdHdl::run);
    std::thread t2(pingFrq);
    t1.join();
    t2.join();
}
