#include "config.h"
#include "util.h"

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    /*
        This program uses Timer1 to produce a 440 Hz square wave
        on pin B7. 

        We assume PBCLK = 40 MHz, so we must toggle B7 every 45455 
        PBCLK cycles (40 MHz / 880 Hz = 45455). Using a prescaler N = 1,
        this means we must set PR1 = 45455 - 1 = 45454.
    
    */
    PORTSetPinsDigitalOut(IOPORT_B, BIT_7);

    T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    PR1 = 45454;
    T1CONbits.ON = 1;    // Turn on Timer1

    while(1) {
        if (IFS0bits.T1IF) {
            IFS0bits.T1IF = 0;  // clear the interrupt flag
            LATBINV = (1 << 7); // Toggle B7
        }
    }
    return 0;
}
