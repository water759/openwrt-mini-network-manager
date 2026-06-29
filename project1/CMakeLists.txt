cmake_minimum_required(VERSION 3.10)
project(traffic_monitoring)

set(CMAKE_C_STANDARD 11)

file(GLOB SOURCES src/*.c)
add_executable(netmon ${SOURCES})

target_include_directories(netmon PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# 交叉编译 OpenWrt：在 sysroot 中查找 libpcap
if(CMAKE_CROSSCOMPILING)
    find_path(PCAP_INCLUDE_DIR
        NAMES pcap.h
        PATHS
            ${CMAKE_FIND_ROOT_PATH}/usr/include
            ${CMAKE_FIND_ROOT_PATH}/include
        NO_DEFAULT_PATH
    )
    find_library(PCAP_LIBRARY
        NAMES pcap
        PATHS
            ${CMAKE_FIND_ROOT_PATH}/usr/lib
            ${CMAKE_FIND_ROOT_PATH}/lib
        NO_DEFAULT_PATH
    )
    if(NOT PCAP_INCLUDE_DIR OR NOT PCAP_LIBRARY)
        message(FATAL_ERROR
            "libpcap not found in OpenWrt sysroot: ${CMAKE_FIND_ROOT_PATH}\n"
            "Install libpcap into target-aarch64_generic_musl/usr first "
            "(SDK: make package/libs/libpcap/compile, or build from source with env-arm.sh).")
    endif()
    target_include_directories(netmon PRIVATE ${PCAP_INCLUDE_DIR})
    target_link_libraries(netmon ${PCAP_LIBRARY})
else()
    # 本机 Ubuntu
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(PCAP REQUIRED libpcap)
    target_include_directories(netmon PRIVATE ${PCAP_INCLUDE_DIRS})
    target_link_libraries(netmon ${PCAP_LIBRARIES})
endif()