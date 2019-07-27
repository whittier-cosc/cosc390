/*
    Control DAC output frequency and audio amplifier with keypad
 */

// keypad connections:
// C0 -- col 1 -- internal pullup resistor -- avoid open circuit input when no button pushed
// C1 -- col 2 -- internal pullup resistor
// C2 -- col 3 -- internal pullup resistor
// D0 -- row 1 -- thru 300 ohm resistor -- avoid short when two buttons pushed
// D1 -- row 2 -- thru 300 ohm resistor
// D2 -- row 3 -- thru 300 ohm resistor
// D3 -- row 4 -- thru 300 ohm resistor

#include <math.h>
#include <stdio.h>
#include "config.h"
#include "util.h"
#include "dac.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"
#include "amp.h"
#include "uart.h"

#define AMP 0  // whether to use amplifier

char buffer[80];
int keytable[12] = {0x82,                           //    0
                               0x11, 0x12, 0x14,    // 1, 2, 3
                               0x21, 0x22, 0x24,    // 4, 5, 6
                               0x41, 0x42, 0x44,    // 7, 8, 9
                               0x81, 0x84};         // *,    #

// DAC constants
#define DAC_CONFIG_A    (DAC_A | DAC_GAIN1X | DAC_ACTIVE)
#define DAC_CONFIG_B    (DAC_B | DAC_GAIN1X | DAC_ACTIVE)

// DDS parameters
#define TWO_32           4294967296.0 // 2^32
#define SAMPLE_FREQ 100000
#define TIMER2PERIOD    (PBCLK / SAMPLE_FREQ)
#define OUT_FREQ_1   440 // 440 Hz
int output_freq_2 = 440;

// Globals for Timer2 interrupt handler
unsigned int dac_data_1, dac_data_2 ; // output values

// DDS units:
unsigned int phase_accum_main_1, phase_accum_main_2;
unsigned int phase_incr_main_1 = OUT_FREQ_1*TWO_32/SAMPLE_FREQ;
volatile unsigned int phase_incr_main_2; // shared by two ISRs

// DDS waveform tables
#define TABLE_SIZE 256
int sin_table[TABLE_SIZE];
int saw_table[TABLE_SIZE];

// Amplifier globals
int gain;
bool sleep = false;

void display_message(char *message) {
    tft_printLine(1, 2, message);
}

// Timer2 Interrupt handler.
// Compute DDS phase, update both DAC channels.
void __ISR(_TIMER_2_VECTOR, IPL3SOFT) Timer2Handler(void)
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

// Timer3 interrupt handler.
// Scan keypad. Lower priority than DAC ISR.
void __ISR(_TIMER_3_VECTOR, IPL2SOFT) Timer3Handler(void) {
    int i;
    int keypad;
    int pattern;
    int pressed = 0;
    static int cleared = 1;

    mT3ClearIntFlag();
    LATAINV = 0x0002; // toggle A1 (pin 3) DEBUG
    pattern = 1;
    for (i = 0; i < 4; i++) {
        //printf("%d", i+1);

        INTEnable(INT_T2, 0);
        ioe_write(OLATD, ~pattern); // pull row i low
        INTEnable(INT_T2, 1);

        INTEnable(INT_T2, 0);
        keypad = ~ioe_read(GPIOC) & 0x7; // read the three columns
        INTEnable(INT_T2, 1);

        if (keypad != 0) {
            //printf("0x%02x ", keypad);
            keypad |= (pattern << 4);
            break;
        }
        pattern <<= 1;
    }

    // search for keycode
    if (keypad > 0) { // then at least one button is pushed
        for (i = 0; i < 12; i++){
            if (keytable[i] == keypad) {
                pressed = 1;  // got a valid keypress (single button)
                break;
            }
        }
    }
    else {
        cleared = 1;
        return;
    }

    if (cleared && pressed) { // valid key, but not just previous one still being held down
        cleared = 0;
        if (i < 10) {
            tft_write('0' + i);
            //printf("%c", '0' + i);
            if (i == 0) {
                sleep = !sleep;
                amp_sleep(sleep);
                display_message("toggle sleep");
            }
            else if (i == 1) {
                output_freq_2 += 40;
                sprintf(buffer, "%d Hz", output_freq_2);
                display_message(buffer);
                phase_incr_main_2 = output_freq_2*TWO_32/SAMPLE_FREQ;
            } else if (i == 3) {
                output_freq_2 -= 40;
                sprintf(buffer, "%d Hz", output_freq_2);
                display_message(buffer);
                phase_incr_main_2 = output_freq_2*TWO_32/SAMPLE_FREQ;
            }
        }
        else if (i == 10) {
            tft_write('*');
            //printf("*");
            if (AMP) {
                display_message("gain -28 dB");
                amp_setGain(-28);
                gain = -28;
            }
        }
        else {   // i == 11
            tft_write('#');
            //printf("#");
            if (AMP) {
                gain += 4;
                sprintf(buffer, "gain %d dB", gain);
                display_message(buffer);
                amp_setGain(gain);
            }
        }
        // brain-dead debounce algorithm, but works fine:
        //delay(25);
    }
}

