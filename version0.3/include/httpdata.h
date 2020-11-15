//
// Created by wuyao on 11/2/20.
//

#ifndef VERSION0_3_HTTPDATA_H
#define VERSION0_3_HTTPDATA_H
#include "httpparse.h"
#include "httpresponse.h"
#include "Socket.h"
#include "Timer.h"
#include <memory>

class TimerNode;

namespace http{
    class HttpData :public std::enable_shared_from_this<HttpData> {
    public:
        HttpData(){
            m_epfd = -1;
        }

        std::shared_ptr<HttpRequest>           m_request;   // 请求
        std::shared_ptr<HttpResponse>          m_respond;   // 响应
        std::shared_ptr<nsocket::ClientSocket> m_client;    // 客户

        int m_epfd;
        void set_timer(std::shared_ptr<TimerNode>);
        void close_timer();
    private:
        std::weak_ptr<TimerNode> m_timer;
    };

};

#endif //VERSION0_3_HTTPDATA_H
