#include "config.h"
#include "util.h"

#include "uart.h"
#include "amp.h"
#include <stdio.h>

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.

    while(1) {
        delay(500);
        LATAINV = 0x0001;   // toggle LED
    }
    return 0;
}
