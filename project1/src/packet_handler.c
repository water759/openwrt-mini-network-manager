#include "packet_handler.h"
#include "history.h"
#include "logger.h"

#include <string.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>

#define ETH_HLEN 14
#define IP_HLEN_MIN 20

static int is_packet_length_valid(const struct pcap_pkthdr *hdr,
                                  const u_char *pkt,
                                  int dlt)
{
    if (!hdr || !pkt)
        return 0;
    if (hdr->caplen == 0 || hdr->caplen > hdr->len)
        return 0;

    if (dlt == DLT_EN10MB)
        return hdr->caplen >= ETH_HLEN + IP_HLEN_MIN;

    return hdr->caplen >= IP_HLEN_MIN;
}

static const u_char *get_l3_start(const u_char *pkt, int dlt, uint16_t *ethertype)
{
    if (dlt != DLT_EN10MB)
        return NULL;

    const struct ether_header *eth = (const struct ether_header *)pkt;
    *ethertype = ntohs(eth->ether_type);
    if (*ethertype != ETHERTYPE_IP)
        return NULL;

    return pkt + ETH_HLEN;
}

static int parse_ipv4_header(const u_char *l3, uint32_t caplen,
                             const struct iphdr **out_ip,
                             uint32_t *ip_hdr_len)
{
    if (caplen < sizeof(struct iphdr))
        return -1;

    const struct iphdr *ip = (const struct iphdr *)l3;
    if (ip->version != 4)
        return -1;

    *ip_hdr_len = (uint32_t)ip->ihl * 4;
    if (*ip_hdr_len < IP_HLEN_MIN || caplen < *ip_hdr_len)
        return -1;

    *out_ip = ip;
    return 0;
}

static int ip_belongs_to_router(uint32_t ip, const handler_ctx_t *ctx)
{
    return ctx->local_ipv4 != 0 && ip == ctx->local_ipv4;
}

static pkt_direction_t direction_by_link_layer(const u_char *pkt,
                                               const handler_ctx_t *ctx)
{
    if (ctx->dlt != DLT_EN10MB)
        return DIR_UNKNOWN;

    const struct ether_header *eth = (const struct ether_header *)pkt;

    if (memcmp(eth->ether_shost, ctx->iface.mac, 6) == 0)
        return DIR_OUTBOUND;
    if (memcmp(eth->ether_dhost, ctx->iface.mac, 6) == 0)
        return DIR_INBOUND;

    return DIR_UNKNOWN;
}

static pkt_direction_t direction_by_ip_layer(const u_char *pkt,
                                             uint32_t caplen,
                                             const handler_ctx_t *ctx)
{
    uint16_t ethertype = 0;
    const u_char *l3 = get_l3_start(pkt, ctx->dlt, &ethertype);
    if (!l3)
        return DIR_UNKNOWN;

    const struct iphdr *ip = NULL;
    uint32_t ip_hdr_len = 0;
    if (parse_ipv4_header(l3, caplen - (uint32_t)(l3 - pkt), &ip, &ip_hdr_len) != 0)
        return DIR_UNKNOWN;

    if (ip_belongs_to_router(ip->saddr, ctx))
        return DIR_OUTBOUND;
    if (ip_belongs_to_router(ip->daddr, ctx))
        return DIR_INBOUND;

    return DIR_UNKNOWN;
}

static pkt_direction_t direction_by_iface_default(const u_char *pkt,
                                                  uint32_t caplen,
                                                  const handler_ctx_t *ctx)
{
    pkt_direction_t dir = direction_by_link_layer(pkt, ctx);
    if (dir != DIR_UNKNOWN)
        return dir;

    return direction_by_ip_layer(pkt, caplen, ctx);
}

static pkt_direction_t direction_fallback_by_addr(const u_char *pkt,
                                                  uint32_t caplen,
                                                  const handler_ctx_t *ctx)
{
    uint16_t ethertype = 0;
    const u_char *l3 = get_l3_start(pkt, ctx->dlt, &ethertype);
    if (!l3)
        return DIR_INBOUND;

    const struct iphdr *ip = NULL;
    uint32_t ip_hdr_len = 0;
    if (parse_ipv4_header(l3, caplen - (uint32_t)(l3 - pkt), &ip, &ip_hdr_len) != 0)
        return DIR_INBOUND;

    return (ip->saddr <= ip->daddr) ? DIR_INBOUND : DIR_OUTBOUND;
}

