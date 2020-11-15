//
// Created by wuyao on 11/1/20.
//
#include "include/httpresponse.h"
#include "include/utils.h"
#include <cstring>

std::unordered_map<std::string, http::Mime_type> http::Mime_Map{
        {".html", "text/html"},
        {".xml", "text/xml"},
        {".xhtml", "application/xhtml+xml"},
        {".txt", "text/plain"},
        {".rtf", "application/rtf"},
        {".pdf", "application/pdf"},
        {".word", "application/msword"},
        {".png", "image/png"},
        {".gif", "image/gif"},
        {".jpg", "image/jpeg"},
        {".jpeg", "image/jpeg"},
        {".au", "audio/basic"},
        {".mpeg", "video/mpeg"},
        {".mpg", "video/mpeg"},
        {".avi", "video/x-msvideo"},
        {".gz", "application/x-gzip"},
        {".tar", "application/x-tar"},
        {".css", "text/css"},
        {"", "text/plain"},
        {"default","text/plain"},
        {"./", "text/html"}
};

void http::HttpResponse::response_header(char *buffer){
    if (m_version == HttpRequest::HTTP_10)
        sprintf(buffer, "HTTP/1.0 %d %s\r\n", m_state_code, m_state_msg.c_str());
    else
        sprintf(buffer, "HTTP/1.1 %d %s\r\n", m_state_code, m_state_msg.c_str());

    for (const auto& iter : m_header)
        sprintf(buffer + strlen(buffer), "%s: %s\r\n", iter.first.c_str(), iter.second.c_str());
    sprintf(buffer + strlen(buffer), "Content-Type: %s\r\n", m_type.m_type.c_str());
    if (!m_keep_alive)
        sprintf(buffer + strlen(buffer), "Connection: close\r\n");
    else
        sprintf(buffer + strlen(buffer), "Connection: keep-alive\r\n");
}


void http::HttpResponse::print_header(http::HttpResponse& response) const{
    std::cout<<"<======= print response header =======>"<<std::endl;
    for (auto iter = m_header.begin(); iter != m_header.end(); ++iter)
        std::cout<<iter->first<<" : "<<iter->second<<std::endl;
    std::cout<<"<======= response header end =======>"<<std::endl;
}


