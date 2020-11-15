//
// Created by wuyao on 10/31/20.
//
#include "include/threadpool.h"
#include <iostream>

thread::ThreadPool::ThreadPool(int threads, int max_request):m_cond(m_mtx), m_shut_mode(0){
    if (threads <= 0 || threads > MAX_THREAD_SIZE)
        threads = 4;
    if (max_request <= 0 || max_request > MAX_REQUEST_SIZE)
        max_request = 1024 * 8;
    m_max_request = max_request;
    m_threads.resize(threads);
    for (int i = 0; i < m_threads.size(); ++i){
        if (pthread_create(&m_threads[i], NULL, worker, this) != 0){
            std::cout<<"pthread_create error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        }
        m_thread_cnt++;
    }
}

thread::ThreadPool::~ThreadPool(){

}

void* thread::ThreadPool::worker(void* arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    if (nullptr == pool)    return nullptr;
    pool->run();
    return nullptr;
}

void thread::ThreadPool::shut_thread_pool(ShutdownMode shut_mode){
    {
        MutexGuard lock(m_mtx);
        if (m_shut_mode) {
            std::cout << "shut_thread_pool()" << std::endl;
            std::cout << "Closed before executing this function" << std::endl;
            return ;
        }
        m_shut_mode = shut_mode;
        m_cond.notify_all();
    }

    for (int i = 0; i < m_threads.size(); ++i){
        if (pthread_join(m_threads[i], NULL) != 0){
            std::cout<<"pthread_join error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        }
    }
}

void thread::ThreadPool::append_task(std::shared_ptr<void> arg, std::function<void(std::shared_ptr<void>)> fun){
    if (m_shut_mode){
        std::cout << "append_task()" << std::endl;
        std::cout << "Closed before executing this function" << std::endl;
        return ;
    }

    MutexGuard lock(m_mtx);
    {
        if (m_request_list.size() > MAX_REQUEST_SIZE) {
            std::cout << "request_list size = " << m_request_list.size() << std::endl;
            std::cout << "too many request~!" << std::endl;
            return;
        }
        ThreadTask task;
        task.arg = arg;
        task.process = fun;
        m_request_list.push_back(task);
        // 当请求队列中存在任务,就可以通知其他线程来消费
        if (m_request_list.size() >= 1)
            m_cond.notify();
    }
}


void thread::ThreadPool::run(){
    while (true){
        ThreadTask task ;
        {
            MutexGuard lock(m_mtx);
            // while 而不是if
            while (!m_shut_mode && m_request_list.empty())
                m_cond.wait();
            if ((m_shut_mode == immediate_mode) ||
                (m_shut_mode == graceful_mode && m_request_list.empty()))
                break;
            // 任务出队
            task = m_request_list.front();
            m_request_list.pop_front();
        }
        task.process(task.arg);
    }
}