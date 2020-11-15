//
// Created by wuyao on 11/1/20.
//

#ifndef VERSION0_3_SOCKET_H
#define VERSION0_3_SOCKET_H
#include <arpa/inet.h>

namespace nsocket{

    class ClientSocket;

    class ServerSocket{
    public:
        ServerSocket(const char* ip = nullptr, int port = 9090);
        ~ServerSocket();
        void bind();
        void listen();
        int accept(ClientSocket&)const;
        void close();


        struct sockaddr_in m_server;
        int m_listen_fd;
        int m_epfd;
        const char* m_ip;
        int m_port;
    };

    class ClientSocket{
    public:
        ClientSocket();
        ~ClientSocket();
        void close();


        struct sockaddr_in m_client;
        socklen_t m_client_len;
        int con_fd;
    };
}

#endif //VERSION0_3_SOCKET_H
