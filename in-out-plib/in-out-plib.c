#include "config.h"
#include "uart.h"
#include "util.h"  // for delay()
#include <stdio.h> // for printf()

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    // Initialize UART1 peripheral
    uart_init();

    // Configure B2 and B4 as digital inputs with internal pullups enabled.
    // The PORTSetDigitalIn PLIB function sets TRISx and ANSELx as necessary.
    PORTSetPinsDigitalIn(IOPORT_B, BIT_2 | BIT_4);

    ConfigCNBPullups(CNB2_PULLUP_ENABLE | CNB4_PULLUP_ENABLE);

    // Print a message via UART
    // Our wcpic32lib configures printf() to print to UART1
    printf("Hi from PIC!\n");
    printf("%d + %d = %d\n", 2, 3, 2 + 3);

    // Print current value of TRISB via UART
    printf("TRISB = Ox%08X\n", TRISB);

    // Configure B5 and B7 as digital outputs
    PORTSetPinsDigitalOut(IOPORT_B, BIT_5 | BIT_7);

    // Print new value of TRISB via UART
    printf("TRISB = Ox%08X\n", TRISB);

    while(1) {
        // Read input, write !value to output and print value to UART
        LATBbits.LATB5 = !PORTBbits.RB2;
        LATBbits.LATB7 = !PORTBbits.RB4;
        
        printf("%d %d\n", PORTBbits.RB2, PORTBbits.RB4);

        // Wait 1 second
        delay(1000);
    }
    return 0;
}
