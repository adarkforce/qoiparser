cmake_minimum_required(VERSION 3.21)
project(qoiparser)

set(CMAKE_CXX_STANDARD 17)

add_executable(qoiparser main.cpp)

target_include_directories(qoiparser PRIVATE "./qoiparser"
                                             "./external")
