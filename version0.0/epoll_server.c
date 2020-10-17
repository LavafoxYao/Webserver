// Created by wuyao on 2020/10/16 4:26

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <error.h>
#include <dirent.h>
#include <ctype.h>
#include "epoll_server.h"

#define MAXEVENTNUM 2048
#define LITTERSIZE 128
#define BUFFSIZE 4096


int init_server(int port, int epfd){
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1){
        printf("socket error!\n");
        exit(1);
    }
    struct sockaddr_in server;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);
    // 设置端口复用
    int optval = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    int ret = bind(lfd, (struct sockaddr*)&server, sizeof(server));
    if (ret == -1){
        printf("bind error\n");
        exit(1);
    }
    /* backlog参数提示内核监听队列的最大长度,监听队列的长度如果超过backlog
    *  服务器将不再受理新的客户连接,客户端也将受到ECONNERFUSED错误信息.
    *  内核2.2后,它表示处于完全连接状态的socket的上限,处于半连接状态的socket的上限由系统定义
    *  backlog参数的典型值为5
    */
    ret = listen(lfd, 5);
    if (ret == -1){
        printf("listen error\n");
        exit(1);
    }
    addfd(epfd, lfd);
    return lfd;
}

void server_run(int port){
    // IO 复用
    // epoll_create() size现在并不起作用,只是给内核一个提示,告诉它事件表需要多大.
    int epfd = epoll_create(1024);
    if (epfd == -1){
        printf("epoll_create error");
        exit(1);
    }

    int lfd = init_server(port, epfd);
    struct epoll_event ev[MAXEVENTNUM];
    while(1){
        // cnum 为epoll监测到多少个fd发生了待监听事件
        int cnum = epoll_wait(epfd, ev, MAXEVENTNUM, -1);
        if (cnum < 0){
            printf("epoll failure !\n");
            break;
        }
        for (int i = 0; i < cnum; ++i){
            // 只处理读事件
            if ((ev[i].events & EPOLLIN) == 0)
                continue;
            else if (ev[i].data.fd == lfd){
                // 建立新的连接
                do_accept(epfd, lfd);
            }
            else{
                // 处理浏览器发来的数据
                do_read(epfd, ev[i].data.fd);
            }
        }
    }
    return ;
}

void do_accept(int epfd, int lfd){
    struct sockaddr_in client;
    socklen_t len = sizeof(client);
    int confd = accept(lfd,(struct sockaddr*)&client, &len);
    if (confd == -1){
        printf("do_accept accept error!\n");
        exit(1);
    }
    char buf[LITTERSIZE];
    printf("new client ip: %s, port: %d\n", 
        inet_ntop(AF_INET, &client.sin_addr, buf, sizeof(buf)), ntohs(client.sin_port));

    addfd(epfd, confd);
}


void do_read(int epfd, int fd){
    // 浏览器发来的数据读取到内存中
    // 因为我们只实现GET方法 无需处理数据==>故只用读取请求行
    char buf[BUFFSIZE];
    // 先读请求行 并且去除/r
    int ret = get_line(fd, buf, BUFFSIZE);
    if (ret < 0){
        printf("getline error");
        exit(1);
    }
    else if (ret == 0){
        // do_read被触发是因为发生了EPOLLIN事件,但无数据,则说明客户端断开连接
        printf("客户端断开连接\n");
        exit(1);
    }
    // 打印请求头 ==> 由于项目中只处理get请求 只需获得请求行的信息就完事了
    printf("请求行:%s", buf);
    // 打印剩下的请求头中的信息
    printf("========== 请求头 ==========\n");
    while(ret > 1){
        char tmp[BUFFSIZE] = {0};
        ret = get_line(fd, tmp, BUFFSIZE);
        printf("-----:%s", tmp);
        printf("%d\n", ret);
    }
    printf("========== The END ==========\n");
    // 处理GET方法
    if (strncasecmp("get", buf, 3) == 0){
        // 处理http请求
        printf("strncasecmp()\n");
        http_request(fd, buf);
        // 处理完成 ==> 关闭套接字
        disconnect(epfd, fd);
    }
    return ;
}

// 处理http请求行
void http_request(int cfd, char* request){
    // 拆分http请求行
    // get /xxx http/1.1
    char method[12], path[1024], protocol[12];
    // sscanf将字符串格式化%[^ ] %[^ ]正则匹配非空格的字符,并且以空格作为分隔符
    sscanf(request, "%[^ ] %[^ ] %[^ ]", method, path, protocol);
    printf("method = %s, path = %s, protocol = %s\n", method, path, protocol);

    // 转码 将不能识别的中文乱码 - > 中文
    // 解码 %23 %34 %5f
    decode_str(path, path);
    // 处理 资源path  /URL ==> 去除/
    char* file = path+1;
    // 如果没有指定访问的资源, 默认显示资源目录中的内容
    if(strcmp(path, "/") == 0){
        // file的值, 资源目录的当前位置
        file = "./";
    }
    // 获取文件属性
    struct stat st;
    int ret = stat(file, &st);
    if(ret == -1){
        // show 404
        http_respond(cfd, 404, "File Not Found", ".html", -1);
        send_file(cfd, "404.html");
    }
    // 如果是目录
    if(S_ISDIR(st.st_mode)){
        // 发送头信息
        http_respond(cfd, 200, "OK", get_file_type(".html"), -1);
        // 发送目录信息
        send_dir(cfd, file);
    }
    else if(S_ISREG(st.st_mode)){
        // 文件
        // 发送消息报头
        http_respond(cfd, 200, "OK", get_file_type(file), st.st_size);
        // 发送文件内容
        send_file(cfd, file);
    }
}

