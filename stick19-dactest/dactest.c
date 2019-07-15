/*!
 *  @file   dactest.c
 *
 *  @brief  Generates simple waveforms using dual 12-bit DAC (MCP4822)
 *
 *          Portions based on code written by Bruce R. Land of Cornell University
 *
 *  @author Jeff Lutgen
 */

#include <math.h>
#include "config.h"
#include "dac.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"

char msg[80];

// DAC constants
#define DAC_CONFIG_A    (DAC_A | DAC_GAIN1X | DAC_ACTIVE)
#define DAC_CONFIG_B    (DAC_B | DAC_GAIN1X | DAC_ACTIVE)

// DDS constants
#define two32 		4294967296.0 // 2^32
#define samples_per_sec 100000
#define timer2period 	(PBCLK/samples_per_sec)
#define output_freq 	440 // 440 Hz

// Globals for Timer2 interrupt handler
volatile unsigned int dac_data_1, dac_data_2 ;// output value
volatile int spiClkDiv = 4 ; // 10 MHz max speed for port expander

// DDS units:
volatile unsigned int phase_accum_main;
volatile unsigned int phase_incr_main = output_freq*two32/samples_per_sec;

// DDS waveform tables
#define table_size 256
int sin_table[table_size];
int saw_table[table_size];

// Globals for Timer2 interrupt handler
char out_value = 0x55;

// Timer2 Interrupt handler. 
// Compute DDS phase, update both DAC channels.
void __ISR(_TIMER_2_VECTOR, IPL2SOFT) Timer2Handler(void)
{
    mT2ClearIntFlag();
    
    // main DDS phase and wave table lookup
    phase_accum_main += phase_incr_main;
    dac_data_1 = sin_table[phase_accum_main>>24];
    dac_data_2 = saw_table[(phase_accum_main)>>24];
    dac_write(DAC_CONFIG_A | dac_data_1);
    dac_write(DAC_CONFIG_B | dac_data_2);
}

// Timer4 Interrupt handler.
// Toggle all I/O expander GPIO pins.
void __ISR(_TIMER_4_VECTOR, IPL1SOFT) Timer4Handler(void) {
    static volatile int t = 33;

    mT4ClearIntFlag();

    LATAINV = 0x0001; // toggle LED

    INTEnable(INT_T2, 0);
    ioe_write(OLATC, out_value);
    INTEnable(INT_T2, 1);

    INTEnable(INT_T2, 0);
    ioe_write(OLATD, ~out_value);
    INTEnable(INT_T2, 1);

    sprintf(msg, "out_value = 0x%02x", out_value & 0x00ff);
    tft_printLine(1, 3, msg);

    out_value = ~out_value;
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    INTCONbits.MVEC = 1;   // multi-vector mode on

    ANSELA = 0; ANSELB = 0;

    // set up DAC
    // timer interrupt //////////////////////////
    // Timer2 on, interrupts, internal clock, prescaler 1, period in counts
    // At 40 MHz PB clock 40 counts is one microsecond
    // 400 counts is 100 ksamples/sec (10 microseconds between samples)
    // 2000 counts is 20 ksamples/sec
    OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, timer2period);

    // set up the timer interrupt with a priority of 2
    ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
    mT2ClearIntFlag(); // and clear the interrupt flag
    
    // Build the lookup tables
    // Scaled to produce values between 0 and 4095
    int i;
    for (i = 0; i < table_size; i++) {
        sin_table[i] = 2048 + 2047*sin(i*6.2831853/table_size);
        saw_table[i] = 4095*i/table_size;
    }

    dac_init();

    // Set up I/O expander
    OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_256, 65535);
    ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_1);
    mT4ClearIntFlag(); // and clear the interrupt flag
    ioe_init(); // initialize I/O expander
    ioe_PortCSetPinsOut(0xff); // set all port C (GPA) pins as outputs
    ioe_PortDSetPinsOut(0xff); // set all port D (GPB) pins as outputs

    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
    __builtin_enable_interrupts();

    while(1) {
        ;
    }
    return 0;
}
