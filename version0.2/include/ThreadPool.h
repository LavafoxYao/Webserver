//
// Created by wuyao on 10/27/20.
//

#ifndef WEBSERVER_THREADPOOL_H
#define WEBSERVER_THREADPOOL_H
#include "MutexLock.h"
#include "Condition.h"
#include <vector>
#include <list>
#include <functional>

#define MAX_THREAD_SIZE 512
#define MAX_REQUEST_SIZE 1024

namespace thread{

    struct Task{
        std::function<void(void*)> process;
        void* arg;
    };

    class ThreadPool{
    public:
        ThreadPool(int thread_size);
        ~ThreadPool();

        void shutdown();
        void append_task(Task*);
        void run();
    private:
        static void* worker(void *);

        // 互斥量 + 条件变量
        MutexLock m_mtx;
        Condition m_cond;
        int  m_thread_size;
        int  m_cur_thread_cnt;
        bool m_shutdown;
        std::vector<pthread_t> m_thread;        // 线程池
        std::list<Task*> m_requst;              // 请求队列
    };
}

#endif //WEBSERVER_THREADPOOL_H
