#include <string.h>
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

void parse_args(int argc, char **argv)
{
    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strncpy(g_config.device, argv[i + 1], sizeof(g_config.device) - 1);
            g_config.device[sizeof(g_config.device) - 1] = '\0';
            i++;
        }
    }
}