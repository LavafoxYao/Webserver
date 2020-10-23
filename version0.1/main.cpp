#include <iostream>
#include "include/nsock.h"
#include "include/server.h"

int main() {
    chdir("/home/wuyao/CLionProjects/10/webserver_2/res/");
    server::HttpServer server;
    server.server_running();
    return 0;
}
