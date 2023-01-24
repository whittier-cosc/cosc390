#include "config.h"
#include "uart.h"
#include "util.h"

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    uart_init();

    // B2: digital input
    TRISBbits.TRISB2 = 1; // input
    ANSELBbits.ANSB2 = 0; // digital, not analog
    CNPUBbits.CNPUB2 = 1; // enable internal pull-up

    // B4: digital output
    TRISBbits.TRISB4 = 0; // output

    uart_write("Hi from PIC\r\n");

    while(1) {
        // Read input, write value to output
        LATBbits.LATB4 = !PORTBbits.RB2;
        if (PORTBbits.RB2) 
            uart_write("1");
        else
            uart_write("0");
        delay(1000);
        
    }
    return 0;
}
