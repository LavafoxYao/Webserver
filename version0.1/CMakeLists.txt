cmake_minimum_required(VERSION 3.17)
project(version0_1)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(include)
include_directories(res)

add_executable(version0_1
        include/httpparse.h
        include/httpresponse.h
        include/nsock.h
        include/server.h
        include/utils.h
        res/403.html
        res/404.html
        res/index.html
        httpparse.cpp
        httpresponse.cpp
        main.cpp
        nsock.cpp
        server.cpp
        utils.cpp)
