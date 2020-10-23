//
// Created by wuyao on 10/22/20.
//

#ifndef WEBSERVER_2_HTTPPARSE_H
#define WEBSERVER_2_HTTPPARSE_H

#include <unordered_map>
#include <iostream>
#include <string>

#define CR '\r'
#define LF '\n'
#define LINE_END '\0'
#define PASS

namespace http{
    class HttpRequest;
    std::ostream& operator << (std::ostream&,  const HttpRequest&);
    struct HttpParse{
        enum PARSE_STATUS{PARSE_REQUEST_LINE = 0, PARSE_REQUEST_HEAD, PARSE_REQUEST_BODY};
        enum LINE_STATUS{LINE_OK = 0, LINE_BAD, LINE_MORE};
        // http_code 表示http处理http请求的结果
        enum HTTP_CODE{NO_REQUEST = 0, GET_REQUEST, BAD_REQUEST, FORBIDDEN_REQUEST,
                INTERNAL_ERROR, CLOSED_CONNECTION};
        static LINE_STATUS parse_line(char* line, int& check_index, int& read_index);
        static HTTP_CODE parse_requestline(char* line, PARSE_STATUS& status, HttpRequest& request);
        static HTTP_CODE parse_header(char* buffer, PARSE_STATUS& status, HttpRequest& requset);
        static HTTP_CODE parse_body(char* buffer, HttpRequest& request);
        static HTTP_CODE parse_content(char* buffer, int& check_index, PARSE_STATUS& state, int& read_index, int& start_line, HttpRequest& request);
    };

    class HttpRequest{
    public:
        friend std::ostream& operator<<(std::ostream&, const HttpRequest&);
        enum HTTP_METHOD{GET = 0, PUT, POST, DELETE, METHOD_NOT_SUPPORT};
        enum HTTP_VERSION {HTTP_10 = 0, HTTP_11, VERSION_NOT_SUPPORT};
        enum HTTP_HEADER{Host = 0, User_Agent, Connection, Accept_Encoding, Accept_Language, Accept, Cache_Control, Upgrade_Insecure_Requests};
        static std::unordered_map<std::string, HTTP_HEADER> header_mapping;

        // 做了简单的优化，将hashfunction尽可能的简单点
        struct HttpHashFunc{
            template<typename T>
            std::size_t operator()(T t) const{
                return static_cast<std::size_t>(t);
            }
        };
        HttpRequest(HTTP_METHOD method = METHOD_NOT_SUPPORT,
                    std:: string url = "", HTTP_VERSION version = VERSION_NOT_SUPPORT)
                :m_method(method), m_uri(url), m_version(version), m_content(nullptr),
                m_headers(std::unordered_map<HTTP_HEADER, std::string, HttpHashFunc>()){};

        ~HttpRequest(){
            if (m_content != nullptr)
                delete[] m_content;
        }
    public:
        HTTP_METHOD m_method;
        HTTP_VERSION m_version;
        std::string m_uri;
        char* m_content;
        // 存储该请求头中键值对
        std::unordered_map<HTTP_HEADER, std::string, HttpHashFunc> m_headers;
    };
}

#endif //WEBSERVER_2_HTTPPARSE_H
