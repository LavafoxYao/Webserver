//
// Created by wuyao on 11/2/20.
//
#include "include/Server.h"
#include "include/Epoll.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

// 默认的路径为当前文件
void HttpServer::server_running(int threads, int max_request_size) {
    thread::ThreadPool pool(threads, max_request_size);
    int epfd = Epoll::init_epoll(Epoll::MAX_EPOLL_EVENTS);
    std::shared_ptr<http::HttpData> http_data;
    http_data->m_epfd = epfd;
    this->server.m_epfd = epfd;
    __uint32_t events = (EPOLLIN | EPOLLET);
    Epoll::add_fd(epfd, this->server.m_listen_fd, events, http_data);
    while (true){
        // 主线程的作用是监控事件的发生
        // 然后把发生的事件都添加到请求队列中
        // 最后线程池中的线程来作为消费者完成消费
        std::vector<std::shared_ptr<http::HttpData>> res_events =
                Epoll::poll(this->server, Epoll::MAX_EPOLL_EVENTS, -1);
        // 将事件传递给线程池
        for (const auto& iter : res_events)
            pool.append_task(iter, std::bind(&HttpServer::do_request, this, std::placeholders::_1));
        // 处理定时器超时事件
        Epoll::timer_manager.handler_expired_event();
    }
}

void HttpServer::do_request(std::shared_ptr<void> arg){
    // share_ptr为了支持转型，所以提供了类似的转型函数即static_pointer_cast<T>
    // 从而使转型后仍然为shared_pointer对象，仍然对指针进行管理；
    std::shared_ptr<http::HttpData> http_data = std::static_pointer_cast<http::HttpData>(arg);
    char buffer[BUFFER_SIZE] = {0};
    int checked_index = 0, read_index = 0, start_line = 0;
    ssize_t data_read;
    http::HttpParse::PARSE_STATUS  check_state = http::HttpParse::PARSE_REQUEST_LINE;
    while (1){
        // 由于是非阻塞IO，所以返回-1 也不一定是错误，还需判断error
        // 在non-blocking模式下，如果返回值为-1，
        // 且errno == EAGAIN表示没有可接受的数据或很在接受尚未完成
        data_read = recv(http_data->m_client->con_fd, buffer + read_index, BUFFER_SIZE - read_index, 0);
        if (data_read == -1){
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                return;
            std::cout<<"recv error file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
            exit(1);
        }
        else if (data_read == 0){
            std::cout<<"client has closed the connection"<<std::endl;
            break;
        }
        read_index += data_read;

        http::HttpParse::HTTP_CODE res= http::HttpParse::parse_content
                (buffer,checked_index, check_state, read_index, start_line, *http_data->m_request);

        if (res == http::HttpParse::NO_REQUEST)
            continue;
        else if (res == http::HttpParse::GET_REQUEST){
            // 查看请求头中的Connection是否是keep-alive
            auto iter = http_data->m_request->m_headers.find(http::HttpRequest::Connection);
            if (iter != http_data->m_request->m_headers.end()){
                if (iter->second == "keep-alive" || iter->second == "Keep-Alive"){
                    http_data->m_respond->set_keep_alive(true);
                    http_data->m_respond->append_header("keep-alive", "timeout=20");
                }
                else
                    http_data->m_respond->set_keep_alive(false);
            }
            get_mime(http_data);
            version(http_data);
            // 根据请求文件的类型调用不同类型
            FILE_TYPE file_type = stat_filetype(http_data, base_path.c_str());
            send_file(http_data, file_type);
            // 给请求头中connection的keep-alive的请求添加定时器
            if (http_data->m_respond){
                Epoll::mod_fd(http_data->m_epfd, http_data->m_client->con_fd, Epoll::DEFAULT_EVENTS, http_data);
                Epoll::timer_manager.add_timer(http_data, TimerManager::DEFAULT_TIME_OUT);
            }
        }
        else
            std::cout<<"Bad Request"<<std::endl;
    }
}

void HttpServer::version(std::shared_ptr<http::HttpData> http_data){
    if (http_data->m_request->m_version == http::HttpRequest::HTTP_11)
        http_data->m_respond->set_http_version(http::HttpRequest::HTTP_11);
    else
        http_data->m_respond->set_http_version(http::HttpRequest::HTTP_10);
}

void HttpServer::get_mime(std::shared_ptr<http::HttpData> http_data){
    std::string cur_uri = http_data->m_request->m_uri;
    std::string res;
    // ?在url中的作用 分隔实际的URL和参数
    int ret = cur_uri.rfind('?');
    if (ret != std::string::npos)
        cur_uri.erase(ret);
    utils::trim(cur_uri);

    ret = cur_uri.rfind('.');
    if (ret != std::string::npos)
        res = cur_uri.substr(ret);
    // 找到对应的Mime类型
    auto iter = http::Mime_Map.find(res);
    if (iter != http::Mime_Map.end())
        http_data->m_respond->set_mimetype(iter->second);
    else
        http_data->m_respond->set_mimetype(http::Mime_Map["default"]);

    http_data->m_respond->set_filepath(cur_uri);
}

