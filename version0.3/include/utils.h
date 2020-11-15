//
// Created by wuyao on 10/31/20.
//

#ifndef VERSION0_3_UTILS_H
#define VERSION0_3_UTILS_H
#include <string>

namespace utils{
    std::string& ltrim(std::string& str);
    std::string& rtrim(std::string& str);
    std::string& trim(std::string& str);
    int hexit(char c);
    void encode_str(char* to, int tosize, const char* from);
    void decode_str(char *to, char *from);
    void set_reuse_port(int fd);
    int set_nonblocking(int fd);
    int check_base_path(char *basePath);
    void handle_sigpipe();
    void daemon_run();
} //utils

#endif //VERSION0_3_UTILS_H
