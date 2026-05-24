#include "stats_export.h"
#include <stdio.h>
#include <arpa/inet.h>

static void ip_str(uint32_t ip, char *buf, size_t len)
{
    struct in_addr a = { .s_addr = ip };
    inet_ntop(AF_INET, &a, buf, len);
}

int history_write_json(const history_t *hist, const char *path)
{
    if (!hist || !path)
        return -1;

    FILE *fp = fopen(path, "w");
    if (!fp)
        return -1;

    fprintf(fp, "{\n");
    fprintf(fp, "  \"totals\": {\n");
    fprintf(fp, "    \"rx_bytes\": %llu,\n", (unsigned long long)hist->total_rx_bytes);
    fprintf(fp, "    \"tx_bytes\": %llu,\n", (unsigned long long)hist->total_tx_bytes);
    fprintf(fp, "    \"rx_pkts\": %llu,\n",  (unsigned long long)hist->total_rx_pkts);
    fprintf(fp, "    \"tx_pkts\": %llu\n",   (unsigned long long)hist->total_tx_pkts);
    fprintf(fp, "  },\n");
    fprintf(fp, "  \"flows\": [\n");

    int first = 1;
    char sip[16], dip[16];
    for (int i = 0; i < MAX_FLOWS; i++) {
        const flow_entry_t *e = &hist->flows[i];
        if (!e->used)
            continue;
        ip_str(e->src_ip, sip, sizeof(sip));
        ip_str(e->dst_ip, dip, sizeof(dip));
        if (!first)
            fprintf(fp, ",\n");
        first = 0;
        fprintf(fp, "    {\"src_ip\":\"%s\",\"dst_ip\":\"%s\","
                    "\"src_port\":%u,\"dst_port\":%u,\"protocol\":%u,"
                    "\"rx_bytes\":%llu,\"tx_bytes\":%llu}",
                sip, dip, e->src_port, e->dst_port, e->protocol,
                (unsigned long long)e->rx_bytes,
                (unsigned long long)e->tx_bytes);
    }

    fprintf(fp, "\n  ]\n}\n");
    fclose(fp);
    return 0;
}