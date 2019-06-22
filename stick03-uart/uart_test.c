#include "config.h"
#include "uart.h"

#define BUFLEN 40
char msg[BUFLEN];

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // Map pins for UART1 RX/TX
    
    CFGCONbits.IOLOCK = 0;
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    CFGCONbits.IOLOCK = 1;

    // Configure UART1
    
    // Set baud to BAUDRATE
    U1MODEbits.BRGH = 0;  // High-speed mode disabled
    // With PBCLK = SYSCLK = 40 M, we have U1BRG = 259, giving 
    // baud rate = 9615.4 (see DS61107F, Table 21-2).
    U1BRG = ((PBCLK / BAUDRATE) / 16) - 1;
    // 8 bit, no parity bit, 1 stop bit (8N1)
    U1MODEbits.PDSEL = 0;
    U1MODEbits.STSEL = 0;

    // Enable TX & RX, taking over U1RX/TX pins
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;
    // Do not enable RTS or CTS
    U1MODEbits.UEN = 0;

    // Enable the UART
    U1MODEbits.ON = 1;

    TRISA = 0xFFFE;     // Pin 0 of Port A is LED. Clear
                        // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0; // Turn LED off.

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    Stick_WriteUART1(msg);
    while(1) {
        Stick_ReadUART1(msg, BUFLEN - 1); // Block until we receive a string
        LATAINV = 0x0001;                 // Toggle LED
		// Echo the string we received
        Stick_WriteUART1(msg);
		Stick_WriteUART1("\n");
    }
    return 0;
}
