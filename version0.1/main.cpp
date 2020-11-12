#include <iostream>
#include "include/nsock.h"
#include "include/server.h"

int main() {
    chdir("/home/wuyao/CLionProjects/version0.1/res");
    server::HttpServer server;
    server.server_running();
    return 0;
}
