#include "history.h"
#include <stdio.h>
#include <string.h>

int main() {
    history_t hist;
    history_init(&hist);

    parsed_packet_t p1 = {
        .src_ip = 1,
        .dst_ip = 2,
        .src_port = 1000,
        .dst_port = 80,
        .protocol = 6,
        .direction = DIR_INBOUND
    };

    parsed_packet_t p2 = p1;
    p2.direction = DIR_OUTBOUND;

    // 模拟两个包
    history_update_flow(&hist, &p1, 100);
    history_update_flow(&hist, &p2, 200);

    history_dump(&hist);

    return 0;
}