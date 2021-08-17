/*
    Generate interrupt on input change with MCP23S17 i/o expander.

 */

#include <stdio.h>
#include "config.h"
#include "util.h"
#include "tft.h"
#include "tft_printline.h"
#include "io_expander.h"

char msg[80];

void __ISR(_EXTERNAL_1_VECTOR, IPL2SOFT) pe_isr(void) {
    static unsigned char val;
    IFS0bits.INT1IF = 0;  // clear interrupt flag on PIC
    val = ioe_read(GPIOC);  // must read from pin on PE that generated interrupt (C0)
    ioe_write(OLATD, val);  // update pin D0 on PE
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    sprintf(msg, "OSCTUN = 0x%02x", OSCTUN);
    tft_printLine(1, 3, msg);

    INTCONbits.MVEC = 1;   // multi-vector mode on
    IEC0bits.INT1IE = 1;   // enable INT1
    IPC1bits.INT1IP = 2;   // priority 2
    IFS0bits.INT1IF = 0;   // clear interrupt flag
    INTCONbits.INT1EP = 0; // trigger on falling edge

    ioe_init(); // initialize port expander
    // default for port expander: interrupt line active-low
    //ioe_write(IOCON, SET_INTPOL); // interrupt line active-high
    ioe_PortDSetPinsOut(0x01); // set D0 as output
    ioe_PortCIntEnable(0x01); // enable interrupt-on-change for C0
    PPSInput(4, INT1, RPA3); // Pin 10

    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
    // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
    __builtin_enable_interrupts();

    while(1) {
    }
    return 0;
}
