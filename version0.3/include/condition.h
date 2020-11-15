//
// Created by wuyao on 10/31/20.
//

#ifndef VERSION0_3_CONDITION_H
#define VERSION0_3_CONDITION_H
#include "noncopyable.h"
#include "mutexlock.h"

class Condition{
public:
    Condition(Mutex& lock):m_mtx(lock){
        pthread_cond_init(&m_cond, NULL);
    }

    ~Condition(){
        pthread_cond_destroy(&m_cond);
    }

    void wait(){
        pthread_cond_wait(&m_cond, m_mtx.get_mutex());
    }

    void notify(){
        pthread_cond_signal(&m_cond);
    }

    void notify_all(){
        pthread_cond_broadcast(&m_cond);
    }

private:
    Mutex& m_mtx;
    pthread_cond_t m_cond;
};

#endif //VERSION0_3_CONDITION_H
