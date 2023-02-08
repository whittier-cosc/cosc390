#include "config.h"
#include "util.h"
#include "uart.h"
#include <stdbool.h>
#include <stdio.h>

volatile uint32_t g_millis;  // milliseconds
volatile bool do_seconds;

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) timer1_task() {
    static uint16_t my_millis = 0;
    g_millis++;
    my_millis++;
    if (my_millis == 1000) {
        my_millis = 0;
        do_seconds = true;
    }
    IFS0bits.T1IF = 0; // clear the interrupt flag
}

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    INTCONbits.MVEC = 1; // Enable multi-vector interrupt mode

    uart_init();

    PORTSetPinsDigitalOut(IOPORT_A, BIT_0);


    T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    PR1 = 39999;         // For 1 kHz interrupt frequency
    IEC0bits.T1IE = 1;   // Enable Timer1 interrupts
    IPC1bits.T1IP = 2;   // Set interrupt priority level
    T1CONbits.ON = 1;    // Turn on Timer1


    __builtin_enable_interrupts();

    printf("Starting. \n");
    uint32_t sec = 0;
    while(1) {
        if (do_seconds) {
            sec++;
            printf("%d ", sec);
            do_seconds = false;
        }
    }
    return 0;
}
