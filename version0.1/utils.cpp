//
// Created by wuyao on 10/22/20.
//
#include "include/utils.h"
#include <cstring>

std::string& utils::ltrim(std::string& str){
    if (str.empty())    return str;
    int res = str.find_first_not_of(" \t",0);
    if (res == std::string::npos)   return str;
    str.erase(str.begin(), str.begin() + res);
    return str;
}

std::string& utils::rtrim(std::string& str){
    if (str.empty())    return str;
    int res = str.find_last_not_of(" \t");
    if (res == std::string::npos)   return str;
    str.erase(res + 1);
    return str;
}

std::string& utils::trim(std::string& str){
    if (str.empty())    return str;
    ltrim(str);
    rtrim(str);
    return str;
}
// 16进制数转化为10进制
int utils::hexit(char c){
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

/*
 *  这里的内容是处理%20之类的东西！是"解码"过程。
 *  %20 URL编码中的‘ ’(space)
 *  %21 '!' %22 '"' %23 '#' %24 '$'
 *  %25 '%' %26 '&' %27 ''' %28 '('......
 */
void utils::encode_str(char* to, int tosize, const char* from){
    int tolen;
    for (tolen = 0; *from != '\0' && tolen + 4 < tosize; ++from) {
        if (isalnum(*from) || strchr("/_.-~", *from) != (char*)0) {
            *to = *from;
            ++to;
            ++tolen;
        }
        else{
            // %%%02x   前两个%%是为了输出%  (int) *from & 0xff ==> 十进制转16进制
            sprintf(to, "%%%02x", (int) *from & 0xff);
            to += 3;
            tolen += 3;
        }
    }
    *to = '\0';
}

void utils::decode_str(char *to, char *from){
    for ( ; *from != '\0'; ++to, ++from  ){
        if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2])) {
            *to = hexit(from[1])*16 + hexit(from[2]);
            from += 2;
        }
        else{
            *to = *from;
        }
    }
    *to = '\0';
}
