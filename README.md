# 基于 OpenWrt 的迷你网络管理系统

这是一个运行在 OpenWrt 平台上的轻量级网络管理系统，提供 Web 可视化界面，实现网络流量监控与防火墙管理等功能。

## 功能特性

### 流量监控
支持实时监测网络流量信息，包括：

- 源 IP 地址 / 目的 IP 地址
- 累计发送流量与接收流量
- 实时流量速率
- 流量峰值统计
- 指定时间窗口内的平均流量

### 防火墙管理
支持通过 Web 页面进行防火墙配置与管理，包括：

- 查看当前防火墙规则状态
- 新增防火墙规则
- 删除防火墙规则
- 清空防火墙规则
- 防火墙规则生效验证

## 项目结构
.
├── project1
│   ├── CMakeLists.txt
│   ├── include  // 对外接口定义
│   │   ├── capture.h
│   │   ├── config.h
│   │   ├── history.h
│   │   ├── iface_info.h
│   │   ├── logger.h
│   │   └── packet_handler.h
│   ├── src
│   │   ├── capture.c // 捕获模块
│   │   ├── config.c // 配置文件
│   │   ├── history.c  // 历史记录模块
│   │   ├── iface_info.c  // 地址信息获取模块
│   │   ├── logger.c // 自定义日志
│   │   ├── main.c  // 入口
│   │   └── packet_handler.c  // 包处理模块
│   └── tests // 模块测试
├── README.md
└── toolchain.cmake



## ！！注意！！
- 每台机器使用的网卡可能不同，运行前请查看网卡类型，命令如下：
``` bash
ip link
```
获得网卡类型后，在运行文件的命令行中加入 ** -i 网卡类型 **,如：
``` bash
./netmon -i wlo1
```

## 运行说明
本项目采用ubuntu与openwrt交叉编译，实际运行的文件需放在src文件夹下，测试文件放在tests下（！！！务必！！！）。
### 将c文件编译成openwrt可运行文件

``` bash
cd project1
mkdir -p build && cd build  #可以省略，但建议加上（因为make后会生成大量文件）。
cmake .. -DCMAKE_TOOLCHAIN_FILE=../../toolchain.cmake
make netmon
```





