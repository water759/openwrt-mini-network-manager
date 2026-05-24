#ifndef CONFIG_H
#define CONFIG_H

#include <pcap.h>

typedef struct {
// pcap_t*pcap_open_live
// (const char* device, int snaplen, int promisc, int to_ms, char* errbuf);
// 1. device: 网络接口名称
// 2. snaplen: 抓包长度
// 3. promisc: 是否混杂模式
// 4. to_ms: 超时时间
// 5. errbuf: 错误信息缓冲区
    char device[64];
    int snaplen;
    int promisc;
    int to_ms;
//int pcap_loop(pcap_t*p,intcnt,pcap_handlercallback,unsignedchar*user);
// 1. p: 抓包句柄
// 2. cnt: 抓包次数
// 3. callback: 回调函数
// 4. user: 用户数据
    int cnt;

//intpcap_compile
// (pcap_t*p,structbpf_program*fp,constchar*str,intoptimize,bpf_u_int32netmask);
// 1. p: 抓包句柄
// 2. fp: 过滤器程序
// 3. str: 过滤器表达式
// 4. optimize: 是否优化
// 5. netmask: 网络掩码
    char filter_str[64];
    int optimize;

} Config;

void load_config();
extern Config g_config;

#endif