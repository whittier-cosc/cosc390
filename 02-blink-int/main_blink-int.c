#include "config.h"

/*
   The core timer increments every two system clock (SYSCLK) cycles
   (DS61113E-page 2-5). Toggle LED when core timer reaches SYSCLK. If SYSCLK is
   8 M, this will take 16 M system clock cycles, so one LED on/off cycle will
   take 32 M system clock cycles. Actual SYSCLK is thus (LED freq)*(32 M).

   In general, SYSCLK_actual = (LED freq)*(4*SYSCLK).

   With SYSCLK set to 8 MHz, measurement gives LED freq = 0.25090 Hz, so actual
   SYSCLK is 8.0288 MHz, about 0.36% high, but well within stated tolerance of
   +/- 0.9% (DS60001168K-page 273).

   With SYSCLK set to 40 MHz, measurement gives LED freq = 0.25100 Hz, so
   actual SYSCLK is 40.16 MHz, which is 0.40% high.
*/

void __ISR(_CORE_TIMER_VECTOR, IPL6SOFT) CoreTimerISR(void) {
    IFS0bits.CTIF = 0;         // clear CT int flag IFS0<0>, same as IFS0CLR=0x0001
    LATAINV = 0x1;             // toggle LED
    _CP0_SET_COUNT(0);         // set core timer counter to 0
    _CP0_SET_COMPARE(SYSCLK);  // must set CP0_COMPARE again after interrupt
}

int main(void) {
    SYSTEMConfig(SYSCLK, SYS_CFG_WAIT_STATES | SYS_CFG_PCACHE);
    wclib_init(SYSCLK, PBCLK);
    __builtin_disable_interrupts();
    TRISA = 0xFFFE;         // Pin 0 of Port A is LED. Clear
                            // bit 0 to zero, for output.  Others are inputs.
    LATAbits.LATA0 = 0;     // Turn LED off.
    _CP0_SET_COMPARE(SYSCLK);       // CP0_COMPARE set to 40 M
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

