#include "config.h"

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    while(1) {
        ;
    }
    return 0;
}

