cmake_minimum_required(VERSION 3.17)
project(webserver_2)

set(CMAKE_CXX_STANDARD 14)

add_executable(webserver_2 main.cpp include/nsock.h nsock.cpp include/server.h include/httpparse.h httpparse.cpp server.cpp include/utils.h utils.cpp include/httpresponse.h httpresponse.cpp)