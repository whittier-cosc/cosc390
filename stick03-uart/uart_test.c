#include "config.h"
#include "uart.h"

char msg[80];

int main(void) {// Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    uart_init();

    TRISA = 0xFFFE;     // Pin 0 of Port A is LED. Clear
    // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0; // Turn LED off.

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    uart_write(msg);
    while(1) {
        uart_read(msg, BUFLEN - 1); // Block until we receive a string
        LATAINV = 0x0001;           // Toggle LED
        // Echo the string we received
        uart_write(msg);
        uart_write("\n");
    }
    return 0;
}
