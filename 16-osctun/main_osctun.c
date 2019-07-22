/*
 * Experiment with OSCTUN. Reference manual says we can change 
 * FRC oscillator frequency by +/- 12.5%. I get more like +/- 1.5%.
 *
 * Hmmmm. Well, the datasheet does say that "The tuning step size is an
 * approximation, and is neither characterized, nor tested."
 *
 * Oscilloscope on pin 2. Actual SYSCLK frequency is given by
 * 4*CP0PERIOD*pin2freq.
 * 
 * With OSCTUN = 0 (default), system frequency is 40.152 MHz
 *
 * With OSCTUN = 32 (max slowdown), it is         39.523 MHz (-1.6%)
 * With OSCTUN = 31 (max speedup), it is          40.762 MHz (+1.5%)
 *
 * If we use FRC only (without PLL + div/mult), so that nominal SYSCLK is 8M,
 * we get the same results:
 *
 * With OSCTUN = 0 (default), system frequency is 8.026 MHz
 *
 * With OSCTUN = 32 (max slowdown), it is         7.901 MHz (-1.6%)
 * With OSCTUN = 31 (max speedup), it is          8.148 MHz (+1.5%)
 */

#include "config.h"
#include "tft.h"
#include "tft_printline.h"
#include "util.h"

#define CP0PERIOD 100000
char msg[80];

void __ISR(_CORE_TIMER_VECTOR, IPL6SOFT) CoreTimerISR(void) {
    IFS0bits.CTIF = 0;         // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
    LATAINV = 0x1;             // toggle LED
    _CP0_SET_COUNT(0);         // set core timer counter to 0
    _CP0_SET_COMPARE(CP0PERIOD);  // must set CP0_COMPARE again after interrupt
}

int main(void) {
    // Configure the device for maximum performance for the given system clock.
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    osc_tune(56);

    tft_init();
    tft_fillScreen(ILI9340_BLACK);
    tft_setRotation(3); // landscape mode, pins at left

    sprintf(msg, "OSCTUN = 0x%02x", OSCTUN);
    tft_printLine(1, 3, msg);

    __builtin_disable_interrupts();
    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    _CP0_SET_COMPARE(CP0PERIOD);    // CP0_COMPARE set to 40 M
    IPC0bits.CTIP = 6;              // interrupt priority
    IPC0bits.CTIS = 0;              // subpriority 0 (default)
    IFS0bits.CTIF = 0;              // clear CT interrupt flag
    IEC0bits.CTIE = 1;              // enable core timer interrupt
    __builtin_enable_interrupts();
    _CP0_SET_COUNT(0);              // set core timer counter to 0

    while(1) {
        ;
    }
    return 0;
}
