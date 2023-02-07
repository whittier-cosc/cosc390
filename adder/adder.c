#include "config.h"
#include "util.h"
#include "uart.h"
#include <stdio.h>


char msg[12];

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    uart_init();

    // B4, B5, B7, B8 digital inputs with internal pull-up
    TRISBbits.TRISB4 = 1;
    // ANSELBbits.ANSB4 = 0;
    CNPUBbits.CNPUB4 = 1;
    TRISBbits.TRISB5 = 1;
    // ANSELBbits.ANSB5 = 0;
    CNPUBbits.CNPUB5 = 1;
    TRISBbits.TRISB7 = 1;
    // ANSELBbits.ANSB7 = 0;
    CNPUBbits.CNPUB7 = 1;
    TRISBbits.TRISB8 = 1;
    // ANSELBbits.ANSB8 = 0;
    CNPUBbits.CNPUB8 = 1;

    // A0, A1, A3, A4, B9, B10, B11 digital outputs
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA4 = 0;
    TRISBbits.TRISB9 = 0;
    TRISBbits.TRISB10 = 0;
    TRISBbits.TRISB11 = 0;


    LATAbits.LATA0 = 1;
    LATAbits.LATA1 = 1;
    LATAbits.LATA3 = 1;
    LATAbits.LATA4 = 1;
    LATBbits.LATB9 = 1;
    LATBbits.LATB10 = 1;
    LATBbits.LATB11 = 1;

    uint8_t disp_x = 9, disp_y = 9;
    while(1) {
        // copy switch inputs to A0 A1 A2 A4
        LATAbits.LATA0 = !PORTBbits.RB5;
        LATAbits.LATA1 = !PORTBbits.RB4;
        LATAbits.LATA3 = !PORTBbits.RB8;
        LATAbits.LATA4 = !PORTBbits.RB7;

        uint8_t x = (~PORTB & 0b110000) >> 4;
        uint8_t y = (~PORTB & 0b110000000) >> 7;
        if (x != disp_x || y != disp_y) {
            disp_x = x;
            disp_y = y;
            
            // compute sum, write to <B9 B10 B11> and UART
            uint8_t sum = disp_x + disp_y;
            sprintf(msg, "%d + %d = %d\n", disp_x, disp_y, sum);
            uart_write(msg);
            LATBbits.LATB11 = sum % 2;
            sum = sum >> 1;
            LATBbits.LATB10 = sum % 2;
            sum = sum >> 1;
            LATBbits.LATB9 = sum % 2;
        }
    }
    return 0;
}
