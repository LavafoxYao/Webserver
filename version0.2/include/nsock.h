//
// Created by wuyao on 10/27/20.
//

#ifndef WEBSERVER_NSOCK_H
#define WEBSERVER_NSOCK_H
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>


namespace nsock{
    void set_reuse_port(int fd);
    class ClientInit;
    class ServerInit{
    public:
        ServerInit(const char* ip = nullptr, int port = 9090):m_port(port), m_ip(ip){
        }
        ~ServerInit(){
            close(m_sfd);
        };
        void init_socket();
        void socket_bind();
        void socket_listen();
        void socket_accept(ClientInit&);

    private:
        int m_sfd;
        int m_port;
        const char* m_ip;
        struct sockaddr_in m_server;
    };

    class ClientInit{
    public:
        ClientInit(){
            m_client_len = sizeof(m_client);
        };
        ~ClientInit(){
            close(m_cfd);
        };

        struct sockaddr_in m_client;
        socklen_t m_client_len;
        int m_cfd;
    };


};  // namespacet nsock
#endif //WEBSERVER_NSOCK_H
