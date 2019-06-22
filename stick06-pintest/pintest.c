#include "config.h"

void delay(void);

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    TRISA = 0;        // All port A pins are outputs
    TRISB = 0;        // All port B pins are outputs
    LATA = 0;         // Set all port A pins low 
    LATB = 0;         // Set all port B pins low 
    while(1) {
        delay();
        LATAINV = 0xFFFF;    // Toggle all port A pins
        LATBINV = 0xFFFF;    // Toggle all port B pins
        //LATBINV = 1 << 10;
        //delay();
        //LATBINV = 1 << 11;
    }
    return 0;
}

void delay(void) {
    volatile int j;
    for (j = 0; j < 1000000; j++) { // number is 1 million
    }
}
