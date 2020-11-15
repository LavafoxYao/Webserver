//
// Created by wuyao on 10/31/20.
//

#ifndef VERSION0_3_MUTEXLOCK_H
#define VERSION0_3_MUTEXLOCK_H
#include "noncopyable.h"
#include <pthread.h>

class Mutex: public noncopyable{
public:
    Mutex(){
        pthread_mutex_init(&m_mtx, NULL);
    };

    ~Mutex(){
        pthread_mutex_destroy(&m_mtx);
    };

    void lock(){
        pthread_mutex_lock(&m_mtx);
    }

    void unlock(){
        pthread_mutex_unlock(&m_mtx);
    }

    pthread_mutex_t* get_mutex(){
        return &m_mtx;
    }

private:
    pthread_mutex_t m_mtx;
};

// 自动锁
class MutexGuard : public noncopyable{
public:
    MutexGuard(Mutex& lock): m_Mtx(lock){
        m_Mtx.lock();
    }

    ~MutexGuard(){
        m_Mtx.unlock();
    }

private:
    Mutex &m_Mtx;
};

#endif //VERSION0_3_MUTEXLOCK_H
