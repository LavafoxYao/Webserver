//
// Created by wuyao on 11/1/20.
//

#ifndef VERSION0_3_HTTPRESPONSE_H
#define VERSION0_3_HTTPRESPONSE_H
#include "httpparse.h"
#include <unordered_map>

namespace http {
    struct Mime_type {
        Mime_type(const char *ptr) : m_type(ptr) {};

        Mime_type(const std::string &s) : m_type(s) {};
        std::string m_type;
    };

    extern std::unordered_map<std::string, Mime_type> Mime_Map;

    class HttpResponse {
    public:
        enum HTTP_STATE_CODE {
            UNKNOW = 0, k200OK = 200, k403Forbidden = 403, k404Notfound = 404
        };

        explicit HttpResponse(bool keep_alive = true) : m_state_code(UNKNOW), m_state_msg(""),
                                            m_version(http::HttpRequest::VERSION_NOT_SUPPORT), m_type("text/plain"),
                                            m_content(nullptr),
                                            m_keep_alive(keep_alive) {};

        ~HttpResponse() {
            if (m_content != nullptr)
                delete[] m_content;
        }

        void set_state_code(const HTTP_STATE_CODE state) {
            m_state_code = state;
        }

        void set_state_msg(const std::string &str) {
            m_state_msg = str;
        }

        void set_http_version(const http::HttpRequest::HTTP_VERSION version) {
            m_version = version;
        }

        void set_keep_alive(bool state) {
            m_keep_alive = state;
        }

        bool get_keep_alive(){
            return m_keep_alive;
        }

        void set_content(const char *ptr) {
            m_content = ptr;
        }

        void set_content_len(const int len) {
            m_content_len = len;
        }

        void set_mimetype(const Mime_type &type) {
            m_type = type;
        }

        void set_filepath(const std::string &filepath) {
            m_file_path = filepath;
        }

        HTTP_STATE_CODE get_state_code() const {
            return m_state_code;
        }

        const std::string &get_state_msg() const {
            return m_state_msg;
        }

        const HttpRequest::HTTP_VERSION get_version() const {
            return m_version;
        }

        const Mime_type get_mimetype() const {
            return m_type;
        }

        const char *get_content() const {
            return m_content;
        }

        int get_content_len() const {
            return m_content_len;
        }

        const std::string &get_filepath() const {
            return m_file_path;
        }

        void append_header(const std::string &key, const std::string &val) {
            m_header[key] = val;
        }

        void response_header(char *buffer);

        void print_header(HttpResponse& ) const;


    private:
        HTTP_STATE_CODE m_state_code;
        std::string m_state_msg;
        http::HttpRequest::HTTP_VERSION m_version;
        bool m_keep_alive;
        Mime_type m_type;
        const char *m_content;
        int m_content_len;
        std::string m_file_path;
        std::unordered_map<std::string, std::string> m_header;
    };
}   //http
#endif //VERSION0_3_HTTPRESPONSE_H
