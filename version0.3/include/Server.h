//
// Created by wuyao on 11/2/20.
//

#ifndef VERSION0_3_SERVER_H
#define VERSION0_3_SERVER_H

#include "httpdata.h"
#include "httpparse.h"
#include "httpresponse.h"
#include "Socket.h"
#include "utils.h"
#include "Timer.h"
#include "threadpool.h"
#include <string>

const int BUFFER_SIZE = 2048;
extern std::string base_path;
const char NOT_FOUND_PAGE[] =  "<html>\n"
                               "<head><title>404 Not Found</title></head>\n"
                               "<body bgcolor=\"white\">\n"
                               "<center><h1>404 Not Found</h1></center>\n"
                               "<hr><center>WY WebServer/0.3 (Linux)</center>\n"
                               "</body>\n"
                               "</html>";

const char FORBIDDEN_PAGE[] =  "<html>\n"
                               "<head><title>403 Forbidden</title></head>\n"
                               "<body bgcolor=\"white\">\n"
                               "<center><h1>403 Forbidden</h1></center>\n"
                               "<hr><center>WY WebServer/0.3 (Linux)</center>\n"
                               "</body>\n"
                               "</html>";

const char INDEX_PAGE[] =  "<!DOCTYPE html>\n"
                           "<html>\n"
                           "<head>\n"
                           "    <title>Welcome to WY WebServer!</title>\n"
                           "    <style>\n"
                           "        body {\n"
                           "            width: 35em;\n"
                           "            margin: 0 auto;\n"
                           "            font-family: Tahoma, Verdana, Arial, sans-serif;\n"
                           "        }\n"
                           "    </style>\n"
                           "</head>\n"
                           "<body>\n"
                           "<h1>Welcome to WY WebServer!</h1>\n"
                           "<p>If you see this page, the wy webserver is successfully installed and\n"
                           "    working. </p>\n"
                           "\n"
                           "<p>For online documentation and support please refer to\n"
                           "    <a href=\"https://github.com/LavafoxYao/Webserver\">WY WebServer</a>.<br/>\n"
                           "\n"
                           "<p><em>Thank you for using WY WebServer.</em></p>\n"
                           "</body>\n"
                           "</html>";

class HttpServer{
public:
    enum FILE_TYPE{FILE_OK = 0, FILE_NOT_FOUND, FILE_FORBIDDEN };
    explicit HttpServer(const char* ip = nullptr, int port = 9090): server(ip, port){
        server.bind();
        server.listen();
    }

    void server_running(int threads, int max_request_size = MAX_REQUEST_SIZE);
    void do_request(std::shared_ptr<void> arg);

    void version(std::shared_ptr<http::HttpData> http_data);
    void get_mime(std::shared_ptr<http::HttpData> http_data);

    FILE_TYPE stat_filetype(std::shared_ptr<http::HttpData>, const char* file_path);
    void send_file(std::shared_ptr<http::HttpData> http_data, FILE_TYPE file_type);

private:
    nsocket::ServerSocket server;
};


#endif //VERSION0_3_SERVER_H