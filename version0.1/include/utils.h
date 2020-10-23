//
// Created by wuyao on 10/22/20.
//

#ifndef WEBSERVER_2_UTILS_H
#define WEBSERVER_2_UTILS_H
#include <string>

namespace utils{
    std::string& ltrim(std::string& str);
    std::string& rtrim(std::string& str);
    std::string& trim(std::string& str);
    void decode_str(char *to, char *from);
    void encode_str(char* to, int tosize, const char* from);
    int hexit(char c);
}

#endif //WEBSERVER_2_UTILS_H
