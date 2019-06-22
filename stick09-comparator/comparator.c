#include "config.h"
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
    tft_fillRoundRect(0, v_pos, 319, char_size * 10, 1, ILI9340_BLUE);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_YELLOW); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

//void __ISR(_CORE_TIMER_VECTOR, IPL6AUTO) CoreTimerISR(void) {
//    IFS0bits.CTIF = 0;              // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
//    sprintf(msg, "%d", clock++);
//    printLine(10, 5, msg);
//    _CP0_SET_COUNT(0);              // set core timer counter to 0
//    _CP0_SET_COMPARE(ROLLOVER);   // must set CP0_COMPARE again after interrupt
//}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    //tft_init_hw();
    //tft_begin();
    //tft_fillScreen(ILI9340_BLUE);
    //tft_setRotation(3); // landscape mode, pins at left
    //sprintf(msg, "TRISA: 0x%08X", TRISA);
    //printLine(0, 2, msg);
    //sprintf(msg, "TRISB: 0x%08X", TRISB);
    //printLine(2, 2, msg);

    // Comparator voltage reference
    CVRCONbits.CVROE = 1; // enable output on CVrefout (pin 25)
                          // Pin 25 is used for SPI clock output, however,
                          // so can't use my usual TFT LCD setup.
    CVRCONbits.CVR = 8;   // Should give output = 0.5 * VDD
    CVRCONbits.ON = 1;
    // Measurement gives output voltage = 1.62 V and VDD = 3.25 V, so
    // seems OK.

    // Comparator
    CFGCONbits.IOLOCK = 0;
    RPB9R = 7; // Map C1OUT to RPB9 (pin 18)
    CFGCONbits.IOLOCK = 1;
    // We'll compare C1INB (pin 6) to internal voltage reference
    // configured above.
    CM1CONbits.COE = 1; // Enable output on C1OUT
    CM1CONbits.EVPOL = 0; // Disable generation of interrupts by comparator
    CM1CONbits.CREF = 1;  // Compare to Vref configured above (non-inverting)
    CM1CONbits.CCH = 0;     // Use C1INB input (as inverting input)
    CM1CONbits.ON = 1;

    while(1) {
    }
    return 0;
}

void delay(void) {
    volatile int j;
    for (j = 0; j < 1000000; j++) { // number is 1 million
    }
}