HttpServer::FILE_TYPE HttpServer::
stat_filetype(std::shared_ptr<http::HttpData> http_data, const char* file_path){
    char file[strlen(http_data->m_request->m_uri.c_str()) + strlen(file_path) + 1];
    strcpy(file, file_path);
    strcat(file, http_data->m_request->m_uri.c_str());
    struct stat st;
    int ret = stat(file, &st);
    // 文件不存在
    if (ret < 0 || http_data->m_respond->get_filepath() == "/"){
        http_data->m_respond->set_mimetype("text/html");
        if (http_data->m_respond->get_filepath() == "/"){
            http_data->m_respond->set_state_code(http::HttpResponse::k200OK);
            http_data->m_respond->set_state_msg("OK");
        }
        else{
            http_data->m_respond->set_state_code(http::HttpResponse::k404Notfound);
            http_data->m_respond->set_state_msg("Not Found");
        }
        return FILE_NOT_FOUND;
    }
    // 不是普通文件
    if (!S_ISREG(st.st_mode)){
        http_data->m_respond->set_mimetype("text/html");
        http_data->m_respond->set_state_code(http::HttpResponse::k403Forbidden);
        http_data->m_respond->set_state_msg("Forbidden");
        return FILE_FORBIDDEN;
    }
    // 普通文件就发送请求的路径文件
    http_data->m_respond->set_mimetype("text/html");
    http_data->m_respond->set_state_code(http::HttpResponse::k200OK);
    http_data->m_respond->set_state_msg("Ok");
    return FILE_OK;
}

void HttpServer::send_file(std::shared_ptr<http::HttpData> http_data, FILE_TYPE file_type){
    char respond_header[BUFFER_SIZE] = {0};
    http_data->m_respond->response_header(respond_header);
    if (FILE_NOT_FOUND == file_type){
        if (http_data->m_respond->get_filepath() == "/"){
            sprintf(respond_header + strlen(respond_header), "Content-Length:%d\r\n\r\n", strlen(INDEX_PAGE));
            sprintf(respond_header + strlen(respond_header), "%s", INDEX_PAGE);
        }
        else{
            sprintf(respond_header + strlen(respond_header), "Content-Length:%d\r\n\r\n", strlen(NOT_FOUND_PAGE));
            sprintf(respond_header + strlen(respond_header), "%s", NOT_FOUND_PAGE);
        }
        ::send(http_data->m_client->con_fd, respond_header, strlen(respond_header), 0);
    }
    else if (FILE_FORBIDDEN == file_type){
        sprintf(respond_header + strlen(respond_header), "Content-Length:%d\r\n\r\n", strlen(FORBIDDEN_PAGE));
        sprintf(respond_header + strlen(respond_header), "%s", FORBIDDEN_PAGE);
        ::send(http_data->m_client->con_fd, respond_header, strlen(respond_header), 0);
    }
    else{
        struct stat st;
        // 请求的资源的url
        const char* file = http_data->m_respond->get_filepath().c_str();
        const char* internal_err = "Internal error";
        int ret = stat(file, &st);
        //  请求的资源的并不存在
        if (ret < 0){
            std::cout<<"internal error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
            sprintf(respond_header + strlen(respond_header),"Content-Length:%d\r\n\r\n", strlen(internal_err));
            sprintf(respond_header + strlen(respond_header), "%s", internal_err);
            ::send(http_data->m_client->con_fd, respond_header, strlen(respond_header), 0);
            return;
        }

        int file_fd = ::open(file, O_RDONLY);
        if (file_fd < 0){
            std::cout<<"internal error <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
            sprintf(respond_header + strlen(respond_header),"Content-Length:%d\r\n\r\n", strlen(internal_err));
            sprintf(respond_header + strlen(respond_header), "%s", internal_err);
            ::send(http_data->m_client->con_fd, respond_header, strlen(respond_header), 0);
            ::close(file_fd);
            return;
        }

        sprintf(respond_header + strlen(respond_header), "Content-Length:%d\r\n\r\n", st.st_size);
        ::send(http_data->m_client->con_fd, respond_header, strlen(respond_header), 0);
        // 使用mmap来共享文件减少文件拷贝的次数
        void* ptr_map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
        ::send(http_data->m_client->con_fd, ptr_map, st.st_size, 0);
        munmap(ptr_map, st.st_size);
        ::close(file_fd);
        return ;
    }
}
