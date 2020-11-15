//
// Created by wuyao on 11/2/20.
//

#ifndef VERSION0_3_EPOLL_H
#define VERSION0_3_EPOLL_H
#include "httpdata.h"
#include "Timer.h"
#include "Socket.h"
#include <sys/epoll.h>
#include <unordered_map>


class Epoll{
public:
    static int init_epoll(int max_events);
    static void add_fd(int epfd, int fd, const __uint32_t events, std::shared_ptr<http::HttpData> data);
    static void mod_fd(int epfd, int fd, const __uint32_t events, std::shared_ptr<http::HttpData> data);
    static void del_fd(int epfd, int fd, const __uint32_t events);

    // 处理连接请求
    static void handle_connection(const nsocket::ServerSocket& server);

    // 返回活跃事件数
    static std::vector<std::shared_ptr<http::HttpData>>
        poll(nsocket::ServerSocket& server, int max_event, int timeout);


public:
    static const int MAX_EPOLL_EVENTS;                 // epoll数组上限
    static struct epoll_event* epoll_events;           // epoll数组
    static std::unordered_map<int, std::shared_ptr<http::HttpData>> http_data_map;   // client_fd <==>httpdata
    static TimerManager timer_manager;                  // 定时器
    static const  __uint32_t DEFAULT_EVENTS;            // 默认事件
};



#endif //VERSION0_3_EPOLL_H
