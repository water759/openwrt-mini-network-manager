#include "config.h"

Config g_config;
void load_config() {
    strcpy(g_config.device, "eth0");
    g_config.snaplen = 65535;
    g_config.promisc = 1;
    g_config.to_ms = 1000;

    g_config.cnt = -1;

    strcpy(g_config.filter_str, "tcp");
    g_config.optimize = 1;
    
}