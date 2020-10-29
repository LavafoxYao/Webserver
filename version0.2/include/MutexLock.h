//
// Created by wuyao on 10/27/20.
//

#ifndef WEBSERVER_MUTEXLOCK_H
#define WEBSERVER_MUTEXLOCK_H
#include "noncopyable.h"
#include <pthread.h>

// 继承了noncopyable保证了 显式调用构造函数不会再出现改类型的对象,确保了锁的唯一性
class MutexLock : public noncopyable{
public:
    MutexLock(){
        pthread_mutex_init(&m_mxt, NULL);   //只是初始化并没有加锁!!
    }

    ~MutexLock() {
        pthread_mutex_destroy(&m_mxt);
    }

    void lock(){
        pthread_mutex_lock(&m_mxt);
    }

    void unlock(){
        pthread_mutex_unlock(&m_mxt);
    }

    pthread_mutex_t* get_mutex_lock(){
        return &m_mxt;
    }

private:
    pthread_mutex_t m_mxt;          //互斥量
};

class MutexGuard : public noncopyable{
public:
    //在MutexGuard的构造函数中加锁
    MutexGuard(MutexLock& lock):m_Mtx(lock){
        m_Mtx.lock();
    }
    //在MutexGuard的析构函数unlock
    ~MutexGuard(){
        m_Mtx.unlock();
    }
private:
    MutexLock &m_Mtx;           // 引用
};

#endif //WEBSERVER_MUTEXLOCK_H
