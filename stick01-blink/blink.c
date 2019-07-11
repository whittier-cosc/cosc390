#include "config.h"

void delay(int ms);

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.

    while(1) {
        delay(8);
        LATAINV = 0x0001;   // toggle LED
    }
    return 0;
}

// Delay for a given number of milliseconds. This crude implementation
// is often good enough, but accuracy will suffer if significant time
// is spent in interrupt service routines. See delay_ms() in peripherals/tft_master.c
// for a better implementation that uses the core timer.
void delay(int ms) {
    volatile int j;
    for (j = 0; j < (SYSCLK / 8920) * ms; j++) { // magic constant 8920 obtained empirically
    }
}
