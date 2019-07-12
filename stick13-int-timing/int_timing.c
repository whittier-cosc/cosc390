/*
 * See how long an interrupt service takes.
 */

#include "config.h"
#include "tft.h"

char msg[80];

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, 10*char_size, 1, ILI9340_BLACK);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_BLUE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

void timer3_init() {
    T3CONbits.TCKPS = 0; // prescaler = 1, so freq = PB freq = 40 MHz 
    PR3 = 0xffff; // default
    T3CONbits.ON = 1;
}

void __ISR(_CORE_SOFTWARE_0_VECTOR, IPL2SOFT) my_isr(void) {
    //LATAINV = 2; // toggle A1
    IFS0CLR = 0x0002; // just clear the flag
}

int main(void) {
    // Configure the device for maximum performance for the given system clock,
    // but do not change the PBDIV.
    // With the given options, this function will change the flash wait states,
    // RAM wait state, and enable prefetch and cache mode.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);

    TRISA = 0x1D; // make pin 3 (A1) output for measurement purposes

    INTCONbits.MVEC = 1; // multi-vector interrupt mode

    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    timer3_init();

    /* Set up Software Interrupt 0 */
    IEC0bits.CS0IE = 1;
    IFS0bits.CS0IF = 0;
    IPC0bits.CS0IP = 2;
    __builtin_enable_interrupts();

    int start, end;
    start = TMR3;

    /* Dummy line */
    CoreSetSoftwareInterrupt1(); // not handled

    /* Trigger software interrupt. Two ways: */			
//    CoreSetSoftwareInterrupt0();  // first way: muck with CP0 Cause register
    //IFS0SET = 0x0002; // second way: just set the flag

    // Empirically, it appears that 4 more instructions are needed in order
    // to ensure that "end = TMR3" doesn't happen until after the ISR runs.
    _nop();_nop();_nop();_nop();

    end = TMR3; 
    sprintf(msg, "%d - %d = %d", end, start, end - start);
    printLine(10, 3, msg);

    // Results: with dummy interrupt:   14 ticks
    //          with real interrupt:    78 ticks 
    // So interrupt takes 64 ticks = 64 * T_PB = 66 * 25 ns = 1.6 us
    //
    // Measuring another way (toggling output A1 in ISR, and not clearing
    // the interrupt flag, so that the ISR is called repeatedly forever)
    // also gives 1.6 us.
    //
    // Update: enabling prefetch cache (see first line in main()) speeds
    // things up; interrupt service takes only 63 - 13 = 50 ticks, or 1.25 us.
    //
    // Update: changing to IPLSOFT from IPLAUTO saves a couple of ticks,
    // as claimed in xc32 user's manual. Now interrupt service takes only 
    // 55 - 13 = 42 ticks, or 1.05 us.
    //
    // Update: using -O1 optimization (instead of none) gives
    // 49 - 11 = 38 ticks, or 0.95 us.

    while(1) {
        ;
    }
    return 0;
}
