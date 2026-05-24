#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include "packet_handler.h"
#include "history.h"
/* 测试自己持有，不要 extern capture.c 里的 static 变量 */
static handler_ctx_t g_handler_ctx;
static history_t g_history;
static handler_user_t user;

/* 模拟调用 packet_handler */
static void fake_packet(uint32_t src, uint32_t dst, pkt_direction_t dir)
{
    parsed_packet_t pkt;
    memset(&pkt, 0, sizeof(pkt));

    pkt.src_ip = src;
    pkt.dst_ip = dst;
    pkt.src_port = 1234;
    pkt.dst_port = 80;
    pkt.protocol = 6;
    pkt.direction = dir;

    history_update_flow(&g_history, &pkt, 100);
}

int main()
{
    /* 初始化上下文（模拟 capture_init 做的事） */
    memset(&g_handler_ctx, 0, sizeof(g_handler_ctx));
    g_handler_ctx.local_ipv4 = inet_addr("192.168.1.1");
    g_handler_ctx.forward_mode = 1;

    history_init(&g_history);

    user.ctx = &g_handler_ctx;
    user.hist = &g_history;

    /* 模拟流量 */
    fake_packet(inet_addr("10.0.0.1"), inet_addr("192.168.1.1"), DIR_INBOUND);
    fake_packet(inet_addr("192.168.1.1"), inet_addr("10.0.0.1"), DIR_OUTBOUND);
    fake_packet(inet_addr("10.0.0.2"), inet_addr("192.168.1.1"), DIR_INBOUND);

    history_dump(&g_history);

    return 0;
}