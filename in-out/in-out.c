#include "config.h"
#include "util.h"
#include "uart.h"
#include <stdio.h>  // for sprintf

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    uart_init();

    uart_write("Hi from PIC!\n");
    char msg[20];
    sprintf(msg, "%d + %d = %d\n", 2, 3, 2 + 3);
    uart_write(msg);

    // print current value of TRISB in hex
    sprintf(msg, "TRISB = 0x%08X\n", TRISB);
    uart_write(msg);


    // B2: digital input
    TRISBbits.TRISB2 = 1;   // input
    ANSELBbits.ANSB2 = 0;   // digital, not analog
    CNPUBbits.CNPUB2 = 1;   // enable pull-up

    // B4: digital output
    TRISBbits.TRISB4 = 0;   // output

    while(1) {
        // Read state of B2, write !state to B4
        LATBbits.LATB4 = !PORTBbits.RB2;
    }
    return 0;
}
