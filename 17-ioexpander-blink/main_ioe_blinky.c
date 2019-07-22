/*
   Blinky for the MCP23S17 i/o expander.

 */

#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"

char msg[80];

int main(void) {
    // Configure the device for maximum performance for the given system clock
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    sprintf(msg, "OSCTUN = 0x%02x", OSCTUN);
    tft_printLine(1, 3, msg);

    ioe_init(); // initialize port expander
    ioe_PortCSetPinsOut(0xff); // set all port C (GPA) pins as outputs
    ioe_PortDSetPinsOut(0xff); // set all port D (GPB) pins as outputs
    //ioe_write(OLATC, 0x42);

    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
    // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
    unsigned char out_value = 0x00;
    unsigned char read_value;
    while(1) {
        LATAINV = 0x1;             // toggle LED
        ioe_write(OLATC, out_value);
        ioe_write(OLATD, ~out_value);
        sprintf(msg, "out_value = 0x%02x", 0x00ff & out_value);
        tft_printLine(4, 3, msg);
        out_value = ~out_value;
        read_value = ioe_read(OLATC);
        sprintf(msg, "OLATC = 0x%02x", 0x00ff & read_value);
        tft_printLine(7, 3, msg);
        delay(250);
    }
    return 0;
}
