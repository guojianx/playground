cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -O0")
project(tcp_server)
add_executable(tcp_server tcp_server.cc tcp.cc)
