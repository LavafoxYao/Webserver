//
// Created by wuyao on 10/31/20.
//

#ifndef VERSION0_3_THREADPOOL_H
#define VERSION0_3_THREADPOOL_H
#include "condition.h"
#include <vector>
#include <list>
#include <functional>
#include <memory>

const int MAX_THREAD_SIZE = 1024;
const int MAX_REQUEST_SIZE = 2048 * 8;

namespace thread{

    typedef enum{
        immediate_mode = 1,
        graceful_mode
    }ShutdownMode;

    struct ThreadTask{
        std::function<void(std::shared_ptr<void>)> process;
        std::shared_ptr<void> arg;
    };

    class ThreadPool{
    public:
        ThreadPool(int threads, int max_request);
        ~ThreadPool();
        static void* worker(void* arg);
        void run();
        void shut_thread_pool(ShutdownMode);
        void append_task(std::shared_ptr<void>arg, std::function<void(std::shared_ptr<void>)> fun);


    private:
        // 线程同步 mutex在condition前
        Mutex m_mtx;
        Condition m_cond;

        // 线程池属性
        int m_shut_mode;                        // 是否关闭 与 关闭的模式
        int m_thread_cnt;                       // 当前线程数
        int m_max_request;                      // 最大请求数
        std::vector<pthread_t> m_threads;       // 线程
        std::list<ThreadTask> m_request_list;   // 请求队列
    };


} //namespace thread;


#endif //VERSION0_3_THREADPOOL_H
