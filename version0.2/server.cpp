//
// Created by wuyao on 10/28/20.
//
#include "include/server.h"
#include "include/httpparse.h"
#include "include/utils.h"
#include "include/httpresponse.h"
#include "include/ThreadPool.h"
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <dirent.h>
#include <iostream>

void server::HttpServer::server_running(){
    thread::ThreadPool pool(4);
    while (1){
        nsock::ClientInit* client  = new nsock::ClientInit;
        thread::Task* tsk = new thread::Task;
        m_http_server.socket_accept(*client);
        tsk->process = std::bind(&HttpServer::do_request, this, std::placeholders::_1);
        tsk->arg = client;
        pool.append_task(tsk);
    }
}

void server::HttpServer::do_request(void *arg) {
    nsock::ClientInit client = *(static_cast<nsock::ClientInit*>(arg));
    char buffer[BUFFSIZE] = {0};
    int data_read = 0, read_index = 0, checked_index = 0, start_line = 0;
    http::HttpParse::PARSE_STATUS check_state = http::HttpParse::PARSE_REQUEST_LINE;
    while (1){
        http::HttpRequest request;
        data_read = recv(client.m_cfd, buffer + read_index, BUFFSIZE - read_index, 0);
        if (data_read == -1){
            std::cout<<"recv error file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
            exit(1);
        }
        else if (data_read == 0){
            std::cout<<"client has closed the connection"<<std::endl;
            break;
        }
        read_index += data_read;
        http::HttpParse::HTTP_CODE res= http::HttpParse::
        parse_content(buffer, checked_index, check_state, read_index, start_line,request);
        if (res == http::HttpParse::NO_REQUEST)
            continue;
        else if (res == http::HttpParse::GET_REQUEST){
            http::HttpResponse response(true);
            get_mime(request,response);
            version(request, response);
            // 根据请求文件的类型调用不同类型
            RETURN_STATE ret = stat_filetype(request, response);
            if (ret == FILE)
                send_file(response, client);
            else
                send_dir(response, client);
        }
        else
            std::cout<<"Bad Request"<<std::endl;
    }
}

void server::HttpServer::version(http::HttpRequest & request, http::HttpResponse &response) {
    if (request.m_version == http::HttpRequest::HTTP_11)
        response.set_http_version(http::HttpRequest::HTTP_11) ;
    else
        response.set_http_version(http::HttpRequest::HTTP_10);
    response.append_header("Webserver", "Wuyao Webserver");
}

void server::HttpServer::get_mime(http::HttpRequest & request, http::HttpResponse &response){
    std::string cur_uri = request.m_uri;
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
        response.set_mimetype(iter->second);
    else
        response.set_mimetype("default");

    response.set_filepath(cur_uri);
}

server::HttpServer::
RETURN_STATE server::HttpServer::stat_filetype(http::HttpRequest& request, http::HttpResponse& response){
    // fixme
    const char* base = "/home/wuyao/CLionProjects/version0.2/res";
    char* file = new char(strlen(request.m_uri.c_str()) + strlen(base) + 1);
    strcpy(file, base);
    strcat(file, request.m_uri.c_str());
    // fixme
    struct stat st;
    int ret = stat(file, &st);
    std::cout<<"file path is "<<file<<std::endl;
    if (ret < 0){
        response.set_state_code(http::HttpResponse::k404Notfound);
        response.set_state_msg("404 not found!");
        response.set_filepath(request.m_uri +"404.html");
        return FILE;
    }
    if (S_ISDIR(st.st_mode)){
        response.set_state_code(http::HttpResponse::k200OK);
        response.set_state_msg("200 OK!");
        response.set_filepath(file);
        return DIR;
    }
    else if (S_ISREG(st.st_mode)){
        response.set_state_code(http::HttpResponse::k200OK);
        response.set_state_msg("200 OK!");
        response.set_filepath(file);
    }
    else{
        response.set_state_code(http::HttpResponse::k403Forbiden);
        response.set_state_msg("403 forbiden");
        response.set_filepath(request.m_uri +"403.html");
    }
    return FILE;
}

