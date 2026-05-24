#ifndef STATS_EXPORT_H
#define STATS_EXPORT_H

#include "history.h"

#define STATS_JSON_PATH "/tmp/netmon/stats.json"

int history_write_json(const history_t *hist, const char *path);

#endif