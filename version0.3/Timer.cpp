//
// Created by wuyao on 11/2/20.
//
#include "include/Timer.h"
#include "include/Epoll.h"
#include <sys/time.h>

size_t TimerNode::m_cur_time = 0;
const size_t TimerManager::DEFAULT_TIME_OUT = 20 * 1000;    // 20s  ==> 20 * 1000 ms

TimerNode::TimerNode(std::shared_ptr<http::HttpData> data, size_t timeout = TimerManager::DEFAULT_TIME_OUT):m_http_data(data), m_is_delete(false){
    TimerNode::get_cur_time();
    m_expire_time = timeout + m_cur_time;
}

TimerNode::~TimerNode(){
    std::cout<<"~TimeNode()"<<std::endl;
    // 析构时如果是被deleted 则httpData为NULL,不用处理.
    // 而如果是超时，则需要删除Epoll中的httpDataMap中
    if (m_http_data){
        auto iter = Epoll::http_data_map.find(m_http_data->m_client->con_fd);
        // 如果存在于httpDataMap中就直接删除
        if (iter != Epoll::http_data_map.end())
            Epoll::http_data_map.erase(iter);
    }
}

void TimerNode::deleted(){
    m_http_data.reset();        // 释放shared_ptr指向的对象
    m_is_delete = true;
}

size_t TimerNode::get_cur_time(){
    struct timeval time;
    // 在C语言中可以使用函数 gettimeofday() 函数来得到时间。它的精度可以达到微妙
    // int gettimeofday(struct  timeval*tv,struct  timezone *tz)
    // gettimeofday() 会把目前的时间用tv结构体返回，当地时区的信息则放到tz所指的结构中
    gettimeofday(&time, NULL);  // 获得 1970到当前的秒 + 微妙
    m_cur_time = (time.tv_usec / 1000) + time.tv_sec * 1000;     // 转化为ms
}

void TimerManager::add_timer(std::shared_ptr<http::HttpData> data, size_t timeout) {
    std::shared_ptr<TimerNode> timer (new TimerNode(data, timeout));
    {
        MutexGuard lock(m_mtx);
        Timer_heap.push(timer);
        // 将TimerNode和HttpData关联起来
        data->set_timer(timer);
    }
}

void TimerManager::handler_expired_event() {
    MutexGuard lock(m_mtx);
    {
        // 更新当前时间
        TimerNode::get_cur_time();
        while (!Timer_heap.empty()){
            Shared_TimerNode top_timer = Timer_heap.top();
            // 删除
            if (top_timer->is_delete())
                Timer_heap.pop();
            // 超时
            else if (top_timer->is_expire_time())
                Timer_heap.pop();
            else
                break;
        }
    }
}