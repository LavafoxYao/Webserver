//
// Created by wuyao on 10/27/20.
//

#ifndef WEBSERVER_CONDITION_H
#define WEBSERVER_CONDITION_H
#include "MutexLock.h"
#include "noncopyable.h"


class Condition{
public:
    Condition(MutexLock& mxt): m_mxt(mxt){
        pthread_cond_init(&m_cond, NULL);
    }

    ~Condition(){
        pthread_cond_destroy(&m_cond);
    }
    //等待条件成立
    // pthread_cond_wait内部的操作顺序是将线程放到等待队列，
    // 然后解锁，等条件满足时重新竞争锁，竞争到后加锁，然后返回。
    void wait(){
        pthread_cond_wait(&m_cond,m_mxt.get_mutex_lock());
    }

    void notify(){
        pthread_cond_signal(&m_cond);
    }

    void notify_all(){
        pthread_cond_broadcast(&m_cond);
    }

private:
    MutexLock& m_mxt;           // 引用
    pthread_cond_t m_cond;
};

#endif //WEBSERVER_CONDITION_H
