/*!
 *  @file   dac_audio.c
 *
 *  @brief  Synthesize some audio with the DAC
 *  @author Jeff Lutgen
 */

#include <math.h>
#include "config.h"
#include "amp.h"
#include "tft_master.h"
#include "tft_gfx.h"

static void delay(void);
char msg[40];

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 21, 1, ILI9340_BLACK); // x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_YELLOW);
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

void display_message(char *message) {
    printLine(1, 2, message);
}

// DAC constants
#define DAC_SPI_CHN             SPI_CHANNEL2
// channel A, 1x gain, active
#define DAC_config_chan_A       0b0011000000000000
// channel B, 1x gain, active
#define DAC_config_chan_B       0b1011000000000000
#define DAC_SET_CS              {mPORTBSetBits(BIT_4);}
#define DAC_CLEAR_CS            {mPORTBClearBits(BIT_4);}

// DDS constants
#define two32           4294967296.0 // 2^32
#define samples_per_sec 100000
#define timer2period    (PBCLK/samples_per_sec)
#define output_freq_1     440 // 440 Hz
#define output_freq_2     660

// Globals for Timer2 interrupt handler
volatile unsigned int DAC_data_1, DAC_data_2 ;// output value

// the DDS units:
volatile unsigned int phase_accum_main_1, phase_accum_main_2;
volatile unsigned int phase_incr_main_1 = output_freq_1*two32/samples_per_sec;
volatile unsigned int phase_incr_main_2 = output_freq_2*two32/samples_per_sec;

// DDS waveform tables
#define table_size 256
int sin_table[table_size];
int saw_table[table_size];

// Globals for Timer2 interrupt handler
char out_value = 0x55;
unsigned char read_value;

inline void dac_write(unsigned int msg) {
    DAC_CLEAR_CS // CS low to start transaction
    SpiChnWriteC(DAC_SPI_CHN, msg);
    while (SpiChnIsBusy(DAC_SPI_CHN)) { ; } // wait for end of transaction
    DAC_SET_CS // CS high to end transaction
    // need to read SPI channel to avoid confusing port expander
    SpiChnReadC(DAC_SPI_CHN); // (ignore return value )
}

// Timer2 Interrupt handler.
// Compute DDS phase, update both DAC channels.
void __ISR(_TIMER_2_VECTOR, IPL2SOFT) Timer2Handler(void)
{
    mT2ClearIntFlag();

    // main DDS phase and wave table lookup
    phase_accum_main_1 += phase_incr_main_1;
    phase_accum_main_2 += phase_incr_main_2;

    DAC_data_1 = sin_table[phase_accum_main_1>>24];
    DAC_data_2 = sin_table[phase_accum_main_2>>24];

    // wait for possible port expander transactions to complete
    while (!SpiChnTxBuffEmpty(DAC_SPI_CHN)) { ; }
    SPI_Mode16(); // reset spi mode to avoid conflict with port expander

    dac_write(DAC_config_chan_A | DAC_data_1);
    dac_write(DAC_config_chan_B | DAC_data_2);
}

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

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

    // CS for DAC is RB4 (pin 11)
    // set CS high initially
    mPORTBSetPinsDigitalOut(BIT_4);
    DAC_SET_CS

    // SCK2 is pin 26
    // The following PPS mapping is already done by initPE, so don't need
    // it if using IO Expander.
    // SDO2 (MOSI) is in PPS output group 2, map to RB5 (pin 14)
    PPSOutput(2, RPB5, SDO2);

    // The following SpiChnOpen is redundant if using IO Expander. initPE() does the same
    // thing, but using MODE8, but that's fine, since when talking to the DAC
    // we always set MODE16 first.
    SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV , 4);
    // end DAC setup

    // === build the lookup tables =======
    // scaled to produce values between 0 and 4095
    int i;
    for (i = 0; i < table_size; i++) {
        sin_table[i] = 2048 + 2047*sin(i*6.2831853/table_size);
        saw_table[i] = 4095*i/table_size;
    }

    __builtin_enable_interrupts();

    tft_init_hw();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    // amp_init() sets up I2C:
    //      SCL1 pin 17 (B8)
    //  SDA1 pin 18 (B9)
    // and UART1 (for debugging):
    //              U1RX pin 9  (RPA2)
    //              U1TX pin 7  (RPB3)
    amp_init();

    display_message("compression 2:1");
    amp_setAGCCompression(TPA2016_AGC_2);
    delay();

    display_message("limiter 0.67 Vpp");
    amp_setLimitLevel(0);
    delay();

//    display_message("release off");
//    // we also have to turn off the release to really turn off AGC
//    amp_setReleaseControl(0);

    // Set gain to reasonable level
    display_message("Setting gain to -28");
    amp_setGain(-28);

    while(1) {
        // We can update the gain, from -28dB up to 30dB
        int8_t i, gain;
        for (i = -28; i <= 0; i += 2) {
            if (i < 0)
                sprintf(msg, "Gain = %d", -(-i & 0x00ff));
            else
                sprintf(msg, "Gain = %d", i & 0x00ff);
            display_message(msg);
            amp_setGain(i);
            delay();
            gain = amp_getGain();
            if (gain < 0)
                sprintf(msg, "getGain returned %d", -(-gain & 0x00ff));
            else
                sprintf(msg, "getGain returned %d", gain & 0x00ff);
            display_message(msg);
            delay();
        }
//        display_message("Right only");
//        amp_enableChannel(true, false);
//        delay();
//        display_message("Left only");
//        amp_enableChannel(false, true);
//        delay();
//        display_message("Left + Right");
//        amp_enableChannel(true, true);
//        delay();
    }
    return 0;
}

static void delay(void) {
    volatile int j;
    for (j = 0; j < 1000000; j++) {
    }
}
