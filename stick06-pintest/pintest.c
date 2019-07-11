#include "config.h"

void delay(int ms);

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    TRISA = 0;        // All port A pins are outputs
    TRISB = 0;        // All port B pins are outputs
    LATA = 0;         // Set all port A pins low 
    LATB = 0;         // Set all port B pins low 
    while(1) {
        delay(250);
        LATAINV = 0xFFFF;    // Toggle all port A pins
        LATBINV = 0xFFFF;    // Toggle all port B pins
        //LATBINV = 1 << 10;
        //delay(250);
        //LATBINV = 1 << 11;
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