// Timer4 interrupt handler.
// Toggle LED. Highest priority!
void __ISR(_TIMER_4_VECTOR, IPL1SOFT) Timer4Handler(void) {
    static int i;   // should be auto-initialized to 0 since it's static

    mT4ClearIntFlag();

    // This ISR gets called 4 times per second, but we only want to toggle the LED
    // twice per second.
    i++;
    if (i % 2 == 1) {
        return;
    }

    LATAINV = 1;
}

int main(void) {
    int i;

    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);    osc_tune(56);

    INTCONbits.MVEC = 1;   // multi-vector mode on

    ANSELA = 0;
    ANSELB = 0;

    // Set up DAC

    // Timer for DAC interrupt.
    // Timer2 on, interrupts, internal clock, prescaler 1, period in counts
    // At 40 MHz PB clock 40 counts is one microsecond
    // 400 counts is 100 ksamples/sec (10 microseconds between samples)
    // 2000 counts is 20 ksamples/sec
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, TIMER2PERIOD);
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_3);
    mT2ClearIntFlag();

    // Build the lookup tables
    // Scaled to generate values between 0 and 4095 for 12-bit DAC

    for (i = 0; i < TABLE_SIZE; i++) {
        sin_table[i] = 2048 + 2047*sin(i*6.2831853/TABLE_SIZE);
        saw_table[i] = 4095*i/TABLE_SIZE;
    }
    phase_incr_main_2 = output_freq_2*TWO_32/SAMPLE_FREQ;
    dac_init();

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
    tft_setCursor(0, 0);
    tft_setTextColor(ILI9340_YELLOW);
    tft_setTextSize(5);
    tft_setTextWrap(1);

    if (AMP) {
        amp_init();

        display_message("compression 2:1");
        amp_setAGCCompression(TPA2016_AGC_2);
        delay(150);

//        display_message("limiter 0.67 Vpp");
//        amp_setLimitLevel(0);
//        delay(150);

        // Set gain to reasonable level
        display_message("gain -28 dB");
        amp_setGain(-28);
        gain = -28;
    }

    ioe_init();
    // init the keypad pins D0-D3 and C0-C2
    // PortA ports as digital outputs
    ioe_PortDSetPinsOut(BIT_0 | BIT_1 | BIT_2 | BIT_3);
    // PortB as inputs
    ioe_PortCSetPinsIn(BIT_0 | BIT_1 | BIT_2);
    // and turn on pull-up resistors on inputs
    ioe_PortCEnablePullUp(BIT_0 | BIT_1 | BIT_2);

    // Timer for keypad scanner interrupt
    OpenTimer3(T3_ON | T3_SOURCE_INT | T3_PS_1_256, 8000); // ~20 interrupts per second
    ConfigIntTimer3(T3_INT_ON | T3_INT_PRIOR_2);
    mT3ClearIntFlag();

    // Timer for blinky interrupt
    OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_256, 39062); // 4 interrupts per second
    ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_1);
    mT4ClearIntFlag();

    display_message("ready");

    uart_init();
    printf("ready\n");
    uart_write("ready\n");
    TRISA = 0xFFFE; // for heartbeat LED
    LATAbits.LATA0 = 1; // LED on initially

    __builtin_enable_interrupts();  // start the whole shebang running

    while(1) {

    }

    return 0;
}
