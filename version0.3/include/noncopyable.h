//
// Created by wuyao on 10/31/20.
//

#ifndef VERSION0_3_NONCOPYABLE_H
#define VERSION0_3_NONCOPYABLE_H

class noncopyable{
public:
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    noncopyable() = default;
    ~noncopyable() = default;
};

#endif //VERSION0_3_NONCOPYABLE_H
