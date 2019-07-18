/*
 * Try internal interrupts 0 and 1. Use single-vector mode, so the 
 * ISR needs to examine INTSTAT to determine which of the two interrupts
 * occurred.
 */

#include "config.h"
#include "util.h"

void __ISR_SINGLE__ ExtIntISR(void) {

    if (INTSTATbits.VEC == _EXTERNAL_0_VECTOR) {
        /* Just toggle the LED if we're handling external interrupt 0 */
        LATAINV = 1;
        IFS0bits.INT0IF = 0;
    }
    else if (INTSTATbits.VEC == _EXTERNAL_1_VECTOR) {
        /* Toggle the LED twice if we're handling external interrupt 1 */
        LATAINV = 1;
        delay(250);
        LATAINV = 1;
        IFS0bits.INT1IF = 0;
    }
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // External interrupt 0 is on pin 16 (fixed)

    INT1R = 0; // Map external interrupt 1 to pin 10 (RPA3)

    IEC0bits.INT0IE = 1;   // enable INT0
    IPC0bits.INT0IP = 1;   // priority 1
    IFS0bits.INT0IF = 0;   // clear interrupt flag
    INTCONbits.INT0EP = 1; // trigger on rising edge

    IEC0bits.INT1IE = 1;   // enable INT1
    IPC1bits.INT1IP = 1;   // priority 1
    IFS0bits.INT1IF = 0;   // clear interrupt flag
    INTCONbits.INT1EP = 1; // trigger on rising edge

    INTCONbits.MVEC = 0; // single-vector mode
    __builtin_enable_interrupts();

    TRISA = 0xfffe;
    LATAbits.LATA0 = 1;
    int i;
    for (i = 0; i < 6; i++) {
        delay(250);
        LATAINV = 1;
    }

    while(1) {
        ;
    }
    return 0;
}
