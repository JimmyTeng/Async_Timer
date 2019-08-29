cmake_minimum_required(VERSION 3.14)
project(async_timer)

set(CMAKE_CXX_STANDARD 14)

add_executable(async_timer main.cpp)
target_link_libraries(async_timer pthread p7  dl rt)