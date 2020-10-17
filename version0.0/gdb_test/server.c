#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char * argv[]){
    if (argc < 2){
        perror("./a.out port");
        exit(1);
    }
    int port = atoi(argv[1]);
    int lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1){
        perror("socket error!\n");
        exit(1);
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = ntohl(INADDR_ANY);
    int optval = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    int ret = bind(lfd, (struct sockaddr*)&server, sizeof(server));
    if (ret == -1){
        perror("bind error!");
        exit(1);
    }
    ret = listen(lfd, 5);
    if (ret == -1){
        perror("listen error!");
        exit(1);
    }
    struct sockaddr_in client;
    socklen_t client_len = sizeof(client);
    while (1){
        int connfd = accept(lfd, (struct sockaddr*)&client, &client_len);
        if (connfd == -1){
            perror("accept error");
            exit(1);
        }
        char ip[64];
        printf("new client: ip = %s, port = %d\n", inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip)), htons(client.sin_port));
        char buf[1024] = {0};
        int ret = read(connfd, buf, 1024);
        if (ret == -1){
            perror("read error");
            exit(1);
        }
        printf("server:%s\n", buf);
    }
}