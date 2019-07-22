#include "config.h"
#include "tft.h"
#include "tft_printline.h"

char msg[80];
volatile int clock;

#define ROLLOVER (SYSCLK / 5)

void __ISR(_CORE_TIMER_VECTOR, IPL6SOFT) CoreTimerISR(void) {
    IFS0bits.CTIF = 0;              // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
    sprintf(msg, "%d", clock++);
    tft_printLine(10, 5, msg);
    _CP0_SET_COUNT(0);              // set core timer counter to 0
    _CP0_SET_COMPARE(ROLLOVER);   // must set CP0_COMPARE again after interrupt
}

int main(void) {
    /*
     * Display an incrementing counter.
     * The tft_printLine function doesn't do anything clever to update
     * the display, so the effect is a bit strobe-like.
     */

    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);    __builtin_disable_interrupts();
    tft_init();
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
