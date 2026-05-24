#include <stdio.h>
#include "iface_info.h"

int main()
{
    iface_info_t info;

    //const char *iface = "eth0";   // OpenWrt
    const char *iface = "wlo1";  // Ubuntu可用

    if (get_iface_info(iface, &info) != 0) {
        printf("get_iface_info failed\n");
        return -1;
    }

    printf("Interface: %s\n", iface);

    printf("IPv4: %s\n", info.ipv4);
    printf("IPv6: %s\n", info.ipv6);

    printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
        info.mac[0], info.mac[1], info.mac[2],
        info.mac[3], info.mac[4], info.mac[5]);

    return 0;
}