/**
 *  @file   main_seven_segs.c
 *
 *  @brief  multiplex a 4-digit, 7-segment LED display (5461A),
 *          and use a 4051B 8-channel multiplexer to drive segments
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

// precise delay for ADC acquisition
#define NOP asm("nop");
// 200 ns
#define NS_200 {NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;}

#define SAMPLING_PERIOD 15625 // 200 ms (if timer prescaler is 256)
#define NSAMP 8

char msg[80];

void adc_init() {
    // configure and enable the ADC
    CloseADC10();// ensure the ADC is off before setting the configuration
    // define setup parameters for OpenADC10
    #define PARAM1 ADC_MODULE_ON | ADC_FORMAT_INTG | ADC_CLK_MANUAL | ADC_AUTO_SAMPLING_OFF
    #define PARAM2 ADC_VREF_AVDD_AVSS | ADC_OFFSET_CAL_DISABLE | ADC_SCAN_OFF | ADC_ALT_INPUT_ON
    #define PARAM3 ADC_CONV_CLK_PB | ADC_CONV_CLK_Tcy
    #define PARAM4 0  // configscan
    #define PARAM5 ENABLE_AN5_ANA | ENABLE_AN0_ANA  // configport
    // configure to sample AN0 & AN5
    SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN5 |
                 ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN0);
    // configure ADC and enable it
    OpenADC10(PARAM1, PARAM2, PARAM3, PARAM4, PARAM5);
    // Now enable the ADC logic
    EnableADC10();
}

void __ISR(_TIMER_2_VECTOR, IPL2SOFT) ADC_run(void) {
    uint32_t reading_x, reading_y;
    static uint32_t curr_reading_x = 1024, curr_reading_y = 1024;
    int i;
    //average NSAMP samples on each channel to smooth things out
    reading_x = reading_y = 0;
    for (i = 0; i < NSAMP; i++) {
        SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN0 |
                     ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN5);
        AD1CON1bits.SAMP = 1; // start sampling
        NS_200; // wait the minimum 200 ns acquisition time
        AD1CON1bits.SAMP = 0; // end sampling, start conversion
        while (!AD1CON1bits.DONE) {
        }
        reading_x += ADC1BUF0;

        SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN5 |
                     ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN0);
        AD1CON1bits.SAMP = 1; // start sampling
        NS_200; // wait the minimum 200 ns acquisition time
        AD1CON1bits.SAMP = 0; // end sampling, start conversion
        while (!AD1CON1bits.DONE) {
        }
        reading_y += ADC1BUF0;
    }
    reading_x = reading_x / NSAMP;
    reading_y = reading_y / NSAMP;

    if (reading_x != curr_reading_x) {
        curr_reading_x = reading_x;
        sprintf(msg, "x: %4d", reading_x);
        tft_printLine(1, 5, msg);
    }
    if (reading_y != curr_reading_y) {
        curr_reading_y = reading_y;
        sprintf(msg, "y: %4d", reading_y);
        tft_printLine(5, 5, msg);
    }

    mT2ClearIntFlag();
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    INTEnableSystemMultiVectoredInt();
    __builtin_disable_interrupts();

    adc_init();

    tft_init();
    tft_fillScreen(ILI9340_BLUE);
    tft_setRotation(0);
    //tft_printLine(2, 5, "ready");

    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_256, SAMPLING_PERIOD);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    mT2ClearIntFlag();

    __builtin_enable_interrupts();

    while(1) {
    }
    return 0;
}

