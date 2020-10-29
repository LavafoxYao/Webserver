//
// Created by wuyao on 10/28/20.
//
#include "include/utils.h"

// 删除字符串前面的空格与\t
std::string& utils::ltrim(std::string& str){
    if (str.size() == 0)    return str;
    int ret = str.find_first_not_of(" \t");
    if (ret == std::string::npos)   return str;
    str.erase(0, ret);
    return str;
}

// 删除字符串后面的空格与\t
std::string& utils::rtrim(std::string& str){
    if (str.size() == 0)    return str;
    int ret = str.find_last_not_of(" \t");
    if (ret == std::string::npos) return str;
    str.erase(ret + 1);
    return str;
}

std::string& utils::trim(std::string& str){
    if (str.size() == 0)    return str;
    ltrim(str);
    rtrim(str);
    return str;
}
