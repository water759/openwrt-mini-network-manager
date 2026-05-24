#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <pcap.h>
#include <signal.h>
#include "config.h"
#include "logger.h"
#include "capture.h"
#include "iface_info.h"
#include "packet_handler.h"
#include "history.h"

static history_t g_history;
static handler_ctx_t g_handler_ctx;
static handler_user_t g_handler_user;
static pcap_t *g_pcap_handle;
static void on_sigint(int sig)
{
    (void)sig;
    if (g_pcap_handle)
        pcap_breakloop(g_pcap_handle);
}

void capture_init(void)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(g_config.device,
                                    g_config.snaplen,
                                    g_config.promisc,
                                    g_config.to_ms,
                                    errbuf);
    if (!handle) {
        log_error("pcap_open_live failed: %s", errbuf);
        return;
    }
    log_info("pcap_open_live success on %s", g_config.device);
    g_pcap_handle = handle;
    signal(SIGINT, on_sigint);

    iface_info_t iface_info;
    if (get_iface_info(g_config.device, &iface_info) != 0) {
        log_error("get_iface_info failed");
        pcap_close(handle);
        return;
    }

    bpf_u_int32 net;//仅占位
    bpf_u_int32 mask = PCAP_NETMASK_UNKNOWN;
    if (pcap_lookupnet(g_config.device, &net, &mask, errbuf) == -1) {
        log_error("pcap_lookupnet failed: %s", errbuf);
        pcap_close(handle);
        return;
    }

    if (g_config.filter_str[0] != '\0') {
        struct bpf_program fp;
        if (pcap_compile(handle, &fp, g_config.filter_str,
                         g_config.optimize, mask) == -1) {
            log_error("pcap_compile failed");
            pcap_close(handle);
            return;
        }
        if (pcap_setfilter(handle, &fp) == -1) {
            log_error("pcap_setfilter failed");
            pcap_freecode(&fp);
            pcap_close(handle);
            return;
        }
        pcap_freecode(&fp);
        log_info("filter: %s", g_config.filter_str);
    }

    memset(&g_handler_ctx, 0, sizeof(g_handler_ctx));
    strncpy(g_handler_ctx.ifname, g_config.device,
            sizeof(g_handler_ctx.ifname) - 1);
    g_handler_ctx.iface = iface_info;
    g_handler_ctx.dlt = pcap_datalink(handle);
    g_handler_ctx.has_filter = (g_config.filter_str[0] != '\0');
    g_handler_ctx.forward_mode = 1;
    g_handler_ctx.multi_addr =
        (iface_info.ipv4[0] != '\0' && iface_info.ipv6[0] != '\0');

    if (iface_info.ipv4[0] != '\0') {
        struct in_addr addr;
        if (inet_pton(AF_INET, iface_info.ipv4, &addr) == 1)
            g_handler_ctx.local_ipv4 = addr.s_addr;
    }

    history_init(&g_history);
    g_handler_user.ctx = &g_handler_ctx;
    g_handler_user.hist = &g_history;

    log_info("start pcap_loop...");
    int ret = pcap_loop(handle, g_config.cnt,
                        packet_handler, (u_char *)&g_handler_user);

    if (ret == -1)
        log_error("pcap_loop failed: %s", pcap_geterr(handle));
    else if (ret == -2)
        log_info("pcap_loop interrupted");

    history_dump(&g_history);
    pcap_close(handle);
    g_pcap_handle = NULL;
}