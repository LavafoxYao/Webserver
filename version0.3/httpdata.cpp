//
// Created by wuyao on 11/2/20.
//
#include "include/httpdata.h"

void http::HttpData::set_timer(std::shared_ptr<TimerNode> timer) {
    m_timer = timer;    // m_timer是weak_ptr并不会增加timer的计数
}

void http::HttpData::close_timer() {
    // 首先判断Timer是否还在,有可能已经超时释放
    // lock()函数返回一个指向共享对象的shared_ptr，否则返回一个空shared_ptr。
    if (m_timer.lock()){
        std::shared_ptr<TimerNode> tmp(m_timer.lock());
        tmp->deleted();
        // 断开weak_ptr
        m_timer.reset();
    }
}

