#include "iface_info.h"

#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#include <netinet/in.h>

int get_iface_info(const char *ifname, iface_info_t *info)
{
    if (!ifname || !info) return -1;

    memset(info, 0, sizeof(*info));

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return -1;

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
    // mac   
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0) {
        memcpy(info->mac, ifr.ifr_hwaddr.sa_data, 6);
    } else {
        close(fd);
        return -1;
    }

    close(fd);

    
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) != 0) {
        return -1;
    }

    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {

        if (!ifa->ifa_addr) continue;
        if (strcmp(ifa->ifa_name, ifname) != 0) continue;

        int family = ifa->ifa_addr->sa_family;

        // IPv4 
        if (family == AF_INET && info->ipv4[0] == '\0') {
            struct sockaddr_in *sa =
                (struct sockaddr_in *)ifa->ifa_addr;

            inet_ntop(AF_INET, &sa->sin_addr,
                      info->ipv4, IPV4_LEN);
        }

        // IPv6 
        if (family == AF_INET6 && info->ipv6[0] == '\0') {
            struct sockaddr_in6 *sa6 =
                (struct sockaddr_in6 *)ifa->ifa_addr;

            inet_ntop(AF_INET6, &sa6->sin6_addr,
                      info->ipv6, IPV6_LEN);
        }
    }

    freeifaddrs(ifaddr);

    return 0;
}