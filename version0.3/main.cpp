#include <iostream>
#include <unistd.h>
#include <string.h>
#include "include/threadpool.h"
#include "include/utils.h"
#include "include/Server.h"


std::string base_path(".");

int main(int argc, char*argv[]) {
    int thread_cnt = 4;     //  默认线程数
    int port = 9090;        // 默认端口
    char tempPath[256];
    int opt = 0;
    const char *str = "t:p:r:d";
    bool daemon_state = false;

    while ((opt = getopt(argc, argv, str))!= -1){
        switch (opt){
            case 't':{
                thread_cnt = atoi(optarg);
                break;
            }
            case 'r':{
                int ret = utils::check_base_path(optarg);
                if (ret == -1) {
                    printf("Warning: \"%s\" 不存在或不可访问, 将使用当前目录作为网站根目录\n", optarg);
                    if(getcwd(tempPath, 300) == NULL){
                        perror("getcwd() error");
                        base_path = ".";
                    }
                    else{
                        base_path = tempPath;
                    }
                    break;
                }
                if (optarg[strlen(optarg) - 1] == '/') {
                    optarg[strlen(optarg) - 1] = '\0';
                }
                base_path = optarg;
                break;
            }
            case 'p':{
                port = atoi(optarg);
                break;
            }
            case 'd':{
                daemon_state = true;
                break;
            }
            default: break;
        }
    }
    if (daemon_state)
        utils::daemon_run();

    //  输出配置信息
    printf("<=======WY WebServer 配置信息=======>\n");
    printf("端口:\t%d\n", port);
    printf("线程数:\t%d\n", thread_cnt);
    printf("根目录:\t%s\n", base_path.c_str());

    // 产生	SIGPIPE 信号时就不会中止程序，直接把这个信号忽略掉。
    utils::handle_sigpipe();

    HttpServer server(nullptr, port);
    server.server_running(thread_cnt);
    return 0;
}
