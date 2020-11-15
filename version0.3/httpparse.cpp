//
// Created by wuyao on 11/1/20.
//

#include "include/utils.h"
#include "include/httpparse.h"
#include <string.h>
#include <algorithm>
#include <iostream>

std::unordered_map<std::string, http::HttpRequest::HTTP_HEADER> http::HttpRequest::header_mapping = {
        {"HOST",                      http::HttpRequest::Host},
        {"USER-AGENT",                http::HttpRequest::User_Agent},
        {"CONNECTION",                http::HttpRequest::Connection},
        {"ACCEPT-ENCODING",           http::HttpRequest::Accept_Encoding},
        {"ACCEPT-LANGUAGE",           http::HttpRequest::Accept_Language},
        {"ACCEPT",                    http::HttpRequest::Accept},
        {"CACHE-CONTROL",             http::HttpRequest::Cache_Control},
        {"UPGRADE-INSECURE-REQUESTS", http::HttpRequest::Upgrade_Insecure_Requests}
};

//解析每一行的内容
http::HttpParse::LINE_STATUS
http::HttpParse::parse_line(char* line, int& check_index, int& read_index){
    char temp;
    for (; check_index < read_index; ++check_index){
        temp = line[check_index];
        if (temp == CR){
            if (check_index + 1 == read_index)
                return LINE_MORE;
            else if (check_index + 1 < read_index && line[check_index + 1] == LF){
                line[check_index ++] = LINE_END;
                line[check_index ++] = LINE_END;
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if (temp == LF){
            if (check_index > 1 && line[check_index - 1] == CR){
                line[check_index - 1] = LINE_END;
                line[check_index++] = LINE_END;
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    return LINE_MORE;
}

http::HttpParse::HTTP_CODE
http::HttpParse::parse_requestline(char* line, PARSE_STATUS& status, HttpRequest& request){
    char *url = strpbrk(line, " \t");
    if (url == nullptr) return BAD_REQUEST;         // 若无" \t"则说明请求出错
    *url++ = '\0';
    char* method = line;
    if (strcasecmp(method, "GET") == 0)
        request.m_method = http::HttpRequest::GET;
    else if (strcasecmp(method, "POST") == 0)
        request.m_method = http::HttpRequest::POST;
    else{
        std::cout<<"parse_requestline error file <"<<__FILE__<<" > at"<<__LINE__<<std::endl;
        exit(1);
        //return BAD_REQUEST;
    }
    //跳过前面的非空格与\t
    url += strspn(url, " \t");
    char *version = strpbrk(url, " \t");
    if (version == nullptr) return BAD_REQUEST;
    *version++ = '\0';
    version += strspn(version, " \t");
    if (strcasecmp(version, "HTTP/1.0") == 0)
        request.m_version = http::HttpRequest::HTTP_10;
    else if (strcasecmp(version, "HTTP/1.1") == 0)
        request.m_version = http::HttpRequest::HTTP_11;
    else{
        std::cout<<"version error file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        return BAD_REQUEST;
    }
    // 检查URL是否合法
    if (strncasecmp(url, "http://", 7) == 0){
        url += 7;
        url = strchr(url, '/');
    }
    else if(strncasecmp(url, "/", 1) == 0)
            PASS;
    else{
        std::cout<<"URL error file <"<<__FILE__<<"> at"<<__LINE__<<std::endl;
        return BAD_REQUEST;
    }

    if (!url || *url != '/')
        return BAD_REQUEST;

    /*
   std::string tmp_url(url);
   if (tmp_url == "/")
       tmp_url = "./";
   */
    request.m_uri = std::string(url);

    // 状态转移至请求头
    status = http::HttpParse::PARSE_REQUEST_HEAD;
    // 只是分析完了 http的请求行 还需继续分析请求头
    return NO_REQUEST;
}

http::HttpParse::HTTP_CODE
http::HttpParse::parse_header(char* buffer, PARSE_STATUS& status, HttpRequest& requset){
    // 已经处理到空行来了(header与body之间)
    if (buffer[0] == LINE_END) {
        if (requset.m_method == http::HttpRequest::GET)
            return GET_REQUEST;
        status = PARSE_REQUEST_BODY;
        return NO_REQUEST;
    }
    // chrome 发送请求的时候可能溢出
    char key[216] = {0}, val[512] = {0};
    sscanf(buffer, "%[^:]:%[^:]", key, val);
    std::string cur_key(key);
    std::string cur_val(val);
    // 将cur_key全部转化成大写字母
    std::transform(cur_key.begin(), cur_key.end(), cur_key.begin(), ::toupper);
    auto iter = http::HttpRequest::header_mapping.find(cur_key);
    if (iter != http::HttpRequest::header_mapping.end())
        requset.m_headers.insert(std::make_pair(iter->second, utils::trim(cur_val)));
    else
        std::cout<<"header not support: "<<cur_key <<": "<<val<<std::endl;
    return NO_REQUEST;
}

// 解析完body ==> 完成解析
http::HttpParse::HTTP_CODE
http::HttpParse::parse_body(char* buffer, HttpRequest& request){
    request.m_content = buffer;
    return GET_REQUEST;
}

http::HttpParse::HTTP_CODE
http::HttpParse::parse_content(char* buffer, int& check_index, PARSE_STATUS& state, int& read_index, int& start_line, HttpRequest& request){
    LINE_STATUS line_state = LINE_OK;
    HTTP_CODE ret_code = NO_REQUEST;
    while((line_state = parse_line(buffer, check_index,read_index)) == LINE_OK){
        char *temp = buffer + start_line;
        start_line = check_index;
        switch(state){
            case PARSE_REQUEST_LINE:{
                ret_code = parse_requestline(temp, state, request);
                if (ret_code == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            }
            case PARSE_REQUEST_HEAD:{
                ret_code = parse_header(temp, state, request);
                if (ret_code == BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret_code == GET_REQUEST)
                    return GET_REQUEST;
                break;
            }
            case PARSE_REQUEST_BODY:{
                ret_code = parse_body(temp, request);
                break;
            }
            default:
                return INTERNAL_ERROR;
        }
    }
    if (line_state == LINE_MORE)   return NO_REQUEST;
    return BAD_REQUEST;
}

std::ostream& http::operator<<(std::ostream& os, const http::HttpRequest& request){
    if (request.m_method == http::HttpRequest::GET)
        os<<"GET ";
    else if (request.m_method == http::HttpRequest::POST)
        os<<"POST ";
    os << request.m_uri;
    if (request.m_version == http::HttpRequest::HTTP_10)
        os<<"HTTP/1.0 "<<std::endl;
    else
        os<<"HTTP/1.1 "<<std::endl;
    for (auto iter = request.m_headers.begin(); iter != request.m_headers.end(); ++iter)
        os<<iter->first<<" : "<<iter->second<<std::endl;
    return os;
}
