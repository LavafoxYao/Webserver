//
// Created by wuyao on 10/27/20.
//

#ifndef WEBSERVER_NONCOPYABLE_H
#define WEBSERVER_NONCOPYABLE_H

// 不可复制类, 保证了锁的唯一性
class noncopyable{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator = (const noncopyable&) = delete;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //WEBSERVER_NONCOPYABLE_H
