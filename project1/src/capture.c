#include <stdio.h>
#include <pcap.h>
#include "config.h"
#include "logger.h"
#include "capture.h"
#include "iface_info.h"
#include "packet_handler.h"

void capture_init() {

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t*handle=pcap_open_live(g_config.device, g_config.snaplen, g_config.promisc, g_config.to_ms,errbuf );

    if(handle==NULL) {
        log_error("pcap_open_live failed: %s", errbuf);
        return;
    }

    log_info("pcap_open_live success");
    
    iface_info_t iface_info;
    if (get_iface_info(g_config.device, &iface_info) != 0) {
        log_error("get_iface_info failed");
        return;
    }

    log_info("get_iface_info success");

    bpf_u_int32 mask= PCAP_NETMASK_UNKNOWN;
    bpf_u_int32 net; // 必须占位，但不用
    if(pcap_lookupnet(g_config.device, &net, &mask, errbuf) == -1) {
        log_error("pcap_lookupnet failed: %s", errbuf);
        return;
    }

    struct bpf_program fp;
    if(pcap_compile(handle, &fp, g_config.filter_str, g_config.optimize, mask) == -1) {
        log_error("pcap_compile failed");
        return;
    }
    if(pcap_setfilter(handle, &fp) == -1) {
        log_error("pcap_setfilter failed");
        return;
    }
    pcap_freecode(&fp);
    log_info("pcap_setfilter success");
    


    int dlt = pcap_datalink(handle);

    
    packet_handler();












}