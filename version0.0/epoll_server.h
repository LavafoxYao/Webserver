#ifndef EPOLL_SERVER_H
#define EPOLL_SERVER_H


// 服务器相关
void server_run(int port);
int init_server(int port, int epfd);
void do_accept(int epfd, int lfd);
void do_read(int epfd, int fd);

// 处理http
void http_request(int fd, char* buf);
void http_respond(int fd, int statenum, const char *description, const char *filetype, long filelen);
void send_file(int cfd, const char* file);
void send_dir(int cfd, const char* dirname);

// 工具
int get_line(int fd, char* buf, int buf_size);
const char *get_file_type(const char *name);
void disconnect(int epfd, int fd);
int setnonblocking(int fd);
void addfd(int epfd, int fd);
void encode_str(char* to, int tosize, const char* from);
void decode_str(char *to, char *from);
#endif