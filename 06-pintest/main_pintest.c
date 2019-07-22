#include "config.h"
#include "util.h"

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);    TRISA = 0;        // All port A pins are outputs
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
