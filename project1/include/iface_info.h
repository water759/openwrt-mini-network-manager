#ifndef IFACE_INFO_H
#define IFACE_INFO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define IPV4_LEN 16
#define IPV6_LEN 46

typedef struct {
    char ipv4[IPV4_LEN];
    char ipv6[IPV6_LEN];
    uint8_t mac[6];
} iface_info_t;

/*
 * @param ifname 网卡名（如 br-lan / eth0）
 * @param info 输出结果
 * @return 0成功 -1失败
 */
int get_iface_info(const char *ifname, iface_info_t *info);

#ifdef __cplusplus
}
#endif

#endif