void server::HttpServer::send_file(http::HttpResponse& response, const nsock::ClientInit& client){
    char buffer[BUFFSIZE] = {0};
    response.response_header(buffer);
    //std::cout<<buffer<<std::endl;
    const char *internal_error = "Internal Error";
    struct stat st;
    int ret = stat(response.get_filepath().c_str(), &st);
    if (ret < 0){
        sprintf(buffer, "Content-length: %d\r\n\r\n", strlen(internal_error));
        // 响应实体
        sprintf(buffer + strlen(buffer), "%s", internal_error);
        ::send(client.m_cfd, buffer, strlen(buffer), 0);
        return ;
    }
    int file_fd = open(response.get_filepath().c_str(), O_RDONLY);
    if (file_fd < 0){
        sprintf(buffer, "Content-Length: %d\r\n\r\n", strlen(internal_error));
        // 响应实体
        sprintf(buffer + strlen(buffer), "%s", internal_error);
        ::send(client.m_cfd, buffer, strlen(buffer), 0);
        return;
    }
    sprintf(buffer + strlen(buffer), "Content-length: %d\r\n\r\n", st.st_size);
    std::cout<<buffer<<std::endl;
    ::send(client.m_cfd, buffer, strlen(buffer), 0);
    // send file
    void *mptr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, file_fd, 0);
    ::send(client.m_cfd, mptr, st.st_size,0);
    munmap(mptr, st.st_size);
    close(file_fd);
}

void server::HttpServer::send_dir(http::HttpResponse& response,  nsock::ClientInit& client){
    char buffer[BUFFSIZE] = {0};
    char dir_buffer[BUFFSIZE] = {0};
    //int dir_len = 0;
    response.response_header(buffer);
    //std::cout<<buffer<<std::endl;
    const char *internal_error = "Internal Error";
    struct stat st;
    int ret = stat(response.get_filepath().c_str(), &st);
    if (ret < 0){
        sprintf(buffer, "Content-Length: %d\r\n\r\n", strlen(internal_error));
        // 响应实体
        sprintf(buffer + strlen(buffer), "%s", internal_error);
        ::send(client.m_cfd, buffer, strlen(buffer), 0);
        return ;
    }
    // fixme
    // sprintf(buffer + strlen(buffer), "Content-length: %d\r\n\r\n", -1);
    // ::send(client.m_cfd, buffer, strlen(buffer), 0);

    int cfd = client.m_cfd;
    std::cout<<"dirname == "<< response.get_filepath().c_str() <<std::endl;
    const char * dirname = response.get_filepath().c_str();
    sprintf(dir_buffer, "<html><head><title>: %s</title></head>", dirname);
    sprintf(dir_buffer + strlen(dir_buffer), "<body><h1>: %s</h1><table>", dirname);
    sprintf(dir_buffer + strlen(dir_buffer), "<meta charset=\"utf-8\">");
    char path[128] = {0};
    // 目录项二级指针
    struct dirent** ptr;
    int num = scandir(dirname, &ptr, NULL, alphasort);
    if (num < 0){
        std::cout<<"scandir erro <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        exit(1);
    }
    // 遍历
    for(int i = 0; i < num; ++i){
        char* name = ptr[i]->d_name;
        // 拼接文件的完整路径
        sprintf(path, "%s/%s", dirname, name);
        printf("path = %s ===================\n", path);
        struct stat st;
        int ret = stat(path, &st);
        if (ret < 0){
            std::cout<<"send_dir stat err!"<<std::endl;
            exit(1);
        }
        // 如果是文件
        if(S_ISREG(st.st_mode)){
            sprintf(dir_buffer + strlen(dir_buffer),
                    "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>",
                    name, name, (long)st.st_size);
        }
            // 如果是目录
        else if(S_ISDIR(st.st_mode)){
            sprintf(dir_buffer + strlen(dir_buffer),
                    "<tr><td><a href=\"%s/\">%s/</a></td><td>%ld</td></tr>",
                    name, name, (long)st.st_size);
        }
        // send(cfd, dir_buffer, strlen(dir_buffer), 0);
        // memset(buffer, 0, sizeof(buffer));
        // 字符串拼接
    }
    sprintf(dir_buffer+strlen(dir_buffer), "</table></body></html>");
    sprintf(buffer + strlen(buffer), "Content-Length: %d\r\n\r\n", strlen(dir_buffer));
    ::send(cfd, buffer, strlen(buffer), 0);
    ::send(cfd, dir_buffer, strlen(dir_buffer), 0);
    printf("dir message send OK!!!!\n");
}

