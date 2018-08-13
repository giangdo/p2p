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
#include <uv.h>
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
        return false;
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
    char buf[512];
    memset(buf, 0, sizeof(buf));
    int len = strlen(data.c_str());
    strncpy(buf, data.c_str(), len);
    if( send(m_sock , buf, len + 1, 0) < 0)
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

uv_loop_t* PingHdl::m_loop = nullptr;
uv_tcp_t PingHdl::m_server;

void PingHdl::free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void PingHdl::alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void PingHdl::echo_write(uv_write_t *req, int status) {
    if (status) {
        fprintf(stderr, "Write error %s\n", uv_strerror(status));
    }
    free_write_req(req);
}

void PingHdl::echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));

        // Get message
        std::string rcvStr = std::string(buf->base);
        std::cout << rcvStr << std::endl;
        auto j = Json::parse(rcvStr);
        std::string cmd = j["cmd"].get<std::string>();

        // Prepare message to send back
        Json jSend;
        std::string pingStr("ping");
        if (cmd == pingStr) {
            jSend["response"] = "ok";
        } else {
            jSend["response"] = "notOK";
        }
        std::string strSend = jSend.dump(2);

        // Prepare buffer to send back
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

void PingHdl::on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error %s\n", uv_strerror(status));
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(m_loop, client);
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

void PingHdl::init() {
    m_loop = uv_default_loop();
    uv_tcp_init(m_loop, &m_server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", 8080, &addr);
    std::cout << "Start server at port" << std::to_string(8080) << std::endl;

    uv_tcp_bind(&m_server, (const struct sockaddr*)&addr, 0);
    int r = uv_listen((uv_stream_t*) &m_server, 128, on_new_connection);
    if (r) {
        fprintf(stderr, "Listen error %s\n", uv_strerror(r));
        exit(1);
    }
}

void PingHdl::run(){
    int re = uv_run(m_loop, UV_RUN_DEFAULT);
    if (re) {
        fprintf(stderr, "run error %s\n", uv_strerror(re));
    }
}

int main(int argc , char *argv[])
{
    Arg arg(argc, argv);
    std::cout << "ip: " << arg.getIp() << " port: " << arg.getPort() << " cmd: " << arg.getCmd() << std::endl;
    std::cout << "day: " << arg.getDay() << " hour: " << arg.getHour() << std::endl;
    Client cl;

    //connect to host
    if (!cl.conn(arg.getIp(), arg.getPort()))
    {
        exit(1);
    }

    // Prepare command to send to server
    Json j;
    if ((arg.getCmd() == "hello") || (arg.getCmd() == "list")) {
        j = {
            {"cmd", arg.getCmd()},
        };
    } else { // querry cmd
        j = {
            {"cmd", arg.getCmd()},
            {"day", arg.getDay()},
            {"hour", arg.getHour()},
        };
    }
    std::string msg =  j.dump(2);
    cout << msg << std::endl;

    // Send command to server
    cl.sendData(msg);

    //Receive and show response message from server
    cout<<"----------------------------\n\n";
    std::cout << "Receive message: " << std::endl;
    cout<<cl.recvData(1024);
    cout<<"\n\n----------------------------\n\n";

    if (arg.getCmd() == "hello") {
        std::cout << "Listen command \"ping\" on port 8080" << std::endl;
        PingHdl::init();
        PingHdl::run();
    }
    return 0;
}
