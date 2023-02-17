#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include <stdbool.h>
#include <stdio.h>

volatile uint32_t g_duty_end = 150;  // 150 ticks = 7.5% duty cycle (servo range 5%-10%, or 100-200 ticks)
#define TIMEBASE 2000       // 2000 ticks = 20 ms

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) timer1_task() {
    static uint16_t ticks = 0;
    static uint16_t my_duty = 150;
    ticks++;
    if (ticks ==  my_duty)
        LATAbits.LATA4 = 0;
    if (ticks == TIMEBASE) {
        ticks = 0;
        if (g_duty_end != my_duty)
            my_duty = g_duty_end;
        LATAbits.LATA4 = 1;
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

    PORTSetPinsDigitalOut(IOPORT_A, BIT_4);  // PWM output on A5
    PORTSetPinsDigitalIn(IOPORT_A, BIT_3);   // push button


    T1CONbits.TCKPS = 0; // Timer1 prescaler N = 1
    PR1 = 399;           // For 100 kHz interrupt frequency (every 10 us or 0.01 ms)
    IEC0bits.T1IE = 1;   // Enable Timer1 interrupts
    IPC1bits.T1IP = 2;   // Set interrupt priority level
    T1CONbits.ON = 1;    // Turn on Timer1

    // ADC init
    // A1: analog input (default)
    AD1CON3bits.ADCS = 0xff;
    AD1CHSbits.CH0SA = 1;    // use A1 as input
    AD1CON1bits.ON = 1;

    __builtin_enable_interrupts();

    char msg[80];
    tft_printLine(1, 3, "PWM");
    uint32_t adc, adc_prev = 1024;
    while(1) {
        AD1CON1bits.SAMP = 1;
        delay(5);
        AD1CON1bits.SAMP = 0;
        delay(5);

        adc = ADC1BUF0;
        if (adc != adc_prev) {
            adc_prev = adc;
            g_duty_end = 100 + (100 * adc) / 1023;
            sprintf(msg, "%d", adc);
            tft_printLine(1, 3, msg);
        }
    }
    return 0;
}
