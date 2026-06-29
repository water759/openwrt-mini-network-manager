set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR mips)

set(OPENWRT_SDK "/home/water/下载/openwrt-sdk-24.10.4-ramips-mt7621_gcc-13.3.0_musl.Linux-x86_64")
set(STAGING_DIR "${OPENWRT_SDK}/staging_dir")

set(CMAKE_C_COMPILER "${STAGING_DIR}/toolchain-mipsel_24kc_gcc-13.3.0_musl/bin/mipsel-openwrt-linux-musl-gcc")
set(CMAKE_CXX_COMPILER "${STAGING_DIR}/toolchain-mipsel_24kc_gcc-13.3.0_musl/bin/mipsel-openwrt-linux-musl-g++")

set(CMAKE_FIND_ROOT_PATH "${STAGING_DIR}/target-mipsel_24kc_musl")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)