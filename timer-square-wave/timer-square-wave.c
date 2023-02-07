#include "config.h"
#include "util.h"

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    PORTSetPinsDigitalOut(IOPORT_B, BIT_7);

    T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    PR1 = 45454;         // For 880 Hz timer match frequency

    while(1) {
        if (IFS0bits.T1IF) {
            IFS0bits.T1IF = 0;  // clear the interrupt flag
            LATBINV = (1 << 7); // Toggle B7
        }
    }
    return 0;
}