static pkt_scope_t classify_scope(uint32_t src_ip, uint32_t dst_ip,
                                  const handler_ctx_t *ctx)
{
    int src_local = ip_belongs_to_router(src_ip, ctx);
    int dst_local = ip_belongs_to_router(dst_ip, ctx);

    if (src_local && !dst_local)
        return PKT_FROM_ROUTER;
    if (dst_local)
        return PKT_TO_ROUTER;
    return PKT_TRANSIT;
}

static int parse_packet(const u_char *pkt, uint32_t caplen, int dlt,
                        parsed_packet_t *out)
{
    uint16_t ethertype = 0;
    const u_char *l3 = get_l3_start(pkt, dlt, &ethertype);
    if (!l3)
        return -1;

    const struct iphdr *ip = NULL;
    uint32_t ip_hdr_len = 0;
    if (parse_ipv4_header(l3, caplen - (uint32_t)(l3 - pkt), &ip, &ip_hdr_len) != 0)
        return -1;

    out->src_ip = ip->saddr;
    out->dst_ip = ip->daddr;
    out->protocol = ip->protocol;
    out->src_port = 0;
    out->dst_port = 0;

    const u_char *l4 = l3 + ip_hdr_len;
    uint32_t l4_len = caplen - (uint32_t)(l4 - pkt);

    if (ip->protocol == IPPROTO_TCP && l4_len >= sizeof(struct tcphdr)) {
        const struct tcphdr *tcp = (const struct tcphdr *)l4;
        out->src_port = ntohs(tcp->th_sport);
        out->dst_port = ntohs(tcp->th_dport);
    } else if (ip->protocol == IPPROTO_UDP && l4_len >= sizeof(struct udphdr)) {
        const struct udphdr *udp = (const struct udphdr *)l4;
        out->src_port = ntohs(udp->uh_sport);
        out->dst_port = ntohs(udp->uh_dport);
    }

    return 0;
}

static pkt_direction_t resolve_direction(const u_char *packet,
                                         uint32_t caplen,
                                         const handler_ctx_t *ctx)
{
    pkt_direction_t dir;

    /* 无过滤：网卡默认规则，未知则默认入站 */
    if (!ctx->has_filter) {
        dir = direction_by_iface_default(packet, caplen, ctx);
        return (dir != DIR_UNKNOWN) ? dir : DIR_INBOUND;
    }

    /* 有过滤：物理层 -> IP 层 */
    dir = direction_by_link_layer(packet, ctx);
    if (dir == DIR_UNKNOWN)
        dir = direction_by_ip_layer(packet, caplen, ctx);
    if (dir != DIR_UNKNOWN)
        return dir;

    /* 多地址场景 */
    if (ctx->multi_addr)
        return DIR_INBOUND;

    /* 转发模式：仍未知则丢弃 */
    if (ctx->forward_mode)
        return DIR_UNKNOWN;

    /* 兜底：按地址大小分配方向 */
    return direction_fallback_by_addr(packet, caplen, ctx);
}

void packet_handler(u_char *user,
                    const struct pcap_pkthdr *header,
                    const u_char *packet)
{
    handler_user_t *hu = (handler_user_t *)user;
    if (!hu || !hu->ctx || !header || !packet)
        return;

    handler_ctx_t *ctx = hu->ctx;

    if (!is_packet_length_valid(header, packet, ctx->dlt))
        return;

    pkt_direction_t dir = resolve_direction(packet, header->caplen, ctx);
    if (dir == DIR_UNKNOWN)
        return;

    parsed_packet_t info;
    memset(&info, 0, sizeof(info));

    if (parse_packet(packet, header->caplen, ctx->dlt, &info) != 0)
        return;

    info.direction = dir;
    info.scope = classify_scope(info.src_ip, info.dst_ip, ctx);

    if (hu->hist)
        history_update_flow(hu->hist, &info, header->caplen);
}