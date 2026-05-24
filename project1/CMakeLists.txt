cmake_minimum_required(VERSION 3.10)
project(traffic_monitoring)
set(CMAKE_C_STANDARD 11)
include_directories(
${CMAKE_FIND_ROOT_PATH}/usr/include
)

#---------正式程序-----------------
file(GLOB SOURCES src/*.c)
add_executable(lab2 ${SOURCES})
target_link_libraries(lab2
pthread
${CMAKE_FIND_ROOT_PATH}/usr/lib/libpcap.so
)

#-------------模块测试---------------
add_executable(test tests/test.c)
target_include_directories(test PRIVATE ${CMAKE_FIND_ROOT_PATH}/usr/include)
target_link_libraries(test pthread ${CMAKE_FIND_ROOT_PATH}/usr/lib/libpcap.so)