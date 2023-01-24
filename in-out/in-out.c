#include "config.h"

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    // B2: digital input
    TRISBbits.TRISB2 = 1; // input
    ANSELBbits.ANSB2 = 0; // digital, not analog

    // B3: digital output
    TRISBbits.TRISB3 = 0; // output

    while(1) {
        // Read input, write value to output
        LATBbits.LATB3 = PORTBbits.RB2;
    }
    return 0;
}
