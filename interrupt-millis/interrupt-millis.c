#include "config.h"
#include "util.h"
#include "uart.h"
#include <stdbool.h>
#include <stdio.h>

// Global variables that are modified by interrupt service routines
// must be declared "volatile" 
volatile uint32_t g_millis;  // milliseconds
volatile bool g_do_second;

// Interrupt service routine (ISR) for Timer1 interrupt
void __ISR(_TIMER_1_VECTOR, IPL2SOFT) timer1_task() {
    static uint16_t my_millis = 0;
    g_millis++;
    my_millis++;
    if (my_millis == 1000) {
        my_millis = 0;
        g_do_second = true;
    }

    // Clear the Timer1 interrupt flag (non-PLIB version)
    // IFS0bits.T1IF = 0;

    // Clear the Timer1 interrupt flag (PLIB version)
    INTClearFlag(INT_T1);
}

int main(void) {
    // Configure the device for maximum performance (PLIB)
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    // Initialize wcpic32lib
    wclib_init(SYSCLK, PBCLK);

    // Initialize UART1
    uart_init();

    /*********************************************************/

    // Configure Timer1 (non-PLIB version)
    // T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    // PR1 = 39999;         // For 1 kHz interrupt frequency
    // T1CONbits.ON = 1;    // Turn on Timer1

    // Configure Timer1 (PLIB version)
    OpenTimer1(T1_ON | T1_PS_1_1, 39999);

    /*********************************************************/

    // Configure Timer1 interrupts (non-PLIB version)
    // IEC0bits.T1IE = 1;   // Enable Timer1 interrupts
    // IPC1bits.T1IP = 2;   // Set interrupt priority level

    // Configure Timer1 interrupts (PLIB version)
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);

    /*********************************************************/

    // Enable multi-vector interrupt mode and turn on interrupts
    // system-wide (non-PLIB version)
    // INTCONbits.MVEC = 1;
    // __builtin_enable_interrupts();

    // Enable multi-vector interrupt mode and turn on interrupts
    // system-wide (PLIB version)
    INTEnableSystemMultiVectoredInt();

    /*********************************************************/

    printf("Starting.\n");
    uint32_t sec = 0;
    while(1) {
        if (g_do_second) {
            sec++;
            printf("%d ", sec);
            g_do_second = false;
        }
    }
    return 0;
}
