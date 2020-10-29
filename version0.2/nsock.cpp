//
// Created by wuyao on 10/27/20.
//
#include "include/nsock.h"
#include <sys/socket.h>
#include <iostream>
#include <string.h>

void nsock::set_reuse_port(int fd) {
    int opt_val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
}

void nsock::ServerInit::init_socket(){
    m_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sfd < 0){
        std::cout<<"socket error < FILE"<<__FILE__<<"> at"<<__LINE__<<std::endl;
    }
    bzero(&m_server, sizeof(m_server));
    m_server.sin_family = AF_INET;
    m_server.sin_port = htons(m_port);
    if (m_ip == nullptr)
        m_server.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, m_ip, &m_server.sin_addr);
    set_reuse_port(m_sfd);
    return ;
}

void nsock::ServerInit::socket_bind() {
    if (::bind(m_sfd, (struct sockaddr*)&m_server, sizeof(m_server)) < 0)
        std::cout<<"bind error < FILE"<<__FILE__<<"> at"<<__LINE__<<std::endl;
}

void nsock::ServerInit::socket_listen() {
    if (::listen(m_sfd, 64) < 0)
        std::cout<<"listen error < FILE"<<__FILE__<<"> at"<<__LINE__<<std::endl;
}

void nsock::ServerInit::socket_accept(ClientInit& client){
    int confd = ::accept(m_sfd, (struct sockaddr*)&client, &client.m_client_len);
    if (confd < 0)
        std::cout<<"accept error < FILE"<<__FILE__<<"> at"<<__LINE__<<std::endl;
    client.m_cfd = confd;
}
