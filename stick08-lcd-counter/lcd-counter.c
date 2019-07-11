#include "config.h"
#include "tft_master.h"
#include "tft_gfx.h"

char msg[40];
volatile int clock;

#define ROLLOVER (SYSCLK / 5)

void printLine(int line_number, int char_size, char *print_buffer) {
    // line number 0 to 30
    // char_size 1 to 5
    // print_buffer the string to print
    int v_pos;
    v_pos = line_number * 10 ;
    tft_fillRoundRect(0, v_pos, 319, char_size * 10, 1, ILI9340_BLUE);// x,y,w,h,radius,color
    tft_setCursor(0, v_pos);
    tft_setTextColor(ILI9340_WHITE); 
    tft_setTextSize(char_size);
    tft_writeString(print_buffer);
}

void __ISR(_CORE_TIMER_VECTOR, IPL6SOFT) CoreTimerISR(void) {
    IFS0bits.CTIF = 0;              // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
    sprintf(msg, "%d", clock++);
    printLine(10, 5, msg);
    _CP0_SET_COUNT(0);              // set core timer counter to 0
    _CP0_SET_COMPARE(ROLLOVER);   // must set CP0_COMPARE again after interrupt
}

int main(void) {
    /*
     * Display an incrementing counter.
     * The printLine function doesn't do anything clever to update
     * the display, so the effect is a bit strobe-like.
     */

    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    __builtin_disable_interrupts();
    tft_init();
    tft_begin();
    tft_fillScreen(ILI9340_BLUE);
    tft_setRotation(3); // landscape mode, pins at left

    _CP0_SET_COMPARE(ROLLOVER);
    IPC0bits.CTIP = 6;              // interrupt priority
    IFS0bits.CTIF = 0;              // clear CT interrupt flag
    IEC0bits.CTIE = 1;              // enable core timer interrupt
    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);              // set core timer counter to 0

    while(1) {
    }
    return 0;
}
