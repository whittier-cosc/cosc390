#include "config.h"

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    // Configure comparator voltage reference
    CVRCONbits.CVROE = 1; // enable output on CVrefout (pin 25)
                          // Pin 25 is used for SPI clock output, however,
                          // so can't use my usual TFT LCD setup.
    CVRCONbits.CVR = 8;   // Should give output = 0.5 * VDD
    CVRCONbits.ON = 1;
    // Measurement gives output voltage = 1.62 V and VDD = 3.25 V, so
    // seems OK.

    // Configure comparator
    PPSOutput(4, RPB9, C1OUT); // Map C1OUT to RPB9 (pin 18)
    // We'll compare C1INB (pin 6) to internal voltage reference
    // configured above.
    CM1CONbits.COE = 1;   // Enable output on C1OUT
    CM1CONbits.EVPOL = 0; // Disable generation of interrupts by comparator
    CM1CONbits.CREF = 1;  // Compare to Vref configured above (non-inverting)
    CM1CONbits.CCH = 0;   // Use C1INB input (as inverting input)
    CM1CONbits.CPOL = 1;  // Invert the output so high means Vin > Vref
    CM1CONbits.ON = 1;    // Turn on the comparator

    while(1) {
    }
    return 0;
}
