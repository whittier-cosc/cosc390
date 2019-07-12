#include "config.h"

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    while(1) {
        ;
    }

    return 0;
}

