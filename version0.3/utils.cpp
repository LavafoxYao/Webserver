//
// Created by wuyao on 10/31/20.
//
#include "include/utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <iostream>
#include <string.h>
#include <sys/stat.h>

//  删除字符串前置空格与\t
std::string& utils::ltrim(std::string& str){
    if (str.empty())    return str;
    int ret = str.find_first_not_of(" \t");
    if (ret == std::string::npos)   return str;
    str.erase(0, ret);
}

// 删除字符串末尾空格与\t
std::string& utils::rtrim(std::string& str){
    if (str.empty())    return str;
    int ret = str.find_last_not_of(" \t");
    if (ret == std::string::npos)   return str;
    str.erase(ret + 1);
    return str;
}

std::string& utils::trim(std::string& str){
    if (str.empty())    return str;
    ltrim(str);
    rtrim(str);
    return str;
}

// 设置端口复用
void utils::set_reuse_port(int fd){
    int opt_val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
}

// 设置非阻塞
int utils::set_nonblocking(int fd){
    int old_opt = fcntl(fd, F_GETFD);
    int new_opt = O_NONBLOCK | old_opt;
    fcntl(fd, F_SETFD, new_opt);
    return old_opt;
}

void utils::handle_sigpipe(){
    struct sigaction sa;
    bzero(&sa, sizeof(sa));
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    // 当服务器close一个连接时，若client端接着发数据。
    // 根据TCP协议的规定，会收到一个RST响应，client再往这个服务器发送数据时，
    // 系统会发出一个SIGPIPE信号给进程，告诉进程这个连接已经断开了，不要再写了。
    if(sigaction(SIGPIPE, &sa, NULL))
        return;
}

void utils::daemon_run(){
    int pid;
    signal(SIGCHLD, SIG_IGN);
    //1）在父进程中，fork返回新创建子进程的进程ID；
    //2）在子进程中，fork返回0；
    //3）如果出现错误，fork返回一个负值；
    pid = fork();
    if (pid < 0){
        std:: cout << "fork error <" <<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
    //父进程退出，子进程独立运行
    else if (pid > 0)
        exit(0);
    // 之前parent和child运行在同一个session里,parent是会话（session）的领头进程,
    // parent进程作为会话的领头进程，如果exit结束执行的话，那么子进程会成为孤儿进程，并被init收养。
    // 执行setsid()之后,child将重新获得一个新的会话(session)id。
    // 这时parent退出之后,将不会影响到child了。
    // setsid()调用成功后，进程成为新的会话组长和新的进程组长，并与原来的登录会话和进程组脱离。
    setsid();
    int fd;
    fd = open("/dev/null", O_RDWR, 0);
    if (fd != -1){
        // 释放资源
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
    }
    if (fd > 2)
        ::close(fd);
    return ;
}
int utils::check_base_path(char *basePath) {
    struct stat file;
    if (stat(basePath, &file) == -1) {
        return -1;
    }
    // 不是目录 或者不可访问
    if (!S_ISDIR(file.st_mode) || access(basePath, R_OK) == -1) {
        return -1;
    }
    return 0;
}

// 16进制数转化为10进制
int utils::hexit(char c){
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

/*
 *  这里的内容是处理%20之类的东西！是"解码"过程。
 *  %20 URL编码中的‘ ’(space)
 *  %21 '!' %22 '"' %23 '#' %24 '$'
 *  %25 '%' %26 '&' %27 ''' %28 '('......
 */
void utils::encode_str(char* to, int tosize, const char* from){
    int tolen;
    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
            *to = *from;
            ++to;
            ++tolen;
        }
        else{
            // %%%02x   前两个%%是为了输出%  (int) *from & 0xff ==> 十进制转16进制
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }
    }
    *to = '\0';
}

void utils::decode_str(char *to, char *from){
    for ( ; *from != '\0'; ++to, ++from  ){
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
            *to = hexit(from[1])*16 + hexit(from[2]);
            from += 2;
        }
        else{
            *to = *from;
        }
    }
    *to = '\0';
}

