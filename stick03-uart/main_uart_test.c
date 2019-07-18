#include "config.h"
#include "uart.h"
#include <stdio.h>

#define BUFSZ 80
char msg[BUFSZ];

int main(void) {// Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    uart_init();

    TRISA = 0xFFFE;     // Pin 0 of Port A is LED. Clear
    // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0; // Turn LED off.

//    sprintf(msg, "U1BRG = %d\n", U1BRG);
//    uart_write(msg);

    printf("U1BRG = %d", U1BRG);  // more convenient?

//    sprintf(msg, "IOL1WAY = %d\n", DEVCFG3bits.IOL1WAY);
//    uart_write(msg);

    printf("IOL1WAY = %d\n", DEVCFG3bits.IOL1WAY);

//    sprintf(msg, "IOLOCK = %d\n", CFGCONbits.IOLOCK);
//    uart_write(msg);

    printf("IOLOCK = %d\n", CFGCONbits.IOLOCK);

    while(1) {
        uart_read(msg, BUFSZ - 1); // Block until we receive a string
        LATAINV = 0x0001;           // Toggle LED
        // Echo the string we received
//        uart_write(msg);
//        uart_write("\n");
        printf(msg);
        printf("\n");
    }
    return 0;
}

