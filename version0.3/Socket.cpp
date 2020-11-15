//
// Created by wuyao on 11/1/20.
//
#include "include/Socket.h"
#include "include/utils.h"
#include <iostream>
#include <string.h>
#include <unistd.h>

nsocket::ServerSocket::ServerSocket(const char *ip, int port):m_ip(ip), m_port(port){
    m_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listen_fd < 0) {
        std::cout << "socket error <" << __FILE__ << "> at" << __LINE__ << std::endl;
        exit(1);
    }
    bzero(&m_server, sizeof(m_server));
    m_server.sin_family = AF_INET;
    m_server.sin_port = htons(m_port);
    if (nullptr == m_ip)
        m_server.sin_addr.s_addr = htonl(INADDR_ANY);
    else
        inet_pton(AF_INET, m_ip, &m_server.sin_addr);

    // 设置socket使得端口复用和非阻塞
    utils::set_reuse_port(m_listen_fd);
    utils::set_nonblocking(m_listen_fd);
}

nsocket::ServerSocket::~ServerSocket(){
    close();
}

void nsocket::ServerSocket:: bind(){
    int ret = ::bind(m_listen_fd, (struct sockaddr*)&m_server, sizeof(m_server));
    if (ret < 0){
        std::cout << "bind error <" << __FILE__ << "> at" << __LINE__ << std::endl;
        exit(1);
    }
}

void nsocket::ServerSocket::listen(){
    int ret = ::listen(m_listen_fd, 1024);
    if (ret < 0){
        std::cout << "listen error <" << __FILE__ << "> at" << __LINE__ << std::endl;
        exit(1);
    }
}

int nsocket::ServerSocket::accept(ClientSocket& client) const{
    // 非阻塞accept
    int con_fd = ::accept(m_listen_fd, (struct sockaddr*)&client, &client.m_client_len);
    // 由于是非阻塞
    // 在non-blocking模式下，如果返回值为-1，且errno == EAGAIN或errno == EWOULDBLOCK
    // 表示no connections没有新连接请求, 大于0表示连接请求建立成功
    if (con_fd < 0){
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return con_fd;
        std::cout << "accept error <" << __FILE__ << "> at" << __LINE__ << std::endl;
        exit(1);
    }
    client.con_fd = con_fd;
    return con_fd;
}

void nsocket::ServerSocket::close(){
    if (m_listen_fd > 0){
        ::close(m_listen_fd);
        m_listen_fd = -1;
    }
}

nsocket::ClientSocket::ClientSocket() {
    m_client_len = sizeof(m_client);
}

nsocket::ClientSocket:: ~ClientSocket(){
    close();
}

void nsocket::ClientSocket::close(){
    if (con_fd > 0){
        ::close(con_fd);
        con_fd = -1;
    }
}

