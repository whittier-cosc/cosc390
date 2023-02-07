#include "config.h"
#include "util.h"
#include "uart.h"
#include <stdio.h>

char msg[10];

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    uart_init();

    // A0, A1, A3, A4, B9, B10, B11 digital outputs
    TRISAbits.TRISA0 = 0;
    TRISAbits.TRISA1 = 0;
    TRISAbits.TRISA3 = 0;
    TRISAbits.TRISA4 = 0;

    uint8_t count = 0;
    uint8_t t;
    while(1) {
        sprintf(msg, "%d\n", count);
        uart_write(msg);
        
        t = count;
        LATAbits.LATA4 = t % 2;
        t = t >> 1;
        LATAbits.LATA3 = t % 2;
        t = t >> 1;
        LATAbits.LATA1 = t % 2;
        t = t >> 1;
        LATAbits.LATA0 = t % 2;

        delay(1000);
        count++;
        if (count == 16) count = 0;
    }
    return 0;
}
