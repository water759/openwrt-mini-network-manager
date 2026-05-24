#include <stdio.h>

#include "config.h"
#include "capture.h"

int main(int argc, char **argv)
{
    load_config();
    parse_args(argc, argv);
    capture_init();
    return 0;
}