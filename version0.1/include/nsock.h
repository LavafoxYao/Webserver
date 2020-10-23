//
// Created by wuyao on 10/21/20.
//

#ifndef WEBSERVER_2_NSOCK_H
#define WEBSERVER_2_NSOCK_H
#include <arpa/inet.h>
#include <unistd.h>

namespace nsock{
    class ClientInit;

    class ServerInit{
    public:
        ServerInit(const char* ip = nullptr, int port = 9090);
        ~ServerInit(){
            ::close(m_sfd);
        };
        void bind();
        void listen();
        void accept(ClientInit&);


        struct sockaddr_in m_server;
        int m_sfd;
        const char* m_ip;
        const int m_port;
    };

    struct ClientInit{
        ClientInit(){
            m_clen = sizeof(m_cfd);
        }
        ~ClientInit(){
            ::close(m_cfd);
        }

        struct sockaddr_in m_client;
        int m_cfd;
        socklen_t m_clen;
    };
};

#endif //WEBSERVER_2_NSOCK_H
