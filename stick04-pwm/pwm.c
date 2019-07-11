#include "config.h"
#include "uart.h"

#define BUFLEN 40
char msg[BUFLEN];

void uart_config(void) {
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
}

void pwm_config(void) {
    // PWM on OC4. Default is 16-bit timer, Timer2.
    PR2 = 0xffff;   // Timer2 period = 65535 (the maximum),
                    // so PWM period = 65535 + 1.
    OC4RS = 0x8000; // Duty cycle = 50% (0x8000 is half of (0xffff + 1))
    OC4R = 0x8000;  // We have to set this once, before firing up PWM.
                    // Subsequent changes to duty cycle are made by writing
                    // to OC4RS.
    OC4CONbits.OCM = 6; // 3 bits, 110 = PWM mode, fault pin disabled
    T2CONbits.ON = 1;   // Turn on Timer2
    OC4CONbits.ON = 1;  // Turn on PWM.
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    CFGCONbits.IOLOCK = 0;
    U1RXR = 0; // Map RPA2 (pin 9) to U1RX
    RPB3R = 1; // Map RPB3 (pin 7) to U1TX
    RPB2R = 5; // Map RPB2 (pin 6) to OC4 (PWM output)
    CFGCONbits.IOLOCK = 1;

    uart_config();
    pwm_config();

    TRISA = 0xFFFE;        // A0 is LED, so make it an output.
    LATAbits.LATA0 = 0;    // Turn LED off.

    sprintf(msg, "U1BRG = %d\n", U1BRG);
    uart_write(msg);
    unsigned int duty_cycle;
    while(1) {
        uart_read(msg, BUFLEN - 1); // Block until we receive a string
        duty_cycle = atoi(msg); // 0..100 percent
        OC4RS = duty_cycle * 0x10000 / 100; // Update duty cycle
        LATAINV = 0x0001;      // Toggle LED
        uart_write(msg); // Echo the string we received
		uart_write("\n");
    }
    return 0;
}
