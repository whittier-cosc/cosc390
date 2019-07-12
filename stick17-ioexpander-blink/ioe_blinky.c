/*
   Blinky for the MCP23S17 i/o expander.

 */

#include "config.h"
#include "util.h"
#include "tft.h"
#include "io_expander.h"

char msg[80];

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 20, 1, ILI9340_BLUE);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_WHITE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

int main(void) {
    // Configure the device for maximum performance for the given system clock
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    osc_tune(56);

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    sprintf(msg, "OSCTUN = 0x%02x", OSCTUN);
    printLine(1, 3, msg);

	ioe_init(); // initialize port expander
	ioe_PortCSetPinsOut(0xff); // set all port Y (GPA) pins as outputs
	ioe_PortDSetPinsOut(0xff); // set all port Z (GPB) pins as outputs
	//writePE(OLATY, 0x42);
	
    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
	unsigned char out_value = 0x00;
	unsigned char read_value;
    while(1) {
		LATAINV = 0x1;             // toggle LED
		ioe_write(OLATY, out_value);
		ioe_write(OLATZ, ~out_value);
		sprintf(msg, "out_value = 0x%02x", 0x00ff & out_value);
		printLine(4, 3, msg);
		out_value = ~out_value;
		read_value = ioe_read(OLATY);
		sprintf(msg, "OLATY = 0x%02x", 0x00ff & read_value);
		printLine(7, 3, msg);
		delay(250);
    }
    return 0;
}
