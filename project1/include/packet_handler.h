#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <pcap.h>
#include "iface_info.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    DIR_UNKNOWN = 0,
    DIR_INBOUND,    /* 相对监控网口的 rx */
    DIR_OUTBOUND,   /* 相对监控网口的 tx */
} pkt_direction_t; // 包方向

typedef enum {
    PKT_TRANSIT = 0, // 转发
    PKT_TO_ROUTER, // 发送到路由器
    PKT_FROM_ROUTER, // 从路由器接收
} pkt_scope_t; // 路由器作用    

typedef struct {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t  protocol;
    pkt_direction_t direction;
    pkt_scope_t     scope;
} parsed_packet_t;  // 解析后的包

typedef struct {
    char ifname[16];
    iface_info_t iface;
    int dlt;
    int has_filter;
    int forward_mode;
    int multi_addr;
    uint32_t local_ipv4;
} handler_ctx_t;  // 处理上下文

struct history;
typedef struct history history_t;  // 历史记录
typedef struct {
    handler_ctx_t *ctx;
    history_t *hist;
} handler_user_t;  // 处理用户



/*
*@Param user 用户自定义数据
*@Param header 包头（不是数据本身，用来统计流量）
*@Param packet 真正的数据
*/
void packet_handler(u_char *user,
    const struct pcap_pkthdr *header,
    const u_char *packet);

#ifdef __cplusplus
}
#endif

#endif