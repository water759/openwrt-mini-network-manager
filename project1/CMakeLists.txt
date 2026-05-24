cmake_minimum_required(VERSION 3.10)
project(traffic_monitoring)

set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES src/*.c)
add_executable(netmon ${SOURCES})

target_include_directories(netmon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 交叉编译 OpenWrt：用 sysroot 里的 libpcap
if(CMAKE_CROSSCOMPILING)
    target_include_directories(netmon PRIVATE
        ${CMAKE_FIND_ROOT_PATH}/usr/include
    )
    target_link_libraries(netmon
        ${CMAKE_FIND_ROOT_PATH}/usr/lib/libpcap.so
    )
else()
    # 本机 Ubuntu
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PCAP REQUIRED libpcap)
    target_link_libraries(netmon ${PCAP_LIBRARIES})
endif()