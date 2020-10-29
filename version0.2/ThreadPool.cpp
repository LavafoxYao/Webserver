//
// Created by wuyao on 10/27/20.
//

#include "include/ThreadPool.h"
#include <iostream>

using namespace thread;

// 构造线程池
ThreadPool::ThreadPool(int thread_size): m_thread_size(thread_size), m_cond(m_mtx), m_shutdown(false){
    if (thread_size <= 0 || thread_size > MAX_THREAD_SIZE)
        thread_size = 3;
    m_thread_size = thread_size;
    m_thread.resize(m_thread_size);
    for (int i = 0; i < m_thread_size; ++i){
        if (pthread_create(&m_thread[i], NULL, worker, this) != 0){
            std::cout<<"pthread_create error! file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        }
        m_cur_thread_cnt++;
    }
    return ;
}

ThreadPool::~ThreadPool(){}

// 关闭线程池
void ThreadPool::shutdown() {
    if (m_shutdown){
        std::cout<<"Already closed"<<std::endl;
    }
    m_cond.notify_all();
    for (int i = 0; i <= m_cur_thread_cnt;++i){
        if (pthread_join(m_thread[i], NULL) != 0)
            std::cout<<"pthread_join error file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
    }
    m_shutdown = true;
    return ;
}

// 线程工作函数
void* ThreadPool::worker(void *arg){
    ThreadPool* pool = static_cast<ThreadPool*>(arg);
    if (pool == NULL)   return nullptr;
    pool->run();
    return nullptr;
}

// 线程真正的工作函数
void ThreadPool::run(){
    while (1){
        Task *tsk = nullptr;
        {
            MutexGuard lock(m_mtx);
            // 这里存在一个虚假唤醒的问题
            // 当线程池未关闭且请求队列为空 等待请求队列
            while (!m_shutdown && m_requst.empty())
                m_cond.wait();
            tsk = m_requst.front();
            m_requst.pop_front();
            tsk->process(tsk->arg);
        }
        if (tsk == nullptr) continue;
        delete(tsk);
    }
}

// 将请求添加到任务队列中
void ThreadPool::append_task(Task* tsk){
   if (tsk == nullptr)  return ;
   MutexGuard guard(m_mtx);
   if (m_requst.size() > MAX_REQUEST_SIZE){
       std::cout<<m_requst.size()<<std::endl;
       std::cout<<"this task request too much!"<<std::endl;
       return ;
   }
   m_requst.push_back(tsk);
   if (m_requst.size() >= 1)
       m_cond.notify();
   return ;
}