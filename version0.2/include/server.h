//
// Created by wuyao on 10/28/20.
//

#ifndef WEBSERVER_SERVER_H
#define WEBSERVER_SERVER_H

#include "nsock.h"
#include "httpresponse.h"
#include "httpparse.h"

#define BUFFSIZE 2048

namespace server{
    class HttpServer{
    public:
        enum RETURN_STATE{FILE = 1, DIR = 2};
        explicit HttpServer(const char* ip = nullptr, int port = 9090):m_http_server(ip, port){
            m_http_server.init_socket();
            m_http_server.socket_bind();
            m_http_server.socket_listen();
        }
        void server_running();
        void do_request(void *arg);
        void version(http::HttpRequest&, http::HttpResponse&);
        RETURN_STATE stat_filetype(http::HttpRequest&, http::HttpResponse&);
        void get_mime(http::HttpRequest&, http::HttpResponse&);
        void send_file(http::HttpResponse&, const nsock::ClientInit&);
        void send_dir(http::HttpResponse&, nsock::ClientInit&);
        nsock::ServerInit m_http_server;
    };
}
#endif //WEBSERVER_SERVER_H
