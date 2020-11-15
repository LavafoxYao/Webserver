cmake_minimum_required(VERSION 3.17)
project(version0_3)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

add_executable(version0_3 main.cpp include/utils.h utils.cpp include/noncopyable.h include/mutexlock.h include/condition.h include/threadpool.h threadpool.cpp include/Socket.h Socket.cpp include/httpparse.h httpparse.cpp include/httpresponse.h httpresponse.cpp include/Timer.h include/httpdata.h Timer.cpp httpdata.cpp include/Epoll.h Epoll.cpp include/Server.h Server.cpp)