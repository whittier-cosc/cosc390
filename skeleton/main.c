#include "config.h"

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    while(1) {
        ;
    }

    return 0;
}

