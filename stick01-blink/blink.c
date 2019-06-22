#include "config.h"

void delay(void);

int main(void) {
    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.

    while(1) {
        delay();
        LATAINV = 0x0001;   // toggle LED
    }
    return 0;
}

void delay(void) {
    volatile int j;
    for (j = 0; j < 1000000; j++) {
    }
}
