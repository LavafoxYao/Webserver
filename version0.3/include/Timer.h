//
// Created by wuyao on 11/1/20.
//

#ifndef VERSION0_3_TIMER_H
#define VERSION0_3_TIMER_H
#include "httpdata.h"
#include "mutexlock.h"
#include <queue>
#include <deque>
#include <memory>

// 添加定时器支持HTTP长连接，定时回调handler处理超时连接
class TimerNode{
public:
    TimerNode(std::shared_ptr<http::HttpData> data, size_t timeout);
    ~TimerNode();

    bool is_delete() const {return m_is_delete;}                          // 是否删除
    void deleted();                                                       // 删除
    bool is_expire_time() const {return m_expire_time > m_cur_time;}      // 是否超时
    bool get_expire_time() const {return m_expire_time;}                  // 获取超时时间

    std::shared_ptr<http::HttpData> get_http_data() const {return m_http_data;}
    static size_t get_cur_time();

    static size_t m_cur_time;                       // 当前时间
private:
    bool m_is_delete;                               // 是否删除
    size_t m_expire_time;                           // 超时时间
    std::shared_ptr<http::HttpData> m_http_data;    // http数据

};

// 比较时间的仿函数
struct TimerCmp{
    bool operator()(std::shared_ptr<TimerNode>& A, std::shared_ptr<TimerNode>& B){
        return A->get_expire_time() > B->get_expire_time();
    }
};

class TimerManager{
public:
    typedef std::shared_ptr<TimerNode> Shared_TimerNode;
    void add_timer(std::shared_ptr<http::HttpData> data, size_t timeout);   //httpData相当于一个keep-alive连接
    void handler_expired_event();

    const static size_t DEFAULT_TIME_OUT;
private:
    std::priority_queue<Shared_TimerNode, std::deque<Shared_TimerNode>, TimerCmp> Timer_heap;   // 小顶堆
    Mutex m_mtx;
};

#endif //VERSION0_3_TIMER_H
