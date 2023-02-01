#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include <stdio.h>  // for sprintf

#define VMAX 3.30
#define LEVEL1 (1 * VMAX / 6.0)
#define LEVEL2 (2 * VMAX / 6.0)
#define LEVEL3 (3 * VMAX / 6.0)
#define LEVEL4 (4 * VMAX / 6.0)
#define LEVEL5 (5 * VMAX / 6.0)

int main(void) {
    // Configure the device for maximum performance.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);

    tft_init(); // uses B2, B8, B9, B11, B14
    tft_setRotation(3);
    tft_fillScreen(ILI9340_BLACK);

    // A1: analog input (default)

    // B3, B4, B5, B7, B10: digital outputs (LEDs)
    TRISBbits.TRISB3 = 0;   // output
    TRISBbits.TRISB4 = 0;   // output
    TRISBbits.TRISB5 = 0;   // output
    TRISBbits.TRISB7 = 0;   // output
    TRISBbits.TRISB10 = 0;   // output

    // ADC init
    AD1CON3bits.ADCS = 0xff;
    AD1CHSbits.CH0SA = 1;    // use A1 as input
    AD1CON1bits.ON = 1;

    float volts;
    char msg[40];

    while(1) {
        AD1CON1bits.SAMP = 1;
        delay(100);
        AD1CON1bits.SAMP = 0;
        delay(100);

        volts = ADC1BUF0 * VMAX / 1023.0;

        LATBbits.LATB3 = (volts > LEVEL1);
        LATBbits.LATB4 = (volts > LEVEL2);
        LATBbits.LATB5 = (volts > LEVEL3);
        LATBbits.LATB7 = (volts > LEVEL4);
        LATBbits.LATB10 = (volts > LEVEL5);

        sprintf(msg, "%4d", ADC1BUF0);
        tft_printLine(0, 4, msg);
        sprintf(msg, "%.2f V", volts);
        tft_printLine(5, 4, msg);
        delay(800);
    }
    return 0;
}
