#include "include/server.h"
#include <iostream>




int main() {
    chdir("/home/wuyao/CLionProjects/webserver/res");
    std::cout<<"/home/wuyao/CLionProjects/webserver/res"<<std::endl;
    server::HttpServer server;
    server.server_running();
    return 0;
}
