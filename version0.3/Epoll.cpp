//
// Created by wuyao on 11/2/20.
//

#include "include/Epoll.h"
#include "include/Socket.h"
#include "include/utils.h"
#include <iostream>
#include <vector>
#include <unistd.h>


const int Epoll::MAX_EPOLL_EVENTS = 1024 * 8;
struct epoll_event* Epoll::epoll_events;
std::unordered_map<int, std::shared_ptr<http::HttpData>> Epoll::http_data_map;
TimerManager Epoll::timer_manager;
const  __uint32_t Epoll::DEFAULT_EVENTS = (EPOLLIN | EPOLLET | EPOLLONESHOT);

int Epoll::init_epoll(int max_events = MAX_EPOLL_EVENTS) {
    int epfd = epoll_create(max_events);
    if (epfd < 0){
        std::cout<<"epoll_create error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
    epoll_events = new epoll_event[max_events];
    return epfd;
}


void Epoll::add_fd(int epfd, int fd, const __uint32_t events, std::shared_ptr<http::HttpData> data){
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    // 将fd与http_data添加到http_data_map中
    http_data_map[fd] = data;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret < 0){
        std::cout<<"epoll_ctl error <"<<__FILE__<<__LINE__<<std::endl;
        // 释放http_data_map中的资源
        http_data_map[fd].reset();
    }
}

void Epoll::mod_fd(int epfd, int fd, const __uint32_t events, std::shared_ptr<http::HttpData> data){
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    http_data_map[fd] = data;
    int ret = epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
    if (ret < 0){
        std::cout<<"epoll_ctl error <"<<__FILE__<<__LINE__<<std::endl;
        http_data_map[fd].reset();
    }
}

void Epoll::del_fd(int epfd, int fd, const __uint32_t events){
    struct epoll_event ev;
    ev.events = events;
    ev.data.fd = fd;
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev);
    if (ret < 0)
        std::cout<<"epoll_ctl error<"<<__FILE__<<__LINE__<<std::endl;
    auto iter = http_data_map.find(fd);
    if (iter != http_data_map.end())
        http_data_map.erase(iter);
}

void Epoll::handle_connection(const nsocket::ServerSocket& server){
        std::shared_ptr<nsocket::ClientSocket> client(new nsocket::ClientSocket);
        // 在non-blocking模式下，如果返回值为-1，
        // 且errno == EAGAIN或errno == EWOULDBLOCK表示no connections没有新连接请求；
        // 大于0表示连接请求建立成功
        while (server.accept(*client) > 0){
            // 设置非阻塞
            int ret = utils::set_nonblocking(client->con_fd);
            if (ret < 0){
                std::cout<<"set_nonblocking error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
                client->close();
                continue;
            }

            std::shared_ptr<http::HttpData> shared_http_data(new http::HttpData);
            shared_http_data->m_request = std::shared_ptr<http::HttpRequest>(new http::HttpRequest());
            shared_http_data->m_respond = std::shared_ptr<http::HttpResponse>(new http::HttpResponse());
            std::shared_ptr<nsocket::ClientSocket> cur_client(new nsocket::ClientSocket);
            cur_client.swap(client);    // swap 交换两个 shared_ptr 对象(即交换所拥有的对象)
            shared_http_data->m_client = cur_client;
            shared_http_data->m_epfd = server.m_epfd;

            // 挂载到epoll树上统一监听事件
            add_fd(server.m_epfd, cur_client->con_fd, Epoll::DEFAULT_EVENTS, shared_http_data);
            // 添加到定时器
            timer_manager.add_timer(shared_http_data, TimerManager::DEFAULT_TIME_OUT);
        }
}

std::vector<std::shared_ptr<http::HttpData>>
Epoll::poll(nsocket::ServerSocket& server, int max_event, int timeout = -1){
    int ret_cnt = epoll_wait(server.m_epfd, epoll_events, max_event, timeout);
    if (ret_cnt < 0){
        std::cout<<"epoll_wait error <"<<__FILE__<<__LINE__<<std::endl;
        exit(1);
    }
    //
    std::vector<std::shared_ptr<http::HttpData>> res;
    for (int i = 0; i < ret_cnt; ++i) {
        int fd = epoll_events[i].data.fd;
        // 建立新连接
        if (fd == server.m_listen_fd)
            handle_connection(server);

            // 数据处理
        else {
            // 出错的描述符，移除定时器，关闭文件描述符
            // 对端正常关闭 EPOLLRDHUP 表示端断开连接
            // EPOLLHUP表示读写都关闭。
            if ((epoll_events[i].events & EPOLLRDHUP) ||
                (epoll_events[i].events & EPOLLHUP) || (epoll_events[i].events & EPOLLERR)) {
                auto iter = http_data_map.find(fd);
                if (iter != http_data_map.end()) {
                    // 将http_data与定时器分离开来
                    // 由于http_data中使用的weak_ptr释放后,会自动析构
                    iter->second->close_timer();
                }
                continue;
            }
            // ================== 正常的数据处理 ==================
            auto iter = http_data_map.find(fd);
            if (iter != http_data_map.end()) {
                // EPOLLPRI中的epoll(7)以及POLLPRI中的poll(2)用于接收这些紧急数据。
                // 紧急数据与正常数据处理
                if ((epoll_events[i].events & EPOLLIN) || (epoll_events[i].events & EPOLLPRI)) {
                    // 将连接放到res中给主线程添加到线程池的请求队列中
                    res.push_back(iter->second);
                    // 放到请求队列后,就可以从定时器中删除了
                    iter->second->close_timer();
                    http_data_map.erase(fd);
                }
                else {
                    std::cout << "error!" << std::endl;
                    ::close(fd);
                    continue;
                }
            }
        }
    }
    return res;
}