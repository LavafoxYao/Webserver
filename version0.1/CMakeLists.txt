cmake_minimum_required(VERSION 3.17)
project(version0.1)

set(CMAKE_CXX_STANDARD 14)

add_executable(version0.1 main.cpp include/nsock.h nsock.cpp include/server.h include/httpparse.h httpparse.cpp server.cpp include/utils.h utils.cpp include/httpresponse.h httpresponse.cpp)