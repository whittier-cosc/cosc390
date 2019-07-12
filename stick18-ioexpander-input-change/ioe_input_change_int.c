/*
    Generate interrupt on input change with MCP23S17 i/o expander.

 */

#include "config.h"
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

void __ISR(_EXTERNAL_1_VECTOR, IPL2SOFT) pe_isr(void) {
    static unsigned char val;
    IFS0bits.INT1IF = 0;  // clear interrupt flag on PIC
    val = ioe_read(GPIOY);  // must read from pin on PE that generated interrupt (Y0)
    ioe_write(OLATZ, val);  // update pin Z0 on PE
}

int main(void) {
    // Configure the device for maximum performance
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    osc_tune(56);

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    sprintf(msg, "OSCTUN = 0x%02x", OSCTUN);
    printLine(1, 3, msg);

    INTCONbits.MVEC = 1;   // multi-vector mode on
    IEC0bits.INT1IE = 1;   // enable INT1
    IPC1bits.INT1IP = 2;   // priority 2
    IFS0bits.INT1IF = 0;   // clear interrupt flag
    INTCONbits.INT1EP = 0; // trigger on falling edge

    ioe_init(); // initialize port expander
    // default for port expander: interrupt line active-low
    //writePE(IOCON, SET_INTPOL); // interrupt line active-high
    ioe_PortDSetPinsOut(0x01); // set GPB0 as output
    ioe_PortCIntEnable(0x01); // enable interrupt-on-change for GPA0
    PPSInput(4, INT1, RPA3); // Pin 10


    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
    // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    
    __builtin_enable_interrupts();

    while(1) {
        ;
    }
    return 0;
}
