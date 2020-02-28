/**
 *  @file   main_round-robin.c
 *
 *  @brief  Round-robin tick-based scheduling experiment.
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "uart.h"

#define SAMPLING_PERIOD 120 // 3000 ns (if timer 3 prescaler is 1)
#define NSAMP 8

#define DRAW_COLOR  ILI9340_BLUE
#define BALL_COLOR  ILI9340_GREEN
#define BALL_RADIUS 8

#define W  240
#define H  320
#define XC (W / 2)
#define YC (H / 2)
#define X0 480
#define Y0 460

// Range of analog readings from joystick
// x-axis on AN0
// y-axis on AN5
#define XR_MIN      170
#define XR_MAX      580
#define XR_RANGE    (XR_MAX - XR_MIN)
#define YR_MIN      150
#define YR_MAX      800
#define YR_RANGE    (YR_MAX - YR_MIN)

#define XSCALE ((XR_MAX - XR_MIN) / W)
#define YSCALE ((YR_MAX - YR_MIN) / H)

//char msg[80];

// Task periods in ms
const uint16_t adc_task_period = 1;
const uint16_t blink_task_period = 2;
uint16_t adc_ticks = 0;
uint16_t blink_ticks = 0;

void adc_init() {
    // configure ADC and enable it
    SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN0 |
                 ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN1);
    OpenADC10(ADC_MODULE_ON | ADC_CLK_TMR | ADC_AUTO_SAMPLING_ON, // AD1CON1
              ADC_ALT_INPUT_ON | ADC_SAMPLES_PER_INT_2,           // AD1CON2
              ADC_CONV_CLK_PB | ADC_CONV_CLK_2Tcy,                // AD1CON3
              0,                                                  // configscan
              ENABLE_AN0_ANA | ENABLE_AN1_ANA);                   // configport
    EnableADC10();
}

#define SAMPLE 0
#define ERASE 1
#define DRAW  2
#define WAIT 3
#define WAIT_PERIOD 10
void adc_task() {
    uint32_t reading_x, reading_y;
    static uint32_t curr_reading_x = 1024, curr_reading_y = 1024;
    static int16_t x = XC, y = YC;
    static int8_t state = SAMPLE;
    static int wait_count;
    int i;

    switch (state) {

    // sampling phase takes only 100 us
    case SAMPLE:
        // Average NSAMP samples on each channel to smooth things out.
        // Once auto-sampling is enabled, conversions are triggered by
        // Timer 3 match.
        // Two readings per "interrupt"; first is from AN0, second from AN5.
        AD1CON1SET = _AD1CON1_ASAM_MASK; // enable auto-sampling
        reading_x = reading_y = 0;
        for (i = 0; i < NSAMP; i++) {
            while (!mAD1GetIntFlag());
            reading_x += ADC1BUF0;
            reading_y += ADC1BUF1;
            mAD1ClearIntFlag();
        }
        AD1CON1CLR = _AD1CON1_ASAM_MASK; // disable auto-sampling

        reading_x = reading_x / NSAMP;
        reading_y = reading_y / NSAMP;

        if (reading_x != curr_reading_x || reading_y != curr_reading_y) {
            curr_reading_x = reading_x;
            curr_reading_y = reading_y;
            state = ERASE;
        } else {
            // otherwise, go to wait state
            wait_count = 0;
            state = WAIT;
        }
        break;

    // next phase (drawing) takes 130 times longer (13 ms) when ball radius = 15:
    // tft_printLine:  9 ms
    // tft_fillCircle: 2 ms
    // tft_fillCircle: 2 ms
    case ERASE:
        //sprintf(msg, "(%3d, %3d)", reading_x, reading_y);
        //tft_printLine(1, 2, msg);

        // "erase" current ball using "draw color"
        mPORTBToggleBits(BIT_7);
        tft_fillCircle(x, y, BALL_RADIUS, DRAW_COLOR);
        state = DRAW;
        mPORTBToggleBits(BIT_7);
        break;

    case DRAW:
        // draw new ball using "ball color"
        mPORTBToggleBits(BIT_7);
        x = (curr_reading_x - XR_MIN) * W / XR_RANGE;
        y = H - (curr_reading_y - YR_MIN) * H / YR_RANGE;
        tft_fillCircle(x, y, BALL_RADIUS, BALL_COLOR);
        wait_count = 0;
        state = WAIT;
        mPORTBToggleBits(BIT_7);
        break;

    case WAIT:
        if (++wait_count == WAIT_PERIOD) {
            state = SAMPLE;
        }
        break;
    }
}

void blink_task(void) {
    mPORTBToggleBits(BIT_5);
}

void __ISR(_TIMER_1_VECTOR, IPL2SOFT) scheduler_isr(void) {
    // Run each task if its time has come

//    mPORTBToggleBits(BIT_7);

    // Do blink task first because it takes almost no time
    if (++blink_ticks == blink_task_period) {
        blink_ticks = 0;
        blink_task();
    }

    if (++adc_ticks == adc_task_period) {
        adc_ticks = 0;
        adc_task();
    }

    mT1ClearIntFlag();

//    mPORTBToggleBits(BIT_7);
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    adc_init();
    uart_init();

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(0);

    // Use RB5 for blink task
    mPORTBSetPinsDigitalOut(BIT_5);

    mPORTBSetPinsDigitalOut(BIT_7); // debug
    mPORTBClearBits(BIT_7);

    // Timer for ADC auto-sampling
    OpenTimer3(T3_ON | T3_SOURCE_INT | T2_PS_1_1, SAMPLING_PERIOD);

    // 1 ms timer for scheduler
    OpenTimer1(T1_ON | T1_SOURCE_INT | T1_PS_1_1, PBCLK/1000);
    ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_2);
    INTEnableSystemMultiVectoredInt();

    while(1) {
        // Timer 1 ISR will interrupt
    }
    return 0;
}
