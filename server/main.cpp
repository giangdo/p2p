#include<iostream>    //cout
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
#include "json.hpp"

using Json = nlohmann::json;

#define DEFAULT_PORT 9090
#define DEFAULT_BACKLOG 128

uv_loop_t *loop;
struct sockaddr_in addr;


typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        std::string rcvStr = std::string(buf->base);
        std::cout << rcvStr << std::endl;
        auto j = Json::parse(rcvStr);
        std::string cmd = j["cmd"].get<std::string>();
        int day = j["day"].get<int>();
        int hour = j["hour"].get<int>();
        std::cout << cmd << day << hour << std::endl;
        Json jSend;
        jSend.push_back("foo0");
        jSend.push_back("foo1");
        jSend.push_back("foo2");
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

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        struct sockaddr_storage peername;
        int namelen = sizeof(peername);
        int rc = uv_tcp_getpeername(client, (struct sockaddr*)&peername, &namelen);
        if (rc < 0) {
            std::cout << "Can not get peer name " << uv_strerror(rc) << std::endl;
        }
        char hostbuf[NI_MAXHOST], portbuf[NI_MAXSERV];
        if (getnameinfo((struct sockaddr*)&peername, namelen , hostbuf, NI_MAXHOST, portbuf, NI_MAXSERV, 0) == 0) {
            char ipStr[INET_ADDRSTRLEN];
            struct sockaddr_in *s = (struct sockaddr_in *)&peername;
            inet_ntop(AF_INET, &s->sin_addr, ipStr, sizeof(ipStr));
            std::cout << "hostname: " << hostbuf << " ip: " << ipStr << " port:" << portbuf << std::endl;
        } else {
            std::cout << "peer (unknonwn hostname) connected" << std::endl;
        }


        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

int main() {
    loop = uv_default_loop();

    uv_tcp_t server;
    uv_tcp_init(loop, &server);

    uv_ip4_addr("0.0.0.0", DEFAULT_PORT, &addr);
    std::cout << "Start server at port" << std::to_string(DEFAULT_PORT) << std::endl;

    uv_tcp_bind(&server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &server, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        return 1;
    }
    int re = uv_run(loop, UV_RUN_DEFAULT);
    return re;
}
