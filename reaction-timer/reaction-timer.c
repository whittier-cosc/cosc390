#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
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

    tft_init();
    tft_setRotation(1);
    tft_fillScreen(ILI9340_BLACK);

    PORTSetPinsDigitalOut(IOPORT_A, BIT_0);  // on-board LED
    PORTSetPinsDigitalIn(IOPORT_A, BIT_3);   // push button


    T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    PR1 = 39999;         // For 1 kHz interrupt frequency
    IEC0bits.T1IE = 1;   // Enable Timer1 interrupts
    IPC1bits.T1IP = 2;   // Set interrupt priority level
    T1CONbits.ON = 1;    // Turn on Timer1


    __builtin_enable_interrupts();

    printf("Starting. \n");
    uint32_t start, stop;
    LATAbits.LATA0 = 0;
    char msg[80];
    uint32_t high_score = 0;
    while(1) {
        tft_printLine(1, 3, "READY");
        while (PORTAbits.RA3) ;
        tft_printLine(1, 3, "ARMED");
        uint16_t random_ms_delay = 7 * (g_millis % 1000) + 1000;  // 1 to 8 seconds
        delay(random_ms_delay);
        LATAbits.LATA0 = 1;
        start = g_millis;
        while (PORTAbits.RA3) ;
        stop = g_millis;
        LATAbits.LATA0 = 0;
        sprintf(msg, "       %d ms", stop - start);
        tft_printLine(8, 3, msg);
        if (high_score == 0 || stop - start < high_score) {
            high_score = stop - start;
            sprintf(msg, "High Score: %d ms", stop - start);
            tft_printLine(15, 3, msg);
        }
        delay(1000);
        
    }
    return 0;
}
