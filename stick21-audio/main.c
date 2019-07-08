#include "config.h"
#include "amp.h"
#include "tft_master.h"
#include "tft_gfx.h"

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

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
	
	tft_init_hw();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left
	
	// I2C pins: 
	//				SCL1 pin 17 (B8)
	//				SDA1 pin 18 (B9)
	
	// Set the I2C baudrate
    I2CSetFrequency(TPA2016_I2C_BUS, GetPeripheralClock(), I2C_CLOCK_FREQ);
	// Enable the I2C bus
    I2CEnable(TPA2016_I2C_BUS, TRUE);
	
	//amp_init();
	
	sprintf(msg, "compression off");
	printLine(1, 3, msg);
	// Turn off AGC for the gain test
	amp_setAGCCompression(TPA2016_AGC_OFF);
	
	sprintf(msg, "release off");
	printLine(1, 3, msg);
	// we also have to turn off the release to really turn off AGC
	amp_setReleaseControl(0);
  
	// We can update the gain, from -28dB up to 30dB
	int8_t i;
	for (i = -28; i <= 30; i++) {
		sprintf(msg, "Gain = %d", i & 0x00ff);
		printLine(1, 3, msg);
		amp_setGain(i);
		delay();
	}
  
    while(1) {
        ;
    }
    return 0;
}

void delay(void) {
    volatile int j;
    for (j = 0; j < 1000000; j++) {
    }
}
