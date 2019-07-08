/*
    Generate waveforms on 2-channel 12-bit DAC (MCP4822)
 */

#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"
#include "io_expander.h"
#include <math.h>

void delay(void);

char msg[40];

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 21, 1, ILI9340_BLACK);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_YELLOW); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

// DAC constants
#define DAC_SPI_CHN			SPI_CHANNEL2
// channel A, 1x gain, active
#define DAC_config_chan_A 	0b0011000000000000
// channel B, 1x gain, active
#define DAC_config_chan_B 	0b1011000000000000
#define DAC_SET_CS     		{mPORTBSetBits(BIT_4);}
#define DAC_CLEAR_CS   		{mPORTBClearBits(BIT_4);}

// DDS constants
#define two32 			4294967296.0 // 2^32 
#define samples_per_sec 100000
#define timer2period 	(PBCLK/samples_per_sec)
#define output_freq 	440 // 440 Hz

// Globals for Timer2 interrupt handler
volatile unsigned int DAC_data, DAC_data2 ;// output value
volatile int spiClkDiv = 4 ; // 10 MHz max speed for port expander
// the DDS units:
volatile unsigned int phase_accum_main;
volatile unsigned int phase_incr_main = output_freq*two32/samples_per_sec;
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
    phase_accum_main += phase_incr_main;
    DAC_data = sin_table[phase_accum_main>>24];
	DAC_data2 = sin_table[(phase_accum_main)>>24];
	//DAC_data2 = saw_table[(phase_accum_main)>>24];
 
	// wait for possible port expander transactions to complete
    while (!SpiChnTxBuffEmpty(DAC_SPI_CHN)) { ; }
    SPI_Mode16(); // reset spi mode to avoid conflict with port expander
	
	dac_write(DAC_config_chan_A | DAC_data);
	dac_write(DAC_config_chan_B | DAC_data2);
}

// Timer4 Interrupt handler.
// Toggle all I/O expander GPIO pins.
void __ISR(_TIMER_4_VECTOR, IPL1SOFT) Timer4Handler(void) {
	static volatile int t = 33;
	
	mT4ClearIntFlag();
	
	LATAINV = 0x0001; // toggle LED
	
	INTEnable(INT_T2, 0);
	ioe_write(OLATY, out_value);
	INTEnable(INT_T2, 1);
	
	INTEnable(INT_T2, 0);
	ioe_write(OLATZ, ~out_value);
	INTEnable(INT_T2, 1);
	
	sprintf(msg, "out_value = 0x%02x", out_value & 0x00ff);
	printLine(1, 3, msg);
	
	out_value = ~out_value;
}

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

	// Tune the internal oscillator
    SYSKEY = 0xAA996655; 	// two-step unlocking sequence
    SYSKEY = 0x556699AA;
    OSCTUN = 56; 			// 56 is best, empirically
    SYSKEY = 0;          	// lock

    tft_init_hw();
    tft_begin();
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

	// CS for DAC is RB4 (pin 11)
    // set CS high initially
    mPORTBSetPinsDigitalOut(BIT_4);
    DAC_SET_CS
    // SCK2 is pin 26 
	// The following PPS mapping is already done by initPE, so don't need
	// it if using IO Expander.
    // SDO2 (MOSI) is in PPS output group 2, map to RB5 (pin 14)
    //PPSOutput(2, RPB5, SDO2);
    
	// The following SpiChnOpen is redundant if using IO Expander. initPE() does the same
	// thing, but using MODE8, but that's fine, since when talking to the DAC
	// we always set MODE16 first.
    //SpiChnOpen(SPI_CHANNEL2, SPI_OPEN_ON | SPI_OPEN_MODE16 | SPI_OPEN_MSTEN | SPI_OPEN_CKE_REV , 4);
	// end DAC setup
    
	// === build the lookup tables =======
	// scaled to produce values between 0 and 4095
	int i;
	for (i = 0; i < table_size; i++) {
		 sin_table[i] = 2048 + 2047*sin(i*6.2831853/table_size);
		 saw_table[i] = 4095*i/table_size;
	}

    // === Set up I/O expander ===
    OpenTimer4(T4_ON | T4_SOURCE_INT | T4_PS_1_256, 65535);
	ConfigIntTimer4(T4_INT_ON | T4_INT_PRIOR_1);
    mT4ClearIntFlag(); // and clear the interrupt flag
	ioe_init(); // initialize I/O expander
	ioe_portYSetPinsOut(0xff); // set all port Y (GPA) pins as outputs
	ioe_portZSetPinsOut(0xff); // set all port Z (GPB) pins as outputs
	
    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
	__builtin_enable_interrupts();
	
	//*****************************************
	// next, check the cause of the Reset
	if(RCON & 0x0003)
	{
		sprintf(msg, "Power-on reset");
	}
	else if(RCON & 0x0002)
	{
		sprintf(msg, "Brown-out reset");
	}
	else if(RCON & 0x0080)
	{
		sprintf(msg, "MCLR reset");
	}
	else 
	{
		sprintf(msg, "Unknown reset");
	}
	RCON &= ~0x0083; // clear the 3 bits checked above
	printLine(4, 3, msg);
	
    while(1) {
		;
    }
    return 0;
}