void http_respond(int fd, int statenum, const char *description, const char *filetype, long filelen){
    char buf[BUFFSIZE] = {0};
    // 发送响应行
    sprintf(buf, "http/1.1 %d %s\r\n", statenum, description);
    printf("http_respond==========\n");
    printf("%s",buf);
    // 因为http基于tcp为流式网络协议,故这里用strlen() 确保后面的数据能够完整的接上而不会多出空余的
    send(fd, buf, strlen(buf), 0);
    sprintf(buf, "Content-Type:%s\r\n", filetype);
    sprintf(buf+strlen(buf), "Content-Length:%ld\r\n",filelen);
    printf("%s",buf);
    send(fd, buf, strlen(buf), 0);
    
    // 发送空行
    printf("\n");
    send(fd, "\r\n", 2, 0);
}

void send_file(int cfd, const char* file){
    int fd = open(file, O_RDONLY);
    if (fd == -1){
        send_file(cfd, "404.html");
        exit(1);
    }
    char buf[BUFFSIZE] = {0};
    int read_n = 0;
    // 循环从file中读取数据
    while ((read_n = read(fd, buf, BUFFSIZE)) > 0){
    // 将读到的数据转交给 cfd中
        send(cfd, buf, read_n, 0);
    }
    if (read_n == -1){
        perror("read error!");
        exit(1);
    }
    close(fd);
    return ;
}

// 发送目录内容
void send_dir(int cfd, const char* dirname){
    // 拼一个html页面<table></table>
    char buf[4094] = {0};

    sprintf(buf, "<html><head><title>目录名: %s</title></head>", dirname);
    sprintf(buf+strlen(buf), "<body><h1>当前目录: %s</h1><table>", dirname);

    char enstr[1024] = {0};
    char path[1024] = {0};
    // 目录项二级指针
    struct dirent** ptr;
    int num = scandir(dirname, &ptr, NULL, alphasort);
    // 遍历
    for(int i = 0; i < num; ++i)
    {
        char* name = ptr[i]->d_name;
        // 拼接文件的完整路径
        sprintf(path, "%s/%s", dirname, name);
        printf("path = %s ===================\n", path);
        struct stat st;
        stat(path, &st);

        encode_str(enstr, sizeof(enstr), name);
        // 如果是文件
        if(S_ISREG(st.st_mode)){
            sprintf(buf+strlen(buf), 
                    "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                    enstr, name, (long)st.st_size);
        }
        // 如果是目录
        else if(S_ISDIR(st.st_mode)){
            sprintf(buf+strlen(buf), 
                    "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>",
                    enstr, name, (long)st.st_size);
        }
        send(cfd, buf, strlen(buf), 0);
        memset(buf, 0, sizeof(buf));
        // 字符串拼接
    }

    sprintf(buf+strlen(buf), "</table></body></html>");
    send(cfd, buf, strlen(buf), 0);

    printf("dir message send OK!!!!\n");
}

int get_line(int fd, char* buf, int buf_size){
    int cnt = 0;
    int ret = 0;
    char c = ' ';
    while (cnt < buf_size && c != '\r' && c != '\n'){
        ret = recv(fd, &c, 1, 0);
        if (ret < 0)
            return ret;
        else if (ret == 0)
            break;
        if (c == '\r'){
            // 如果c 为 '\r'试探性读下一个字符,判断下个字符是否是'\n'
            ret = recv(fd, &c, 1, MSG_PEEK);
            if (ret > 0 && c == '\n')
                recv(fd, &c, 1, 0);
            else
                c = '\n';
        }
        buf[cnt++] = c;
    }
    buf[cnt] = '\0';
    return cnt;
}

int setnonblocking(int fd){
    int old_opt = fcntl(fd, F_GETFL);
    int new_opt = old_opt | O_NONBLOCK;
    fcntl(fd, F_SETFD, new_opt);
    return old_opt;
}

//将fd挂到epoll树上,并使其设置为非阻塞
void addfd(int epfd, int fd){
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN | EPOLLET;
    int ret = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    if (ret == -1){
        printf("addfd epoll_ctl error!\n");
        exit(1);
    }
    setnonblocking(fd);
}

// 将fd从epfd上取下,并关闭fd
void disconnect(int epfd, int fd){
    int ret = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
    if (ret == -1){
        printf("disconnect epoll_clt error\n");
        exit(1);
    }
    close(fd);
    return ;
}

// 通过文件名获取文件的类型
const char *get_file_type(const char *name){
    char* dot;
    // 自右向左查找‘.’字符, 如不存在返回NULL
    dot = strrchr(name, '.');   
    if (dot == NULL)
        return "text/plain; charset=utf-8";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
        return "text/html; charset=utf-8";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".au") == 0)
        return "audio/basic";
    if (strcmp( dot, ".wav" ) == 0)
        return "audio/wav";
    if (strcmp(dot, ".avi") == 0)
        return "video/x-msvideo";
    if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
        return "video/quicktime";
    if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
        return "video/mpeg";
    if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
        return "model/vrml";
    if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
        return "audio/midi";
    if (strcmp(dot, ".mp3") == 0)
        return "audio/mpeg";
    if (strcmp(dot, ".ogg") == 0)
        return "application/ogg";
    if (strcmp(dot, ".pac") == 0)
        return "application/x-ns-proxy-autoconfig";

    return "text/plain; charset=utf-8";
}


// 16进制数转化为10进制
int hexit(char c){
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
 *  相关知识html中的‘ ’(space)是&nbsp
 */
void encode_str(char* to, int tosize, const char* from){
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

void decode_str(char *to, char *from){
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
