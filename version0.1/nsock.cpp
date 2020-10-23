//
// Created by wuyao on 10/21/20.
//
#include "include/nsock.h"
#include <iostream>
#include <string.h>

void set_use_report(int fd){
    int opt_val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
}

// default ip: INADDR_ANY port = 9090
nsock::ServerInit::ServerInit(const char *ip , int port) : m_port(port),m_ip(ip) {
    m_sfd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_sfd == -1){
        std::cout<<"socket error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
    //设置端口复用
    set_use_report(m_sfd);

    bzero(&m_server, sizeof(m_server));
    m_server.sin_family = AF_INET;
    m_server.sin_port = htons(m_port);
    if (m_ip == nullptr)
        m_server.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, m_ip, &m_server.sin_addr);
}

void nsock::ServerInit::bind() {
    int ret = ::bind(m_sfd, (struct sockaddr*)&m_server, sizeof(m_server));
    if (ret == -1){
        std::cout<<"bind error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
}

void nsock::ServerInit::listen() {
    int ret = ::listen(m_sfd, 5);
    if (ret == -1){
        std::cout<<"listen error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
}

void nsock::ServerInit::accept(ClientInit & client) {
    int connfd = ::accept(m_sfd, (struct sockaddr*)&client.m_client, &client.m_clen);
    if (connfd == -1){
        std::cout<<"accept error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
    char ip[64] = {0};
    inet_ntop(AF_INET, &client.m_client.sin_addr, ip, 64);
    std::cout<<"new client ip: "<<ip<<" port: "<<client.m_client.sin_port<<std::endl;
    //
    client.m_cfd = connfd;
}
