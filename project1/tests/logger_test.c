#include <stdio.h>
#include "logger.h"

int main() {
    log_info("This is an info message");
    log_error("This is an error message");

    return 0;
}