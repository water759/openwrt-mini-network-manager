#ifndef HISTORY_H
#define HISTORY_H

#include <stdint.h>
#include "packet_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FLOWS 4096

typedef struct {
    uint32_t src_ip;
    uint32_t dst_ip;
    uint16_t src_port;
    uint16_t dst_port;
    uint8_t  protocol;
    uint64_t rx_bytes;
    uint64_t tx_bytes;
    uint64_t rx_pkts;
    uint64_t tx_pkts;
    int used;
} flow_entry_t;

struct history {
    flow_entry_t flows[MAX_FLOWS];
    uint64_t total_rx_bytes;
    uint64_t total_tx_bytes;
    uint64_t total_rx_pkts;
    uint64_t total_tx_pkts;
};

void history_init(history_t *hist);
void history_update_flow(history_t *hist,
                         const parsed_packet_t *pkt,
                         uint32_t bytes);
void history_dump(const history_t *hist);

#ifdef __cplusplus
}
#endif

#endif