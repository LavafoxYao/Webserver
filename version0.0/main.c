#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "epoll_server.h"

int main(int argc, char* argv[])
{
    if (argc < 3){
        printf("a.out port path !\n");
        return 0;
    }
    // 改变工作目录 到资源目录
    chdir(argv[2]);
    int port = atoi(argv[1]);
  
    server_run(port);
    return 0;
}

