/**
 *  @file   main_joystick.c
 *
 *  @brief  The world's worst paint program.
 *
 *          Uses 2-axis resistive analog "thumb slide" joystick (SparkFun COM-09426).
 *
 *  @author Jeff Lutgen
 */

#include <stdint.h>
#include <stdbool.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"

#define SAMPLING_PERIOD 120 // 3000 ns (if timer 3 prescaler is 1)
#define NSAMP 8

#define DRAW_COLOR  ILI9340_BLUE
#define BALL_COLOR  ILI9340_GREEN
#define BALL_RADIUS 2

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

char msg[80];

void adc_init() {
    // configure to sample AN0 & AN5
    SetChanADC10(ADC_CH0_NEG_SAMPLEA_NVREF | ADC_CH0_POS_SAMPLEA_AN0 |
                 ADC_CH0_NEG_SAMPLEB_NVREF | ADC_CH0_POS_SAMPLEB_AN5);
    // configure ADC and enable it
    OpenADC10(ADC_MODULE_ON | ADC_CLK_TMR | ADC_AUTO_SAMPLING_ON, // AD1CON1
              ADC_ALT_INPUT_ON | ADC_SAMPLES_PER_INT_2,           // AD1CON2
              ADC_CONV_CLK_PB | ADC_CONV_CLK_2Tcy,                // AD1CON3
              0,                                                  // configscan
              ENABLE_AN5_ANA | ENABLE_AN0_ANA);                   // configport
    // Now enable the ADC logic
    EnableADC10();
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    // Tell wcpic32lib our system clock and peripheral bus clock rates
    wclib_init(SYSCLK, PBCLK);

    adc_init();

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(0);

    OpenTimer3(T3_ON | T3_SOURCE_INT | T2_PS_1_1, SAMPLING_PERIOD);

    uint32_t reading_x, reading_y;
    uint32_t curr_reading_x = 1024, curr_reading_y = 1024;
    int16_t x, y;
    int i;
    bool redraw = false;
    while(1) {
        // Average NSAMP samples on each channel to smooth things out.
        // Once auto-sampling is enabled, conversions are triggered by
        // Timer 3 match.
        // Two readings per interrupt; first is from AN0, second from AN5.
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
            redraw = true;
            curr_reading_x = reading_x;
            curr_reading_y = reading_y;
            sprintf(msg, "(%3d, %3d)", reading_x, reading_y);
            tft_printLine(1, 2, msg);
        }

        if (redraw) {
            // "erase" current ball using
            tft_fillCircle(x, y, BALL_RADIUS, DRAW_COLOR);
            x = (reading_x - XR_MIN) * W / XR_RANGE;
            y = H - (reading_y - YR_MIN) * H / YR_RANGE;
            // draw new ball
            tft_fillCircle(x, y, BALL_RADIUS, BALL_COLOR);
        }

        delay(50);
    }
    return 0;
}

