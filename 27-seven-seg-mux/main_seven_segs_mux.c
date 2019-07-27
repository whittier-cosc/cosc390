/**
 *  @file   main_seven_segs.c
 *
 *  @brief  multiplex a 4-digit, 7-segment LED display (5461A),
 *          and use a 4051B 8-channel multiplexer to drive segments
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

// PORT B (segments)
#define SEGA BIT_7
#define SEGB BIT_7 | BIT_8
#define SEGC BIT_7 | BIT_8 | BIT_9
#define SEGD BIT_7 | BIT_9
#define SEGE BIT_9
#define SEGF 0
#define SEGG BIT_8 | BIT_9
#define SEGH BIT_8

// PORT A (digit select)
#define DIG1 BIT_1  // pin 12
#define DIG2 BIT_2  // pin 9
#define DIG3 BIT_3  // pin 8
#define DIG4 BIT_4  // pin 6

// precise delay for ADC acquisition
#define NOP asm("nop");
// 200 ns
#define NS_200 {NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;}

#define NSAMP 16

char msg[80];

uint32_t segs[8] = {SEGA, SEGB, SEGC, SEGD, SEGE, SEGF, SEGG, SEGH};

void adc_init() {
    // 1. Configure the analog port pins
    // We'll use AN0 (pin 2), which is configured as an analog
    // input by default.

    // 2. Select the analog inputs to the ADC multiplexers.
    // Nothing to do here, since AN0 is the default input.

    // 3. Select the format of the ADC result using FORM<2:0>
    // We use the default 16-bit integer result format.

    // 4. Select the sample clock source using SSRC<2:0> (AD1CON1<7:5>)
    //AD1CON1bits.SSRC = 2; // Use Timer3 period match as conversion trigger
    AD1CON1bits.SSRC = 0; // Clearing SAMP bit ends sampling, starts conversion

    // 5. Select the voltage reference source using VCFG<2:0>
    // We use the default: low end = VSS, high end = VDD (0 to 3.3 V)

    // 6. Select the Scan mode using CSCNA (AD1CON2<10>)
    // We don't want to scan through multiple inputs, so leave as default (0).

    // 7. Set the number of conversions per interrupt SMP<3:0> (AD1CON2<5:2>),
    // if interrupts are to be used.
    // We're not using interrupts, so do nothing.

    // 8. Set Buffer Fill mode using BUFM (AD1CON2<1>)
    // We'll try the default (one 16-word results buffer)

    // 9. Select the MUX to be connected to the ADC in ALTS AD1CON2<0>
    // Use default (do not alternate between the two mulitplexers).

    // 10. Select the ADC clock source using ADRC (AD1CON3<15>)
    // Use default (peripheral bus clock).

    // 11. Select the sample time using SAMC<4:0> (AD1CON3<12:8>),
    // if auto-convert is to be used
    // We're not using auto-convert.

    // 12. Select the ADC clock prescaler using ADCS<7:0> (AD1CON3<7:0>)
    AD1CON3bits.ADCS = 0x01; // Tad = 4 * Tpb

    // 13. Turn the ADC module on using AD1CON1<15>
    AD1CON1bits.ON = 1;
}
int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    int i;
    int ms;   // delay between digits (ms)
//    uint32_t reading;
//    uint32_t curr_ms = 9999;
//    uint32_t total;

//    ANSELA = 1; // A0 analog input for delay controller
//    ANSELB = 0;
//    mPORTASetPinsDigitalOut(DIG1 | DIG2 | DIG3 | DIG4);
//    mPORTAOpenDrainOpen(DIG1 | DIG2 | DIG3 | DIG4);
//    mPORTBSetPinsDigitalOut(SEGA | SEGB | SEGC | SEGD | SEGE | SEGF | SEGG | SEGH);

//    mPORTBSetBits(SEGA | SEGB | SEGC | SEGH);   // segment a on
//    mPORTBClearBits(SEGD | SEGE | SEGF | SEGG); // all other segments off
//    mPORTASetBits(DIG1 | DIG2 | DIG3 | DIG4); // all digits unselected


//    adc_init();
//    tft_init();
//    tft_fillScreen(ILI9340_BLACK);
//    tft_setRotation(0);
//    tft_printLine(1, 5, "hello");

    ms = 500;
    i = 0;
    mPORTASetPinsDigitalOut(DIG1 | DIG2 | DIG3 | DIG4);
    mPORTAOpenDrainOpen(DIG1 | DIG2 | DIG3 | DIG4);
    mPORTBSetPinsDigitalOut(BIT_7 | BIT_8 | BIT_9);
    mPORTAClearBits(DIG1); // digit 1 on (cathode connected to ground)
    mPORTASetBits(DIG2 | DIG3 | DIG4);   // other digits off (cathode open)
    while(1) {
//        // average NSAMP samples to smooth things out
//        reading = 0;
//        for (i = 0; i < NSAMP; i++) {
//            AD1CON1bits.SAMP = 1; // start sampling
//            NS_200; // wait the minimum 200 ns acquisition time
//            AD1CON1bits.SAMP = 0; // end sampling, start conversion
//            while (!AD1CON1bits.DONE) {
//            }
//            reading += ADC1BUF0;
//        }
//        reading = reading / NSAMP;
//        ms = reading / 6 - 15;
//        if (ms < 1) {
//            ms = 1;
//        }
//        if (ms != curr_ms) {
//            curr_ms = ms;
//            sprintf(msg, "%4d", reading);
//            tft_printLine(1, 5, msg);
//            sprintf(msg, "%4d ms", ms);
//            tft_printLine(5, 5, msg);
//        }

//        mPORTAClearBits(DIG1); // digit 1 on (cathode connected to ground)
//        mPORTASetBits(DIG2 | DIG3 | DIG4);   // other digits off (cathode open)
        LATB = segs[i];   // segment i on
        i++;
        if (i == 8) {
            i = 0;
        }
        delay(ms);

//        mPORTAClearBits(DIG2);
//        mPORTASetBits(DIG1 | DIG3 | DIG4);
//        delay(ms);
//        mPORTAClearBits(DIG3);
//        mPORTASetBits(DIG1 | DIG2 | DIG4);
//        delay(ms);
//        mPORTAClearBits(DIG4);
//        mPORTASetBits(DIG1 | DIG2 | DIG3);
//        delay(ms);
    }

    return 0;
}

