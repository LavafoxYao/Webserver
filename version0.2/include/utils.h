//
// Created by wuyao on 10/28/20.
//

#ifndef WEBSERVER_UTILS_H
#define WEBSERVER_UTILS_H
#include <string.h>
#include <string>


namespace utils{
    std::string& ltrim(std::string& str);
    std::string& rtrim(std::string& str);
    std::string& trim(std::string& str);
} // utils


#endif //WEBSERVER_UTILS_H
