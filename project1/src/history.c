// 流量历史统计：按五元组（源/目的 IP、端口、协议）维护固定大小的流表，
// 累计每条流的收发包数/字节数，并维护全局合计。
#include "history.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

// 在流表中查找与 pkt 匹配的五元组；不存在则占用第一个空闲槽位并初始化
static flow_entry_t *find_or_create_flow(history_t *hist,
                                         const parsed_packet_t *pkt)
{
    flow_entry_t *empty = NULL;

    for (int i = 0; i < MAX_FLOWS; i++) {
        flow_entry_t *e = &hist->flows[i];
        if (!e->used) {
            if (!empty)
                empty = e;
            continue;
        }

        if (e->src_ip == pkt->src_ip &&
            e->dst_ip == pkt->dst_ip &&
            e->src_port == pkt->src_port &&
            e->dst_port == pkt->dst_port &&
            e->protocol == pkt->protocol) {
            return e;
        }
    }

    // 表满时 empty 仍为 NULL
    if (!empty)
        return NULL;

    memset(empty, 0, sizeof(*empty));
    empty->used = 1;
    empty->src_ip = pkt->src_ip;
    empty->dst_ip = pkt->dst_ip;
    empty->src_port = pkt->src_port;
    empty->dst_port = pkt->dst_port;
    empty->protocol = pkt->protocol;
    return empty;
}

void history_init(history_t *hist)
{
    if (hist)
        memset(hist, 0, sizeof(*hist));
}

// 将本包字节数计入对应流及全局统计；方向由 pkt->direction 区分入站/出站
void history_update_flow(history_t *hist,
                         const parsed_packet_t *pkt,
                         uint32_t bytes)
{
    if (!hist || !pkt)
        return;

    flow_entry_t *flow = find_or_create_flow(hist, pkt);
    if (!flow) {
        log_error("flow table full");
        return;
    }

    if (pkt->direction == DIR_INBOUND) {
        flow->rx_bytes += bytes;
        flow->rx_pkts += 1;
        hist->total_rx_bytes += bytes;
        hist->total_rx_pkts += 1;
    } else {
        flow->tx_bytes += bytes;
        flow->tx_pkts += 1;
        hist->total_tx_bytes += bytes;
        hist->total_tx_pkts += 1;
    }
}

// 网络字节序 IPv4 转点分十进制字符串
static void ip_to_str(uint32_t ip, char *buf, size_t len)
{
    struct in_addr addr;
    addr.s_addr = ip;
    inet_ntop(AF_INET, &addr, buf, len);
}

// 输出全局合计及所有已用流表项（经 logger）
void history_dump(const history_t *hist)
{
    char sip[16], dip[16];

    if (!hist)
        return;

    log_info("=== totals rx=%llu/%llu tx=%llu/%llu ===",
             (unsigned long long)hist->total_rx_bytes,
             (unsigned long long)hist->total_rx_pkts,
             (unsigned long long)hist->total_tx_bytes,
             (unsigned long long)hist->total_tx_pkts);

    for (int i = 0; i < MAX_FLOWS; i++) {
        const flow_entry_t *e = &hist->flows[i];
        if (!e->used)
            continue;

        ip_to_str(e->src_ip, sip, sizeof(sip));
        ip_to_str(e->dst_ip, dip, sizeof(dip));

        log_info("flow %s:%u -> %s:%u proto=%u rx=%llu/%llu tx=%llu/%llu",
                 sip, e->src_port, dip, e->dst_port, e->protocol,
                 (unsigned long long)e->rx_bytes,
                 (unsigned long long)e->rx_pkts,
                 (unsigned long long)e->tx_bytes,
                 (unsigned long long)e->tx_pkts);
    }
}
