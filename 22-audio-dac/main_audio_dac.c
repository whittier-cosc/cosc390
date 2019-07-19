/*!
 *  @file   dac_audio.c
 *
 *  @brief  Synthesize some audio with the DAC
 *
 *  @author Jeff Lutgen
 */

#include <math.h>
#include "config.h"
#include "util.h"
#include "amp.h"
#include "dac.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];

void display_message(char *message) {
    tft_printLine(1, 2, message);
}

// DAC constants
#define DAC_CONFIG_A    (DAC_A | DAC_GAIN1X | DAC_ACTIVE)
#define DAC_CONFIG_B    (DAC_B | DAC_GAIN1X | DAC_ACTIVE)

// DDS constants
#define TWO_32           4294967296.0 // 2^32
#define SAMPLE_FREQ 100000
#define TIMER2PERIOD    (PBCLK / SAMPLE_FREQ)
#define OUT_FREQ_1   440 // 440 Hz
#define output_freq_2   660

// Globals for Timer2 interrupt handler
volatile unsigned int dac_data_1, dac_data_2 ;// output value

// DDS units:
volatile unsigned int phase_accum_main_1, phase_accum_main_2;
volatile unsigned int phase_incr_main_1 = OUT_FREQ_1*TWO_32/SAMPLE_FREQ;
volatile unsigned int phase_incr_main_2 = output_freq_2*TWO_32/SAMPLE_FREQ;

// DDS waveform tables
#define TABLE_SIZE 256
int sin_table[TABLE_SIZE];
int saw_table[TABLE_SIZE];

// Globals for Timer2 interrupt handler
char out_value = 0x55;

// Timer2 Interrupt handler.
// Compute DDS phase, update both DAC channels.
void __ISR(_TIMER_2_VECTOR, IPL2SOFT) Timer2Handler(void)
{
    mT2ClearIntFlag();

    // main DDS phase and wave table lookup
    phase_accum_main_1 += phase_incr_main_1;
    phase_accum_main_2 += phase_incr_main_2;
    dac_data_1 = sin_table[phase_accum_main_1>>24];
    dac_data_2 = sin_table[phase_accum_main_2>>24];
    dac_write(DAC_CONFIG_A | dac_data_1);
    dac_write(DAC_CONFIG_B | dac_data_2);
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    INTCONbits.MVEC = 1;   // multi-vector mode on

    ANSELA = 0; ANSELB = 0;

    // set up DAC
    // timer interrupt //////////////////////////
    // Timer2 on, interrupts, internal clock, prescaler 1, period in counts
    // At 40 MHz PB clock 40 counts is one microsecond
    // 400 counts is 100 ksamples/sec (10 microseconds between samples)
    // 2000 counts is 20 ksamples/sec
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, TIMER2PERIOD);

    // set up the timer interrupt with a priority of 2
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    mT2ClearIntFlag(); // and clear the interrupt flag

    // Build the lookup tables
    // Scaled to generate values between 0 and 4095 for 12-bit DAC
    int i;
    for (i = 0; i < TABLE_SIZE; i++) {
        sin_table[i] = 2048 + 2047*sin(i*6.2831853/TABLE_SIZE);
        saw_table[i] = 4095*i/TABLE_SIZE;
    }

    dac_init();

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    amp_init();

    display_message("compression 2:1");
    amp_setAGCCompression(TPA2016_AGC_2);
    delay(150);

    display_message("limiter 0.67 Vpp");
    amp_setLimitLevel(0);
    delay(150);

//    display_message("release off");
//    // we also have to turn off the release to really turn off AGC
//    amp_setReleaseControl(0);

    // Set gain to reasonable level
    display_message("Setting gain to -28");
    amp_setGain(-28);

    __builtin_enable_interrupts();

    while(1) {
        display_message("Right only");
        amp_enableChannel(true, false);
        delay(50);
        display_message("Left only");
        amp_enableChannel(false, true);
        delay(50);
        display_message("Left + Right");
        amp_enableChannel(true, true);
        delay(50);
    }
    return 0;
}
