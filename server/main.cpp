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

    m_mu.lock();
    m_clDb[ip] = cl;
    m_mu.unlock();
}

std::vector<std::string> Db::getAliveClient() {
    std::vector<std::string> clList;

    m_mu.lock();
    for(auto const &entry : m_clDb) {
        if (entry.second.isAlive == true) {
            clList.push_back(entry.first);
        }
    }
    m_mu.unlock();

    return clList;
}

std::map<std::string, std::string> Db::getAliveClientFrom(int hours) {
    std::map<std::string, std::string> clMap;

    m_mu.lock();
    for(auto const &entry : m_clDb) {
        if (entry.second.isAlive == true) {
            // TODO add more condition
            std::time_t now = std::chrono::system_clock::to_time_t(entry.second.startLive);
            std::string nowStr(std::ctime(&now));
            clMap[entry.first] = nowStr;
        }
    }
    m_mu.unlock();

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

int main() {
    std::cout << "Seeder listen command  on port 9090" << std::endl;
    CmdHdl::init();
    CmdHdl::run();
}
